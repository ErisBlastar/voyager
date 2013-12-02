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
/*
 * And remember, phase 3 is near...
 */
/** \file nomobj.idl 
    
    Implementation file for the NOM class NOMClassMgr.
*/

#ifndef NOM_NOMClassMgr_IMPLEMENTATION_FILE
#define NOM_NOMClassMgr_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
# define INCL_DOS
# include <os2.h>
#endif /* __OS2__ */

#include <string.h>
#ifdef NOM_WITH_GTK
# include <gtk/gtk.h>
#endif


#include "nom.h"
#include "nomtk.h"

#include "nomclassmanager.ih"


/**
   \brief Function which implements the nomFindClassFromID() method of NOMClassMgr.

   \remark This method isn't implemented yet.
 */
NOM_Scope NOMClass* NOMLINK impl_NOMClassMgr_nomFindClassFromId(NOMClassMgr* nomSelf,
                                                                const gulong classId,
                                                                const gulong ulMajorVersion,
                                                                const gulong ulMinorVersion,
                                                                CORBA_Environment *ev)
{
  /*  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf); */
  CORBA_Object nomRetval;

  nomPrintf("%s: %s not implemented yet (line %d)\n", __FILE__, __FUNCTION__, __LINE__);
  nomRetval=NULL;
  return nomRetval;
}

/**
   \brief Function which implements the nomFindClassFromName() method of NOMClassMgr.
 */
NOM_Scope NOMClass* NOMLINK impl_NOMClassMgr_nomFindClassFromName(NOMClassMgr* nomSelf, 
                                                                  const CORBA_char * className,
                                                                  const CORBA_long ulMajorVersion,
                                                                  const CORBA_long ulMinorVersion,
                                                                  CORBA_Environment *ev)
{
  CORBA_Object nomRetval=NULL;
  nomMethodTab * mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  /* This is only for NOMClass objects */
  if(strchr(className, ':'))
    return NULL;

  mtab=g_datalist_get_data(&_gdataClassList, className);

  if(mtab){
    NOMClassPriv* ncPriv;
    ncPriv=(NOMClassPriv*)mtab->nomClsInfo;
    nomRetval=(CORBA_Object)ncPriv->sci->nomCds->nomClassObject;
  }
  return nomRetval;
}

/**
  This function is called when a class is (accidently) removed from our class list.
  This may happen e.g. when a class is registered again using the same name. The old
  registration is removed then and the new inserted (which may be the very same data).
  When this happens this is presumably a bug:
 */
static void
priv_handleClassRemove(gpointer data)
{
  nomMethodTab* mtab;
  mtab=(nomMethodTab*)data;
  if(mtab)
    g_warning("%s: supposed to remove class %s mtab: %lx. This is probably a bug.\n",
              __FUNCTION__, mtab->nomClassName, (long)mtab);
}

/**
   \brief Function which implements the nomRegisterClass() method of NOMClassMgr.

   We register mtabs as unique pointers to classes. It's possible to get every
   information from an mtab.
 */
NOM_Scope void NOMLINK impl_NOMClassMgr_nomRegisterClass(NOMClassMgr* nomSelf, const gpointer classMtab,
                                                         CORBA_Environment *ev)
{
  nomMethodTab* mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  mtab=(nomMethodTab*) classMtab;

  g_datalist_set_data_full(&_gdataClassList, mtab->nomClassName, classMtab, priv_handleClassRemove);
  g_tree_insert(_pClassListTree, mtab, mtab->nomClassName); /* key is the mtab because we want to use
                                                               this tree for fast lookup of mtabs to 
                                                               check for valid objects. */

  //nomPrintf("%s: registering %lx, %s classList: %lx ID: %ld\n", __FUNCTION__, 
  //        classMtab, mtab->nomClassName, _gdataClassList, g_quark_try_string(mtab->nomClassName));
  mtab->classNomId=g_quark_try_string(mtab->nomClassName);
}


/**
   \brief Function which implements the nomGetClassList() method of NOMClassMgr.
 */
NOM_Scope gpointer NOMLINK impl_NOMClassMgr_nomGetClassList(NOMClassMgr* nomSelf, CORBA_Environment *ev)
{
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  return (gpointer) _gdataClassList;
}


NOM_Scope gpointer NOMLINK impl_NOMClassMgr_nomGetClassInfoPtrFromName(NOMClassMgr* nomSelf,
                                                                       const CORBA_char * className,
                                                                       CORBA_Environment *ev)
{
  nomMethodTab * mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  /* This is only for NOMClassPriv objects */
  if(strchr(className, ':'))
    return NULL;

  mtab=g_datalist_get_data(&_gdataClassList, className);
  if(mtab)
    return mtab->nomClsInfo;
  else
    return NULL;
}


/**
  This function is called when a method is (accidently) removed from our list.
  This may happen e.g. when a class is registered again using the same name. The old
  registration is removed then and the new inserted (which may be the very same data).
  When this happens this is presumably a bug:
 */
