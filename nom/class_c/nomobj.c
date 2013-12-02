/* ***** BEGIN LICENSE BLOCK *****
* Version: CDDL 1.0/LGPL 2.1
*
* The contents of this file are subject to the COMMON DEVELOPMENT AND
* DISTRIBUTION LICENSE (CDDL) Version 1.0 (the "License"); you may not use
* this file except in compliance with the License. You may obtain a copy of
* the License at http://www.sun.com/cddl/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is "NOM" Netlabs Object Model
*
* The Initial Developer of the Original Code is
* netlabs.org: Chris Wohlgemuth <cinc-ml@netlabs.org>.
* Portions created by the Initial Developer are Copyright (C) 2005-2007
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
* case the provisions of the LGPL are applicable instead of those above. If
* you wish to allow use of your version of this file only under the terms of
* the LGPL, and not to allow others to use your version of this file under
* the terms of the CDDL, indicate your decision by deleting the provisions
* above and replace them with the notice and other provisions required by the
* LGPL. If you do not delete the provisions above, a recipient may use your
* version of this file under the terms of any one of the CDDL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */
/** \file nomobj.idl 
    
    Implementation file for the NOM class NOMObject.
*/

#ifndef NOM_NOMObject_IMPLEMENTATION_FILE
#define NOM_NOMObject_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
# define INCL_DOS
# include <os2.h>
#endif /* __OS2__ */

#include <string.h>
/* #include <gtk/gtk.h> - why? */

#include "nom.h"
#include "nomtk.h"

#include "nomobj.ih"

#include "nommethod.h"
#include "nomarray.h"
#include "gc.h"

/**
    \brief This function implements the method nomInit() of class NOMObject.
 */
NOM_Scope void  NOMLINK impl_NOMObject_nomInit(NOMObject *nomSelf, CORBA_Environment *ev)
{  

}

/**
    \brief This function implements the method nomUnInit() of class NOMObject.
 */
NOM_Scope void  NOMLINK impl_NOMObject_nomUnInit(NOMObject *nomSelf, CORBA_Environment *ev)
{
  /* NOMObjectData *nomThis = NOMObjectGetData(nomSelf); */
  
  nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. SomSelf is: %s.\n",
            __FUNCTION__, impl_NOMObject_nomUnInit, nomSelf , nomSelf->mtab->nomClassName);
}

/**
    \brief This function implements the method nomGetSize() of class NOMObject.
 */
NOM_Scope CORBA_long NOMLINK impl_NOMObject_nomGetSize(NOMObject* nomSelf, CORBA_Environment *ev)
{
  //nomPrintf("    Entering %s (%x) with nomSelf: 0x%x. nomSelf is: %s.\n",
  //          __FUNCTION__, impl_NOMObject_nomGetSize, nomSelf , nomSelf->mtab->nomClassName);

  if(!nomSelf) {
    return 0;
  }

  return nomSelf->mtab->ulInstanceSize;
}

/**
    \brief This function implements the method delete() of class NOMObject.

    It calls nomUnInit() to give the object a chance of freeing system resources. Afterwards
    the memory occupied by the object is given back to the system and the object is not
    accessible anymore.
 */
NOM_Scope void NOMLINK impl_NOMObject_delete(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
  GC_PTR oldData;
  GC_finalization_proc oldFinalizerFunc;
  NOMClassPriv *ncp;

  /* Unregister finalizer if the class uses nomUnInit. This is done so nomUnInit isn't
     called again when the memory is eventually collected by the GC. */
  ncp=(NOMClassPriv*)nomSelf->mtab->nomClsInfo;
  if(ncp->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN){
    /* A NULL finalizer function removes the finalizer */
    GC_register_finalizer(nomSelf,  NULL, NULL, &oldFinalizerFunc, &oldData);
  }

  /* Give object the chance to free resources */
  _nomUnInit(nomSelf, NULL);

  /* And now delete the object */
  /*
    FIXME: we should probably call a class function here, so the
    class can keep track of objects.
   */
  NOMFree(nomSelf);
}

/**
    \brief This function implements the method nomQueryClass() of class NOMObject.
    It returns a pointer to the class object of this object.

    \param nomSelf The pointer to the object.
    \param ev      Environment pointer or NULL.
    \retval PNOMClass A pointer to the class object for this object. This can never be NULL.
 */
