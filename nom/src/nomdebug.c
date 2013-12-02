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

#ifdef __OS2__
# define INCL_DOS
# define INCL_DOSFILEMGR
# define INCL_DOSERRORS
# define INCL_WIN
# define INCL_WINWORKPLACE
# define INCL_OS2MM
# define INCL_MMIOOS2
# define INCL_MCIOS2
# define INCL_GPI
# define INCL_PM
# include <os2.h>
#endif /* __OS2__ */

#include <stdarg.h>
#include <stdio.h>

#include "nom.h"
#include "nomtk.h"
#include "nomobj.h"
#include "nomclassmanager.h"

//#include "cwsomcls.h"
extern NOMClassMgr* NOMClassMgrObject;
extern gboolean fInitialized;

NOMEXTERN void NOMLINK nomPrintObjectPointerErrorMsg(NOMObject*  nomObject, NOMClass* nomClass, gchar* chrMethodName)
{
  if(!nomObject)
    g_error("The object used to call the method %s is not valid. A NULL pointer was given.", chrMethodName);
  else{
    if(!nomIsObj(nomObject))
      g_error("The object used to call the method %s is not a valid NOM object. ", chrMethodName);
    else
      g_error("The object for which the method %s should be called is not valid for this method.\nThe object must be some instance of class %s (or of a subclass) but is a %s.", chrMethodName, NOMClass_nomQueryCreatedClassName(nomClass, NULL), 
                NOMObject_nomQueryClassName(nomObject, NULL));
  }
}

/*
  This function prints some more info about the object error. It's used for generic checks which
  always return NULL which isn't always correct.
 */
static void nomPrintAdditionalErrorMsg(void)
{
  g_message("Note that NULL is returned for the call (if the method returns a value). This may not be correct. Use the NOMPARMCHECK() macro to specify default return values for methods.");
}

/* Function to check if  NOMObject is valid before calling a method on it. Note that we don't have to check
   the instance class here using nomIsA*(). */
NOMEXTERN gboolean NOMLINK nomCheckNOMObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev)
{
  /* Not initialized yet, so object check won't work. This means the three core NOM classes are not
     yet created.*/
  if(!fInitialized)
    return TRUE;
  
  if(ev && (ev->fFlags & NOMENV_FLG_DONT_CHECK_OBJECT))
    return TRUE;

  //  g_message("In %s with %s %px nomClass: %px (%s)", __FUNCTION__, chrMethodName, nomSelf, nomClass, nomClass->mtab->nomClassName);
  if(!nomIsObj(nomSelf))
    {
      nomPrintObjectPointerErrorMsg(nomSelf, nomClass, chrMethodName);
      nomPrintAdditionalErrorMsg();
      return FALSE;
    }
  return TRUE;
}

#include <string.h>
/* Function to check if an object is valid before calling a method on it */
NOMEXTERN gboolean NOMLINK nomCheckObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev)
{
  /* Not initialized yet, so object check won't work. This means the three core NOM classes are not
     yet created.*/
  if(!fInitialized)
    return TRUE;

  if(ev && (ev->fFlags & NOMENV_FLG_DONT_CHECK_OBJECT))
    return TRUE;

  //  g_message("In %s with %s %px nomClass: %px (%s)", __FUNCTION__, chrMethodName, nomSelf, nomClass, nomClass->mtab->nomClassName);
  if(!nomIsObj(nomSelf) || !_nomIsANoClsCheck(nomSelf, nomClass, NULL))
    {
      nomPrintObjectPointerErrorMsg(nomSelf, nomClass, chrMethodName);
      nomPrintAdditionalErrorMsg();
      return FALSE;
    }
  return TRUE;
}

NOMEXTERN CORBA_Environment* NOMLINK nomCreateEnvNoObjectCheck(void)
{
  CORBA_Environment * tempEnv=(CORBA_Environment*)NOMMalloc(sizeof(CORBA_Environment));
  if(tempEnv)
    tempEnv->fFlags|=NOMENV_FLG_DONT_CHECK_OBJECT;
  return tempEnv;
}
 