static void
priv_handleMethodRemoveFromList(gpointer data)
{
  nomMethodTab* mtab;
  mtab=(nomMethodTab*)data;
  if(mtab)
    g_warning("%s: supposed to remove method mtab: %lx. This is probably a bug.\n",
              __FUNCTION__, (long)mtab);
}

/**
   \brief Function which implements the nomRegisterMethod() method of NOMClassMgr.
 */
NOM_Scope void NOMLINK impl_NOMClassMgr_nomRegisterMethod(NOMClassMgr* nomSelf, 
                                                          const gpointer classMtab, 
                                                          const CORBA_char * chrMethodName,
                                                          CORBA_Environment *ev)
{
  nomMethodTab* mtab;
  NOMClassMgrData *nomThis = NOMClassMgrGetData(nomSelf);

  mtab=(nomMethodTab*) classMtab;

  g_datalist_set_data_full(&_gdataMethodList, chrMethodName, classMtab, priv_handleMethodRemoveFromList);
}

/**
   \brief Function which implements the nomIsObject() method of NOMClassMgr.
 */
NOM_Scope CORBA_boolean NOMLINK impl_NOMClassMgr_nomIsObject(NOMClassMgr* nomSelf, const PNOMObject nomObject,
                                                             CORBA_Environment *ev)
{
  NOMClassMgrData* nomThis=NOMClassMgrGetData(nomSelf);

  if(NULL==nomObject)
    return FALSE;

  return (g_tree_lookup(_pClassListTree, nomObject->mtab)!= NULL); 
}

/**
   \brief Function which implements the nomSubstituteClass() method of NOMClassMgr.
 */
NOM_Scope CORBA_boolean NOMLINK impl_NOMClassMgr_nomSubstituteClass(NOMClassMgr* nomSelf,
                                                                    const CORBA_char * oldClass, 
                                                                    const CORBA_char * replacementClass,
                                                                    CORBA_Environment *ev)
{
/* NOMClassMgrData* nomThis=NOMClassMgrGetData(nomSelf); */
  NOMClass* oClass;
  NOMClass* rClass;
  NOMClassPriv* ncp;

  /* This returns a meta class */
  if((oClass=_nomFindClassFromName( nomSelf, oldClass, 0, 0, NULL))==NULL)
    return FALSE;

  if((rClass=_nomFindClassFromName( nomSelf, replacementClass, 0, 0, NULL))==NULL)
    return FALSE;

  /* Check if the class is a direct child */
  ncp=(NOMClassPriv*)_nomGetObjectCreateInfo(rClass, NULL);
  
  if(strcmp(ncp->parentMtabStruct.next->mtab->nomClassName, oldClass))
    return FALSE;

  /* Save old class object pointer. Hmm, maybe not it's still in the old parentMtab */

  /* Change the class object pointer in the nomClassDataStructure */

  /* Reregister old class with new mtab in the internal list. Make sure we don't get
     a warning by GLib */

  return FALSE;
}

NOM_Scope CORBA_string NOMLINK impl_NOMClassMgr_nomQueryDLLForClass(NOMClassMgr* nomSelf,
                                                                    const CORBA_char * chrClassName,
                                                                    CORBA_Environment *ev)
{
/* NOMClassMgrData* nomThis=NOMClassMgrGetData(nomSelf); */
#if 0
  /* We have a circular reference here to the voyfcls.dll containing NOMPath, which needs nomtk.a
     to be built. */
  NOMPath* thePath;

  if(!chrClassName)
    return NULL;

  if((thePath=NOMPathNew())==NULL)
    return NULL;

  /* We probably use the home dir later */
  thePath=NOMPath_assignCString(thePath, g_get_current_dir(), NULL);
  thePath=NOMPath_appendCString(thePath, "classlist.ini", NULL);
#endif

  return NULL;
}


static
int nomClassMgrCompareFunc(gconstpointer a, gconstpointer b)
{
  if(a < b )
    return -1;
  if(a > b )
    return 1;
  return 0;
}
/**
   \brief Function which implements the nomInit() override NOMClassMgr.
 */
NOM_Scope void NOMLINK impl_NOMClassMgr_nomInit(NOMClassMgr* nomSelf, CORBA_Environment *ev)
{
  NOMClassMgrData* nomThis=NOMClassMgrGetData(nomSelf);

  g_message("In %s", __FUNCTION__);
  NOMClassMgr_nomInit_parent((NOMObject*)nomSelf,  ev);

  g_datalist_init(&_gdataMethodList);
  g_datalist_init(&_gdataClassList);

  /* This balanced binary tree holds the objects in this folder. We create a tree
     which may be searched using the name of the file/directory */

  _pClassListTree=g_tree_new((GCompareFunc)nomClassMgrCompareFunc);

}












