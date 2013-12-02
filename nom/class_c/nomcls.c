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
/** \file noncls.c
    
    Implementation file for the NOMClass class.
*/
#ifndef NOM_NOMClass_IMPLEMENTATION_FILE
#define NOM_NOMClass_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
# define INCL_DOS
# include <os2.h>
#endif /* __OS2__ */

#include <string.h>
#include <stdlib.h>
#ifdef NOM_WITH_GTK
# include <gtk/gtk.h>
#endif 


#include "nom.h"
#include "nomtk.h"

#include "nomcls.ih"
#include "nomclassmanager.h"

#include "gc.h"

extern NOMClassMgr* NOMClassMgrObject;

static void nomObjectFinalizer(GC_PTR obj, GC_PTR client_data)
{
  NOMObject* nObj;
  nObj=(NOMObject*)obj;

  if(nomIsObj(nObj)){
    //nomPrintf("Finalizing 0x%x: %s \n", nObj, nObj->mtab->nomClassName);
    _nomUnInit(nObj, NULL);
  }
  //else
  //nomPrintf("Finalizing 0x%x: no object! \n", nObj);
}

/**
   \brief Function which implements the nomNew() method of NOMClass.
 */
NOM_Scope PNOMObject NOMLINK impl_NOMClass_nomNew(NOMClass* nomSelf, CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);
  NOMClassPriv *ncp;
  gchar* nObj;

  if(!_ncpObject)
    return NULL;

  ncp=(NOMClassPriv*)_ncpObject;

  /* Allocate memory big enough to hold an object. This means the size is that of the
     mtab pointer and all instance variables. */
  if((nObj=_nomAllocate(nomSelf, ncp->mtab->ulInstanceSize, NULL))==NULL)
    return NULL;

  /* _nomInit() is called in _nomRenew() */
  return _nomRenew(nomSelf, (CORBA_Object)nObj, NULL); /* This will also init the object */
}

/**
   \brief Function which implements the nomRenewNoInit() method of NOMClass.
 */
NOM_Scope PNOMObject NOMLINK impl_NOMClass_nomRenewNoInit(NOMClass* nomSelf,
                                                          const gpointer nomObj,
                                                          CORBA_Environment *ev)
{
  NOMClassPriv *ncp;
  NOMClassData *nomThis = NOMClassGetData(nomSelf);
  GC_PTR oldData;
  GC_finalization_proc oldFinalizerFunc;

  ncp=(NOMClassPriv*)_ncpObject;

  /* Docs say the memory is first zeroed */
  memset(nomObj, 0, ncp->mtab->ulInstanceSize);

  /* Set mtab so it's an object */
  ((NOMObject*)nomObj)->mtab=ncp->mtab;

  /* Register finalizer if the class uses nomUnInit */
  ncp=(NOMClassPriv*)ncp->mtab->nomClsInfo;
  if(ncp->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN){
    //nomPrintf("Registering finalizer for %s\n", ((NOMObject*)nomObj)->mtab->nomClassName);
    GC_register_finalizer(nomObj,  nomObjectFinalizer, NULL, &oldFinalizerFunc, &oldData);
  }
  /* NO call to _nomInit() */

  /* Return statement to be customized: */
  return nomObj;
}

/**
   \brief Function which implements the nomRenew() method of NOMClass.
 */
NOM_Scope PNOMObject NOMLINK impl_NOMClass_nomRenew(NOMClass* nomSelf, const gpointer nomObj,
                                                    CORBA_Environment *ev)
{
#if 0
  CORBA_Environment tempEnv={0};
  tempEnv.fFlags=NOMENV_FLG_DONT_CHECK_OBJECT;
#endif
  _nomRenewNoInit(nomSelf, nomObj, NULL);

  /* And now give the object the possibility to initialize... */
  /* Make sure the object is not checked. */
  //_nomInit((NOMObject*)nomObj, &tempEnv);
  _nomInit((NOMObject*)nomObj, NULL);

  return nomObj;
}