static void dumpClassFunc(GQuark gquark, gpointer data, gpointer user_data)
{
  nomMethodTab* mtab;
  NOMClassPriv* priv;

  mtab=(nomMethodTab*)data;
  priv=(NOMClassPriv*)mtab->nomClsInfo;

  if(!priv)
    nomPrintf("%s: gquark: %ld, %lx-> %s mtab: %lx, nomClsInfo->mtab: %lx\n",
              __FUNCTION__, gquark, data, mtab->nomClassName, mtab, priv/*->mtab*/);
  else
    nomPrintf("%s: gquark: %ld, %lx-> %s mtab: %lx, nomClsInfo->mtab: %lx\n",
              __FUNCTION__, gquark, data, mtab->nomClassName, mtab, priv->mtab);

}

NOMEXTERN  void NOMLINK dumpClasses(void)
{
  GData *pgdata;

  nomPrintf("----- %s -----  NOMClassMgrObject: %lx\n", __FUNCTION__, NOMClassMgrObject);

  pgdata=(GData*)_nomGetClassList(NOMClassMgrObject, NULL);
  if(pgdata){
    nomPrintf("%s: classlist: %lx\n", __FUNCTION__, pgdata);
    g_datalist_foreach(&pgdata, dumpClassFunc, pgdata);
  }
  nomPrintf("----- End of %s -----\n", __FUNCTION__);
}

void _dumpSci(nomStaticClassInfo* sci)
{
  nomPrintf("%d: --- sci dump ---:\n", __LINE__);
  nomPrintf("&sci: 0x%x\n", sci);
  nomPrintf("Num static methods: %d\n", sci->ulNumStaticMethods);
  nomPrintf("Num static overrides: %d\n", sci->ulNumStaticOverrides);
  nomPrintf("Major: %d, minor %d\n", sci->ulMajorVersion, sci->ulMinorVersion);
  nomPrintf("Instance data size: %d\n", sci->ulInstanceDataSize);
  //somPrintf("Max methods: %d\n", sci->maxMethods);
  nomPrintf("numParents: %d\n", sci->ulNumParents);
  nomPrintf("classId: %s\n", *sci->nomClassId);
  if(sci->nomExplicitMetaId)
    nomPrintf("explicitMetaId (meta class): %s\n", *sci->nomExplicitMetaId);
  else
    nomPrintf("explicitMetaId (meta class): NULL\n");
  //  somPrintf("*parents: 0x%x\n", sci->parents);
  nomPrintf("somClassDataStructure cds: 0x%x\n",sci->nomCds);
  if(sci->nomCds)
   nomPrintf("classObject: 0x%x\n", sci->nomCds->nomClassObject);
  //somPrintf("somCClassDataStructure ccds: 0x%x\n", sci->ccds);
  nomPrintf("Static method table (nomStaticMethodDesc): 0x%x\n", sci->nomSMethods);
  //somPrintf("Override method table (somOverrideMethod_t): 0x%x\n", sci->omt);

  nomPrintf("------------------\n");

}


void _dumpClassDataStruct(nomClassDataStructure* cds, gulong ulNumMethods)
{
  int a;

  nomPrintf("%d: --- classdataStruct dump num methods: %d ---:\n", __LINE__,  ulNumMethods);
  nomPrintf("classObject: %x\n",  cds->nomClassObject);
  for(a=0;a<ulNumMethods; a++)
    nomPrintf("%d: %x at address %x\n", a+1, cds->nomTokens[a], &cds->nomTokens[a]);

  nomPrintf("-----------------------------\n");
}

void  _dumpMtab(nomMethodTab* mtab)
{
  int a;
  nomMethodProc* entries;

  nomPrintf("\n--- mtab dump for %x, %s (%s) ---\n", mtab, mtab->nomClassName, __FUNCTION__);
  nomPrintf("  mtab: 0x%x\n", mtab);
  nomPrintf("  classObject: 0x%x\n", mtab->nomClassObject);
  nomPrintf("  classInfo: 0x%x\n", mtab->nomClsInfo);
  // somPrintf("*classInfo: 0x%x\n", *sObj->mtab->classInfo);
  nomPrintf("  className: %s\n", mtab->nomClassName);
  nomPrintf("  instanceSize (this and all parents): %d 0x%x\n", mtab->ulInstanceSize, mtab->ulInstanceSize);
  nomPrintf("  mtabSize: %d 0x%x\n", mtab->mtabSize, mtab->mtabSize);
  nomPrintf("  somMethodProcs: (%ld)\n", (mtab->mtabSize-(glong)sizeof(nomMethodTab))/4);
  //  entries=sObj->mtab->entries[0];
   for(a=0; a<=(mtab->mtabSize-sizeof(nomMethodTab))/4; a++)
    nomPrintf("  %d: somMethodProc: 0x%x at 0x%x\n", a, mtab->entries[a], &mtab->entries[a]);
  nomPrintf("--- end of mtab dump ---\n");
}