NOM_Scope PNOMClass NOMLINK impl_NOMObject_nomQueryClass(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */

  return nomSelf->mtab->nomClassObject;
}

/**

   \brief This function implements the method new() of class NOMObject.

  Create a new class of the kind the caller is. This method ensures that subclassing
  is properly handled without the need to override this method in every subclass.
  
  This method will get the class object of nomSelf () which may be any subclass
  of NOMObject) and call nomNew() on it creating
  a new object which has exactly the same class hierarchy as nomSelf.
 */
NOM_Scope PNOMObject NOMLINK impl_NOMObject_new(NOMObject* nomSelf, CORBA_Environment *ev)
{
/* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
  NOMClass* nomCls;

  /* We don't know which class we're actually. So we can't just create a new object using
     <CkassName>New() here.
     It is possible that we are called by a subclass. So get the class object and let the
     class object create the correct class. */
  nomCls=NOMObject_nomQueryClass(nomSelf, NULL);
  return NOMClass_nomNew(nomCls, NULL);
}



NOM_Scope CORBA_boolean NOMLINK impl_NOMObject_nomIsA(NOMObject* nomSelf, const PNOMClass nomClass, CORBA_Environment *ev)
{
  /* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */

  if(!nomIsObj(nomClass)){
    g_warning("%s: class object pointer nomClass does not point to an object.", __FUNCTION__);
    return FALSE;
  }

  return _nomIsANoClsCheck(nomSelf, nomClass, ev);
}


NOM_Scope CORBA_boolean NOMLINK impl_NOMObject_nomIsANoClsCheck(NOMObject* nomSelf, const PNOMClass nomClass,
                                                                CORBA_Environment *ev)
{
  /* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
  NOMClassPriv* ncp;
  nomMethodTabs mtabs; /* List of mtabs */

  /* Check if we have the class in our list of classes */
  ncp=(NOMClassPriv*)nomSelf->mtab->nomClsInfo;
  mtabs=&ncp->mtabList;
  while(mtabs)
    {
      if(nomClass==mtabs->mtab->nomClassObject)
          return TRUE;

      mtabs=mtabs->next;
    }
  return FALSE;
}

NOM_Scope CORBA_boolean NOMLINK impl_NOMObject_nomIsInstanceOf(NOMObject* nomSelf, const PNOMClass nomClass, CORBA_Environment *ev)
{
  /* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
 
  if(!nomIsObj(nomClass)){
    g_warning("%s: class object pointer nomClass does not point to an object.", __FUNCTION__);
    return FALSE;
  }
 
  if(nomClass==_nomQueryClass(nomSelf, NULL))
    return TRUE;
  
  return FALSE;
}

/**
   Function which implements the nomQueryClassName() method of NOMObject.
*/
NOM_Scope CORBA_string NOMLINK impl_NOMObject_nomQueryClassName(NOMObject* nomSelf, CORBA_Environment *ev)
{
  /* NOMObjectData* nomThis=NOMObjectGetData(nomSelf); */
  return nomSelf->mtab->nomClassName;
}

/*
 Create a new NOMArray holding NOMMethod objects.
 */
NOMDLLEXPORT NOM_Scope NOMObject* NOMLINK impl_NOMObject_nomGetMethodList(NOMObject* nomSelf,
                                                                          const CORBA_boolean bIncludingParents,
                                                                          CORBA_Environment *ev)
{
  NOMClassPriv* ncPriv;
  NOMArray*nomArray=NOMArrayNew();
  
  /* NOMObjectData* nomThis = NOMObjectGetData(nomSelf); */
  
  if(TRUE==bIncludingParents)
    g_message("Flag ˚bIncludeParents˚ not supported yet");
  
  ncPriv=(NOMClassPriv*)_nomGetObjectCreateInfo(_nomQueryClass(nomSelf, NULL), NULL);
  
  if(ncPriv){
    gulong a, ulNumIntroducedMethods;

    ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
    for(a=0;a< ulNumIntroducedMethods;a++)
    {
      NOMMethod* nMethod=NOMMethodNew();
      
      _initData(nMethod, (gpointer) &ncPriv->sci->nomSMethods[a], NULL);
      
      NOMArray_append(nomArray, nMethod, NULL);
    }
  }
  return nomArray;
}