/**
   \brief Function which implements the nomAllocate() method of NOMClass.
 */
NOM_Scope gpointer NOMLINK impl_NOMClass_nomAllocate(NOMClass* nomSelf, const CORBA_long ulSize,
                                                         CORBA_Environment *ev)
{
  /* NOMClassData *nomThis = NOMClassGetData(nomSelf); */

  return NOMMalloc(ulSize);
}

/**
   Function which implements the nomQueryCreatedClassName() method of NOMClass.
*/
NOM_Scope CORBA_string NOMLINK impl_NOMClass_nomQueryCreatedClassName(NOMClass* nomSelf, CORBA_Environment *ev)
{
  NOMClassPriv* ncp;

  /* The private struct characterizing the objects this meta class can create. */
  ncp=_nomGetObjectCreateInfo(nomSelf, NULL);

  if(!ncp)
    return ""; /* That can not happen but anyway... */

  //  return nomSelf->mtab->nomClassName;
  return ncp->mtab->nomClassName;
}


/**
   \brief Function which implements the nomDeallocate() method of NOMClass.
 */
NOM_Scope void NOMLINK impl_NOMClass_nomDeallocate(NOMClass* nomSelf, const gpointer memptr, CORBA_Environment *ev)
{
  NOMFree((nomToken)memptr);
}


/**
   \brief Function which implements the nomSetObjectCreateInfo() method of NOMClass.
 */
NOM_Scope void NOMLINK impl_NOMClass_nomSetObjectCreateInfo(NOMClass* nomSelf, const gpointer ncpObject,
                                                            CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);

  _ncpObject=ncpObject;
}

/**
   \brief Function which implements the nomGetObjectCreateInfo() method of NOMClass.
 */
NOM_Scope gpointer NOMLINK impl_NOMClass_nomGetObjectCreateInfo(NOMClass* nomSelf, CORBA_Environment *ev)
{
  NOMClassData* nomThis=NOMClassGetData(nomSelf);

  return _ncpObject;
}

/**
   \brief Function which implements the nomClassReady() method of NOMClass.
 */