void  _dumpObjShort(NOMObject* sObj)
{

  nomPrintf("--- Object dump (mtab) for %s ---\n", sObj->mtab->nomClassName);
  nomPrintf("Obj: 0x%x\n", sObj);
  nomPrintf("SOM_Any->mtab: 0x%x\n", sObj->mtab);
  nomPrintf("classObject: 0x%x\n", sObj->mtab->nomClassObject);
  nomPrintf("classInfo: 0x%x\n", sObj->mtab->nomClsInfo);
#if 0
  nomPrintf("*classInfo: 0x%x\n", *sObj->mtab->nomClsInfo);

  so=(gulong*)sObj->mtab->classInfo;
#endif
  nomPrintf("className: %s\n", sObj->mtab->nomClassName);
  nomPrintf("instanceSize (this and all parents): %d 0x%x\n", sObj->mtab->ulInstanceSize, sObj->mtab->ulInstanceSize);
  nomPrintf("mtabSize: %d 0x%x\n", sObj->mtab->mtabSize, sObj->mtab->mtabSize);
  nomPrintf("nomMethodProcs: (%d)\n", (sObj->mtab->mtabSize-sizeof(nomMethodTab))/4);
  nomPrintf("classObject in entries[]: 0x%x\n", sObj->mtab->entries[0]);
  //  entries=sObj->mtab->entries[0];
  //   for(a=0; a<=(sObj->mtab->mtabSize-sizeof(somMethodTab))/4; a++)
  //  somPrintf("%d: somMethodProc: 0x%x\n", a, sObj->mtab->entries[a]);
  nomPrintf("---------\n");
}


#if 0
void _dumpStaticMTab(somStaticMethod_t* smt, gulong ulMethod)
{
  somStaticMethod_t* tmpSmt=&smt[ulMethod];

  somPrintf("--- static method dump for 0x%x, method index: %d ---:\n", smt, ulMethod);
  somPrintf("classData: %x\n", tmpSmt->classData);
  somPrintf("methodId: %s\n", **tmpSmt->methodId);
  somPrintf("methodDescriptor: %s\n", **tmpSmt->methodDescriptor);
  somPrintf("method: 0x%x\n", tmpSmt->method);

  somPrintf("-----------------------------\n");
}

void _dumpOverrideMTab(somOverrideMethod_t* omt, gulong ulMethod)
{
  somOverrideMethod_t* tmpOmt=&omt[ulMethod];

  somPrintf("--- override method dump for 0x%x, method index: %d ---:\n", omt, ulMethod);
  somPrintf("classId: %s\n", **tmpOmt->methodId);
  //  somPrintf("methodDescriptor: %s\n", **tmpOmt->methodDescriptor);
  somPrintf("method: 0x%x\n", tmpOmt->method);

  somPrintf("-----------------------------\n");
}

void SOMLINK _dumpParentMTab(somParentMtabStructPtr pMtabPtr)
{

  somPrintf("--- ParentMtabStruct dump for 0x%x (%s) ---:\n", pMtabPtr, pMtabPtr->mtab->className);
  somPrintf("this mtab:\t\t\t%x\n", pMtabPtr->mtab);
  somPrintf("somMethodTabs next:\t\t%x\n", pMtabPtr->next);
  if(pMtabPtr->next) {
    somPrintf("parent mtab (next->mtab):\t%x\n", pMtabPtr->next->mtab);
    somPrintf(" next->next:\t\t\t%x\n", pMtabPtr->next->next);
  }
  else
    somPrintf(" NO parent mtab\n");
  somPrintf("classObject:\t\t\t0x%x\n", pMtabPtr->classObject);
  somPrintf("instanceSize:\t\t\t%d 0x%x\n", pMtabPtr->instanceSize, pMtabPtr->instanceSize);
  somPrintf("-----------------------------\n");
}

void _dumpParentMTabList(somParentMtabStructPtr pMtabPtr)
{
  somParentMtabStructPtr tempPtr=pMtabPtr;

  while(tempPtr){
    _dumpParentMTab(tempPtr);
    if(tempPtr->next)
      tempPtr=(void*)tempPtr->next->mtab;
    else
      tempPtr=NULL;
  }

}