NOM_Scope void NOMLINK impl_NOMClass_nomClassReady(NOMClass* nomSelf, CORBA_Environment *ev)
{
  CORBA_Environment tempEnv={0};
  tempEnv.fFlags=NOMENV_FLG_DONT_CHECK_OBJECT;

  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
            __FUNCTION__, nomSelf, nomSelf->mtab->nomClassName);

  /* Register the class object if not alread done */
  if(NULL!=NOMClassMgrObject)
    {
      /* First check if this class is already in the list. This is important
         because there may be several classes having the same metaclass (usually NOMClass).
         The only difference of these metaclasses are different object templates for creating
         the instances. So to prevent unnecessary growth of the list just insert one class
         of each type. This should be sufficient for now and the future.

         We insert the mtab pointer which is an anchor to all other class info we may
         be interested in. */

      /* FIXME: no use of version information, yet. */
      
      //The following was used before changing nomGetname()
      //if(!_nomFindClassFromName(NOMClassMgrObject, _nomGetName(nomSelf, NULL),
      //                        0, 0, NULL))
      if(!_nomFindClassFromName(NOMClassMgrObject, _nomQueryClassName(nomSelf, &tempEnv),
                              0, 0, &tempEnv))

        {
          NOMClassPriv* ncPriv;
          gulong a, ulNumIntroducedMethods;

          nomPrintf("%s: Metaclass not registered yet.\n", __FUNCTION__);

          /* No, not in the list, so register it */
          ncPriv=(NOMClassPriv*)nomSelf->mtab->nomClsInfo;
          //ncPriv->ulIsMetaClass=1; /* Mark that we are a metaclass */
          ncPriv->ulClassFlags|=NOM_FLG_IS_METACLASS; /* Mark that we are a metaclass */
          _nomRegisterClass(NOMClassMgrObject, nomSelf->mtab, NULL);
          /* Register all the methods this class introduces */
          ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
          for(a=0;a<ulNumIntroducedMethods;a++)
            {
              if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                _nomRegisterMethod(NOMClassMgrObject, nomSelf->mtab,
                                   *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULL);
            }
          /* Metaclass is registered. Register the object class this
             metaclass may create. */
          ncPriv=(NOMClassPriv*)_nomGetObjectCreateInfo(nomSelf, NULL);//nomSelf->mtab->nomClsInfo;
          if(ncPriv){
#ifndef _MSC_VER
#warning !!!!! NOMClass does not have this pointer, this is a bug !!!!!
#endif
            //ncPriv->ulIsMetaClass=0; /* Mark that we are not a metaclass (should be 0 already) */
            ncPriv->ulClassFlags&=~NOM_FLG_IS_METACLASS; /* Mark that we are not a metaclass (should be 0 already) */
            _nomRegisterClass(NOMClassMgrObject, ncPriv->mtab, NULL);

            /* Register all the methods this class introduces */
            ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
            for(a=0;a<ulNumIntroducedMethods;a++)
              {
                if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                  _nomRegisterMethod(NOMClassMgrObject, ncPriv->mtab,
                                     *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULL);
              }/* for(a) */
          }/* ncPriv */
        }
      else{
        NOMClassPriv* ncPriv;
        /* Metaclass is already registered. Register the object class this
           metaclass may create. */

        nomPrintf("%s (%d): Metaclass already registered, registering normal object class now.\n", __FUNCTION__, __LINE__);

        ncPriv=(NOMClassPriv*)_nomGetObjectCreateInfo(nomSelf, NULL);//nomSelf->mtab->nomClsInfo;
        
        if(ncPriv){
          gulong a, ulNumIntroducedMethods;

          // ncPriv->ulIsMetaClass=0; /* Mark that we are not a metaclass (should be 0 already) */
          ncPriv->ulClassFlags&=~NOM_FLG_IS_METACLASS; /* Mark that we are not a metaclass (should be 0 already) */

          if(_nomFindClassFromName(NOMClassMgrObject, ncPriv->mtab->nomClassName,
                                   0, 0, &tempEnv))
            nomPrintf("%s: %s already registered\n", __FUNCTION__, ncPriv->mtab->nomClassName);
          
          /* Register all the methods this class introduces */
          ulNumIntroducedMethods=ncPriv->sci->ulNumStaticMethods;
          for(a=0;a<ulNumIntroducedMethods;a++)
            {
              if(*ncPriv->sci->nomSMethods[a].chrMethodDescriptor)
                _nomRegisterMethod(NOMClassMgrObject, ncPriv->mtab,
                                   *ncPriv->sci->nomSMethods[a].chrMethodDescriptor, NULL);
            }
          //nomPrintf("%s %s %s \n", __FUNCTION__, nomSelf->mtab->nomClassName, ncPriv->mtab->nomClassName);
          _nomRegisterClass(NOMClassMgrObject, ncPriv->mtab, NULL);
        }
      }
    }
  else
    nomPrintf("%s: no NOMClassMgrObject yet.\n", __FUNCTION__);

}

NOM_Scope void NOMLINK impl_NOMClass_nomInit(NOMClass* nomSelf, CORBA_Environment *ev)
{
/* NOMClassData* nomThis=NOMClassGetData(nomSelf); */
#if 0
  CORBA_Environment tempEnv={0};
  tempEnv.fFlags=NOMENV_FLG_DONT_CHECK_OBJECT;
#endif
  //  nomPrintf("    Entering %s  with nomSelf: 0x%x. nomSelf is: %s.\n",
  //         __FUNCTION__, nomSelf, nomSelf->mtab->nomClassName);

  /* Don't check object pointer. We are just created but not yet registered as a class. */
  //  NOMClass_nomInit_parent(nomSelf,  &tempEnv);
  NOMClass_nomInit_parent(nomSelf, NULL);
}