void _dumpMethodTabList(somMethodTabList *mtabList)
{
  somMethodTab *mtab;
  int a=0;

  somPrintf("--- somMethodTabList dump for 0x%x  ---:\n", mtabList);
  while(mtabList && a<3)
    {
      mtab=mtabList->mtab;

      if(mtab)
        somPrintf("  %d: mtab: %x , class: %s SOM class object: %x, priv object: %x, next: %x\n", 
                  a, mtab, mtab->className, mtab->classObject, mtab->classInfo, mtabList->next);
      else
        somPrintf("mtab is 0!!\n");

      mtabList=mtabList->next;
      a++;
    }
  somPrintf("-----------------------------\n");
}

void SOMLINK _dumpMTabListPrivClass(SOMClassPriv *sClass)
{
  somMethodTabList *mtabList;
  somMethodTab *mtab;
  int a=0;

  if(!sClass) {
    somPrintf("--- somMethodTabList dump: no SOMClassPriv*  ");
    return;
  }
  mtabList=&sClass->mtabList;

  somPrintf("\n--- somMethodTabList dump for 0x%x %s (%s) ---:\n", sClass, sClass->mtab->className, __FUNCTION__);
  while(mtabList)
    {
      mtab=mtabList->mtab;
      somPrintf("  %d: mtab: %x , class: %s SOM class object: %x, priv object: %x, next %x\n", 
                a, mtab, mtab->className, mtab->classObject, mtab->classInfo, mtabList->next);
      mtabList=mtabList->next;
      a++;
    }
  somPrintf("--- somMethodTabList end ---\n");
}

  /* Dump object data. This means more or less dumping of the methodTabStruct of this
     object. A pointer to this struct is the first word of any object (SOMAny.mtab).

     -- Object Instance Structure:

     struct somMethodTabStruct;
     typedef struct SOMAny_struct {
     struct somMethodTabStruct  *mtab;
     integer4 body[1];
     } SOMAny;

typedef struct somMethodTabStruct {
    SOMClass        *classObject;
    somClassInfo    *classInfo;
    char            *className;
    long            instanceSize;
    long            dataAlignment;
    long            mtabSize;
    long            protectedDataOffset; // from class's introduced data
    somDToken       protectedDataToken;
    somEmbeddedObjStruct *embeddedObjs;
    // remaining structure is opaque 
    somMethodProc* entries[1];           <-- I found that this isn't correct (or I misseed something in the includes). When dumping a mtab
                                             the following has this structure:
                                  
                                             SOMClass         *classObject; //The first class object (SOMObject)
                This is basically a copy ->  somMethodProc*   firstMethod_1;
                of the ClassDataStruct       somMethodProc*   secondMEthod_1;
                                             ...
                                             SOMClass         *classObject; //The second class object
                ClassDataStruct of 2.    ->  somMethodProc*   firstMethod_2; 
                class                        somMethodProc*   secondMEthod_2;
                        
} somMethodTab, *somMethodTabPtr;
 */
void SOMLINK _dumpObj(SOMObject* sObj)
{
  int a;
  somMethodProc* entries;

  somPrintf("\n--- Object dump (mtab) for %s (%s) ---\n", sObj->mtab->className, __FUNCTION__);
  somPrintf("  Obj: 0x%x\n", sObj);
  somPrintf("  SOM_Any->mtab: 0x%x\n", sObj->mtab);
  somPrintf("  classObject: 0x%x\n", sObj->mtab->classObject);
  somPrintf("  classInfo: 0x%x\n", sObj->mtab->classInfo);
  // somPrintf("*classInfo: 0x%x\n", *sObj->mtab->classInfo);
  somPrintf("  className: %s\n", sObj->mtab->className);
  somPrintf("  instanceSize (this and all parents): %d 0x%x\n", sObj->mtab->instanceSize, sObj->mtab->instanceSize);
  somPrintf("  mtabSize: %d 0x%x\n", sObj->mtab->mtabSize, sObj->mtab->mtabSize);
  somPrintf("  somMethodProcs: (%ld)\n", (sObj->mtab->mtabSize-(glong)sizeof(somMethodTab))/4);
  //  entries=sObj->mtab->entries[0];
   for(a=0; a<=(sObj->mtab->mtabSize-sizeof(somMethodTab))/4; a++)
    somPrintf("  %d: somMethodProc: 0x%x at 0x%x\n", a, sObj->mtab->entries[a], &sObj->mtab->entries[a]);
  somPrintf("--- end of object dump ---\n");
}



#endif






























