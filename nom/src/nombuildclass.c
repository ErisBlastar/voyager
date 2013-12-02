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
/** \file
    This File contains most of the magic to create classes for NOM.
 */

#ifdef __OS2__
# define INCL_DOS
# define INCL_DOSERRORS
# include <os2.h>
#endif /* __OS2__ */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if !defined(_MSC_VER) && !defined(__FreeBSD__)
# include <alloca.h>
#endif
#include <glib.h>
#define SOM_NO_OBJECTS  /* Otherwise som.h includes the IBM SOM classes */

/* For somToken etc. */
#include <nom.h>
#include <nomtk.h>
#include <nomobj.h>
#include <nomcls.h>
#include <nomclassmanager.h>

#include <thunk.h>

/* Define if you want to have messages from nomBuildClass() and friends */
//#define DEBUG_NOMBUILDCLASS
/* Define if you want to have messages from building NOMObject */
//#define DEBUG_BUILDNOMOBJECT
/* Define if you want to have messages from building NOMClass */
//#define DEBUG_BUILDNOMCLASS

#ifdef DEBUG_BUILDNOMCLASS
    #define BUILDNOMCLASS_ENTER nomPrintf("\n%s line %d: *** entering %s...\n",__FILE__, __LINE__,  __FUNCTION__);
    #define BUILDNOMCLASS_LEAVE nomPrintf("%s line %d: *** Leaving %s...\n\n",__FILE__, __LINE__,  __FUNCTION__);
#else
    #define BUILDNOMCLASS_ENTER
    #define BUILDNOMCLASS_LEAVE
#endif

#ifdef _MSC_VER
void _inline DBG_NOMBUILDCLASS(gboolean a, const char fmt, ...)
{
    /* sorry nothing here. */
}
#else
# ifdef DEBUG_NOMBUILDCLASS
#  define DBG_BUILD_LINE(a)
#  define DBG_NOMBUILDCLASS(a, b,...)   if(a) nomPrintf("%d: " b , __LINE__,  __VA_ARGS__);
# else
#  define DBG_NOMBUILDCLASS(a, b,...)
# endif
#endif 

#define DBGBUILDNOMCLASS_ENTER BUILDNOMCLASS_ENTER
#define DBGBUILDNOMCLASS_LEAVE BUILDNOMCLASS_LEAVE

/********************************************************/
extern NOMClassMgr* NOMClassMgrObject;

extern PNOM_ENV pGlobalNomEnv;

/******************* somBuildClass **********************/

#if 0
/*
  Thunking code to get the instance var address from an object pointer pushed
  on the stack. The following translates into this assembler code:

  MOV EAX,DWORD PTR [ESP+4] ;Move object ptr into EAX
  ADD EAX, +4
  RET
*/

static gulong thunk[]={0x0424448b, 0x00000405, 0x0000c300};

/*
MOV ECX,DWORD PTR [ESP+4] : move object pointer from stack in ECX
MOV EDX,DWORD PTR [ECX]   : move [ECX] in EDX -> mtab in EDX
JMP DWORD PTR [EDX+0ACh]  : JMP to address pointing to by EDX+0ACh
 */
static gulong mThunkCode[]={0x04244c8b, 0xff00518b, 0x0000aca2 , 0x16000000};
#endif

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
/*
  FIXME:

  This is similar to nomIsA() of NOMObject so better use that later.

 */

static gboolean priv_nomIsA(NOMObject *nomSelf, NOMClass* aClassObj)
{
  nomParentMtabStructPtr pParentMtab=&((NOMClassPriv * )nomSelf)->parentMtabStruct;
  nomMethodTabs psmTab;

  if(!nomSelf||!aClassObj)
    return FALSE;
  
  /*
    FIXME: use nomIsObj here!!
    if(!nomIsObj(nomSelf)||!nomIsObj(aClassObj))
    return FALSE
    */
  
  if(!strcmp(nomSelf->mtab->nomClassName, aClassObj->mtab->nomClassName))
    return TRUE;
  
  psmTab=pParentMtab->next;
  while(psmTab) {
    if(!strcmp(psmTab->mtab->nomClassName, aClassObj->mtab->nomClassName))
      return TRUE;
    
    psmTab=psmTab->next;
  }
  
  /* Return statement to be customized: */
  return FALSE;
}

/*
  Helper function 
 */
static
gulong nomGetNumIntroducedMethods(NOMClassPriv* ncPriv)
{
  return ncPriv->sci->ulNumStaticMethods;
}

/*
  This function should be reworked later to just get the class object from the quark list
  instead of iterating over all parents.
 */
static
gulong priv_getIndexOfMethodInEntries(NOMClassPriv* nClass, nomStaticClassInfo *sci, gchar* chrMethodDescriptor)
{
  int a;
  gulong idx=0;
  gboolean bFound=FALSE;
  char* chrMethodName;

  /* We need the full info here, method name and class introducing it */
  if((chrMethodName=strchr(chrMethodDescriptor, ':'))==NULL)
    return 0;

  chrMethodName++;

  /* chrMethodName is like "ClassName:MethodName". */
  for(a=0; a<sci->ulNumParentsInChain; a++)
    {
      GString *gstr;

      gstr=g_string_new(sci->chrParentClassNames[a]);
      g_string_append_printf(gstr, ":%s", chrMethodName);

      // nomPrintf("Checking %s for %s\n", 
      //      sci->chrParentClassNames[a], gstr->str);
      if(g_quark_try_string(gstr->str))
        {
          int b;
          NOMClassPriv *ncPriv;
          gulong ulNumMethods;

          /* Found class introducing the method */
          //nomPrintf(" %s, %d: Found %s in %s\n", __FUNCTION__, __LINE__,
          //        gstr->str, sci->chrParentClassNames[a]);
          if(NULL==NOMClassMgrObject)
            g_error("%s line %d: No overriding for base classes yet (%s)!\n",
                    __FUNCTION__, __LINE__, gstr->str);
          idx++; /* This is the position for the class pointer */
          ncPriv=_nomGetClassInfoPtrFromName(NOMClassMgrObject, sci->chrParentClassNames[a], NULL);
          /* now find the index */
          ulNumMethods=nomGetNumIntroducedMethods(ncPriv);
          for(b=0;b<ulNumMethods;b++)
            {
              //  nomPrintf("%d, checking %s\n", b, *ncPriv->sci->nomSMethods[b].nomMethodId);
              if(!strcmp(chrMethodName,*ncPriv->sci->nomSMethods[b].nomMethodId ))
                {
                  //  nomPrintf("   %s, %d: Found %s in %s, index: %d\n", __FUNCTION__, __LINE__,
                  //        gstr->str, sci->chrParentClassNames[a], b);
                  idx+=b;
                  bFound=TRUE;
                  break;
                }
            }/* for(b) */
        }
      else
        {
          /* Class not yet found... */
          NOMClassPriv *ncPriv;

          if(NULL==NOMClassMgrObject){
            /* Things are getting dirty now. Someone tries overriding while we are
               bootstrapping. That may be either NOMClass or NOMClassMgr. Let's climb
               the list by hand now... */
            gulong aLoop, numMethods;

            /* We only support overriding of NOMObject for now when no manager is
               available. a is the index into the array of class names. a=0 is the
               first name which is always NOMObject. */
            if(0!=a)
              g_error("%s line %d: No Quark. No class manager and attempt to override a class which is not NOMObject\n(method: %s, current class: %s)!\n",
                      __FUNCTION__, __LINE__, gstr->str, sci->chrParentClassNames[a]);
            ncPriv=pGlobalNomEnv->ncpNOMObject;
            
            numMethods=nomGetNumIntroducedMethods(ncPriv);
            for(aLoop=0;aLoop<numMethods;aLoop++)
              {
                //nomPrintf("%d, checking %s\n", aLoop, *ncPriv->sci->nomSMethods[aLoop].nomMethodId);
                if(!strcmp(chrMethodName,*ncPriv->sci->nomSMethods[aLoop].nomMethodId ))
                  {
                    //nomPrintf("   %s, %d: Found %s in %s (hand search), index: %d\n", __FUNCTION__, __LINE__,
                    //        gstr->str, sci->chrParentClassNames[a], aLoop);
                    idx=aLoop+1;
                    bFound=TRUE;
                    break;
                  }
              }/* for(aLoop) */
          }
          else{
            ncPriv=_nomGetClassInfoPtrFromName(NOMClassMgrObject, sci->chrParentClassNames[a], NULL);
            //nomPrintf("   %s did not introduce the method. Adding %d to index\n",
            //        sci->chrParentClassNames[a], nomGetNumIntroducedMethods(ncPriv)+1);
            idx+=nomGetNumIntroducedMethods(ncPriv)+1; /* classObject pointer */
          }/* NOMClassMgrObject */
        }
      g_string_free(gstr, TRUE);
      if(bFound)
        break;
    }/* for(a) */

  if(bFound)
    return idx;
  else
    return 0;
}

/*
  
 */
void priv_resolveOverrideMethods(NOMClassPriv *nClass, nomStaticClassInfo *sci)
{
  if(sci->ulNumStaticOverrides) {
    int b;

    DBG_NOMBUILDCLASS(TRUE, "%d method(s) to override\n", sci->ulNumStaticOverrides);

    /* There're some methods to override */
    for(b=0;b<sci->ulNumStaticOverrides;b++) {/* For every overriden method do */
      nomMethodProc** entries;
      gulong index;

      entries=&nClass->mtab->entries[0];  /* Adress of array where we enter our resoved method */

      DBG_NOMBUILDCLASS(TRUE,"Going to override \"%s\"\n", *sci->nomOverridenMethods[b].nomMethodId);

      index=priv_getIndexOfMethodInEntries(nClass, sci, *sci->nomOverridenMethods[b].nomMethodId);
      if(0==index){
        g_warning("%s line %d:\n   We are supposed to override \"%s\" but the method can't be found!", 
                  __FUNCTION__, __LINE__, *sci->nomOverridenMethods[b].nomMethodId);
        return;
      }
      /* This is the parent method adress which will be used by the overriding class */
      *sci->nomOverridenMethods[b].nomParentMethod=entries[index];
      /* Using the found index we insert our new method address into the mtab. */
      entries[index]=sci->nomOverridenMethods[b].nomMethod;
      /* Do we just overrid nomUnInit()? Mark it because we need this info for the
         grabage collecting. */
      if(strstr(*sci->nomOverridenMethods[b].nomMethodId, ":nomUnInit"))
        {
          DBG_NOMBUILDCLASS(TRUE, "  nomUnInit() overriden.\n", "");
          nClass->ulClassFlags|=NOM_FLG_NOMUNINIT_OVERRIDEN; /* This flag has to be propagated down to
                                                                each child class. */
        }
    } /* for(b) */  
  }/* if(sci->numStaticOverrides) */

#ifdef DEBUG_NOMBUILDCLASS
  else
    nomPrintf(" No methods to override\n");
#endif
}


void fillCClassDataStructParentMtab(nomStaticClassInfo *sci, NOMClassPriv *nClass, NOMClass *nomClass)
{
  /* Insert pointer into CClassDataStructure */
  sci->ccds->parentMtab=&nClass->parentMtabStruct;  /* Insert pointer into CClassDataStructure */

  /* Fill somParentMtabStruct in CClassDataStructure */
  sci->ccds->parentMtab->mtab=nClass->mtab;         /* This class mtab */
  sci->ccds->parentMtab->next=nClass->mtabList.next;
  sci->ccds->parentMtab->nomClassObject=nomClass;        /* Class object    */
  sci->ccds->parentMtab->ulInstanceSize=nClass->mtab->ulInstanceSize;
  /* C Class data structure */
}


/**
  - This function builds the nomMethodTab of class nClass.
  - Calculates the method thunks
  - Calculates the instance variable thunks using the parent class info

  It takes a parent class ncpParent which may also have introduced instance variables and
  methods. 
 */
boolean addMethodAndDataToThisPrivClassStruct(NOMClassPriv* nClass, NOMClassPriv* ncpParent, nomStaticClassInfo *sci) 
{
  guint8 * mem;
  int a;

BUILDNOMCLASS_ENTER

  /* Get mem for method thunking code. This assembler code is needed so the indirect
     jump to the methods from the object pointer which is known does work. For each class
     an individual thunking code must be calculated because the number of instance
     variables is not defined. */
  if(0!=sci->ulNumStaticMethods){
    nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
    if(!nClass->mThunk)
      return FALSE;
  }

  nClass->sci=sci;   /* Save static class info for internal use */

  /* Copy assembler thunking code for instance data */
  memcpy(nClass->thunk, thunk, sizeof(thunk));
  /* Link parent mtab in */
  nClass->mtabList.next=&ncpParent->mtabList;    

  /* Fill all the pointers to the methodtable we need */
  nClass->mtab=(nomMethodTab*)&nClass->thisMtab;              /* thisMtab is the address where our mtab starts */
  nClass->mtabList.mtab= (nomMethodTab*)&nClass->thisMtab;
  nClass->parentMtabStruct.mtab=(nomMethodTab*)&nClass->thisMtab;
  
  DBG_NOMBUILDCLASS(TRUE, "nClass->mtabList.next: %x\n", nClass->mtabList.next);
  
  /* There're some parents. Copy parent mtab data into new mtab. */
  /* Copy object data. This all goes at the address of "nomMethodProc* entries[0]" 
    entries[] contain copies of the ClassDataStruct and thus the proc addresses of the static methods.
    */
  mem=(guint8*)nClass->mtab; /* Target address */
  memcpy(mem, ncpParent->mtab, ncpParent->mtab->mtabSize); /* copy parent mtab with all proc addresses */
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("copy parent data: %d (mtabSize) from %x (mtab of %s, taken from NOMClassPriv) to %x (mtab to build)\n", 
              ncpParent->mtab->mtabSize,
              sci->nomCds, ncpParent->mtab->nomClassName, mem);
#endif

  mem=((guint8*)nClass->mtab) + ncpParent->mtab->mtabSize; /* points right after the parent mtab now in 
                                                           our private struct */
  /* Add class struct of this class. This includes the proc adresses. */
  if(sci->ulNumStaticMethods) {
    DBG_NOMBUILDCLASS(TRUE, 
                      "copy own data: %d (classptr+numProcs*procpointersize) from %x (cds, classDataStruct of %x) to %x (our mtab part)\n", 
                      sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*),
                      sci->nomCds, sci->nomCds->nomClassObject, mem);

    nClass->entries0=(NOMClass**)mem; /* Our part in the mtab starts here. We need this position to insert the class object pointer
                                         later. */
    memcpy( mem, sci->nomCds, sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*));

    /* Now finally put the thunking in so the procedures are resolved correctly. */
    for(a=0;a<sci->ulNumStaticMethods;a++) {
      gulong ulOffset;

      memcpy(&nClass->mThunk[a], mThunkCode, sizeof(mThunkCode)); /* Copy thunking code */

      ulOffset=(gulong)((char*)(mem+sizeof(NOMClass*))-(char*)nClass->mtab); /* Skip class object pointer */
      nClass->mThunk[a].thunk[2]=((ulOffset+a*sizeof(nomMethodProc*))<<8)+0xa2;
      /* Put thunking code address into CClassStruct */
       sci->nomCds->nomTokens[a]=(void*)&nClass->mThunk[a];
    }
  }
  /* Thunking code see above. Adjust the offset to the instance data so the right
     variables are accessed. The offset is different depending on the number of parent
     classes (which isn't fix) and the number of parent instance vars (which isn't known
     at compile time) */
  nClass->thunk[1]=(ncpParent->mtab->ulInstanceSize<<8)+0x05; //0x00000405

  sci->ccds->instanceDataToken=&nClass->thunk;

  BUILDNOMCLASS_LEAVE;
  return TRUE;
}

/*
  This function is called when asking for information about the parent of a class.
  It checks if NOMClassMgrObject is already built and if yes asks it for the class.
  If it's not yet built we are still in the bootstrapping stage and the only parent
  we may search is NOMObject. Replacing of NOMObject is not supported by NOM.   
 */
static NOMClassPriv* priv_getClassFromName(gchar* chrClassName)
{
  NOMClassPriv *ncpParent=NULL;

  DBG_NOMBUILDCLASS(TRUE, "Find NOMClassPriv for %s %x\n", chrClassName, chrClassName);

  if(NULL==NOMClassMgrObject){
    /* If we build NOMClass or NOMClassMgr we just use the pointer we saved before. */
    if(!strcmp(chrClassName, "NOMObject"))
      ncpParent=pGlobalNomEnv->ncpNOMObject;
    else
      g_error("Asking for a parent not being NOMObject while NOMClassMgrObject is NULL. This can't be!!!");
  }/* NULL== NOMClassMgrObject */
  else
    {
      ncpParent=_nomGetClassInfoPtrFromName(NOMClassMgrObject, chrClassName,
                                  NULL);
    }

  return ncpParent;
}

#if 0
static void priv_checkForNomUnInitOverride(  NOMClass *nomClass,  NOMClassPriv *ncpParent)
{
  /* Mark the class as using nomUnInit() if any parent did that. We just have to
     check the flag and the flag of the parent class. This information is important
     because if this method is overriden the garbage collector has to run a finalizer
     on the object when collecting memory so the object may do its uninit stuff. */
  if(nomClass && !(((NOMClassPriv*)nomClass->mtab->nomClsInfo)->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN))
    {
      /* Flag not set. Try parent */
      if(ncpParent && ncpParent->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN)
        ((NOMClassPriv*)nomClass->mtab->nomClsInfo)->ulClassFlags|= NOM_FLG_NOMUNINIT_OVERRIDEN;
    }

  DBG_NOMBUILDCLASS(TRUE, "nomUnInit overriden in class %s: %s\n",
                    nomClass->mtab->nomClassName,
                    (((NOMClassPriv*)nomClass->mtab->nomClsInfo)->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN ?
                     "Yes" : "No"));

};
#endif

static void priv_checkForNomUnInitOverride(  NOMClassPriv *nomClassPriv,  NOMClassPriv *ncpParent)
{
  /* Mark the class as using nomUnInit() if any parent did that. We just have to
     check the flag and the flag of the parent class. This information is important
     because if this method is overriden the garbage collector has to run a finalizer
     on the object when collecting memory so the object may do its uninit stuff. */
  DBG_NOMBUILDCLASS(TRUE, " %s (%x): %x, %s (%x): %x ", nomClassPriv->mtab->nomClassName, nomClassPriv,
                    nomClassPriv->ulClassFlags, 
                    ncpParent->mtab->nomClassName, ncpParent, ncpParent->ulClassFlags);
  if(nomClassPriv && !(nomClassPriv->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN))
    {
      /* Flag not set. Try parent */
      if(ncpParent && ncpParent->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN)
        nomClassPriv->ulClassFlags|= NOM_FLG_NOMUNINIT_OVERRIDEN;
    }

  DBG_NOMBUILDCLASS(TRUE, "   %x nomUnInit overriden in class %s: %s\n",
                    nomClassPriv->ulClassFlags, nomClassPriv->mtab->nomClassName,
                    (nomClassPriv->ulClassFlags & NOM_FLG_NOMUNINIT_OVERRIDEN ?
                     "Yes" : "No"));
};


/*
  This function creates a private class structure for an object from the given sci with 
  correctly resolved methods which holds the mtab. This struct is kept by the metaclass
  and holds all the information for creating objects.

  nomClass: class object
  */
static NOMClassPriv * NOMLINK priv_buildPrivClassStruct(long inherit_vars,
                                                        nomStaticClassInfo *sci,
                                                        long majorVersion,
                                                        long minorVersion,
                                                        NOMClass* nomClass)
{
  gulong ulParentDataSize=0;
  gulong mtabSize;
  gulong ulMemSize=0;
  NOMClassPriv  *nClass, *ncpParent;

  DBGBUILDNOMCLASS_ENTER

  if(!nomClass||!sci)
    return NULL;

  /* The addresse of static methods in sci are already resolved. See nomBuildClass() */

  /* Get parent class if any */
  if(NULL!=sci->chrParentClassNames){
    ncpParent=priv_getClassFromName(sci->chrParentClassNames[sci->ulNumParentsInChain-1]);
  }/* nomIdAllParents */
  else
    ncpParent = NULL;

  if(!ncpParent) {
    g_warning("We are supposed to create a NOMClassPriv but there's no parent!\n");
    /* FIXME:       
       Maybe we should panic here.  */
    return NULL; /* Only NOMObject has no parent, so we have a problem here. */
  }

  /* Calculate size of new class object */
  ulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct */

  DBG_NOMBUILDCLASS(TRUE, "  ncpParent->mtab->mtabSize: %d. Parent is: %x (priv) %s\n",
                    ncpParent->mtab->mtabSize, ncpParent, ncpParent->mtab->nomClassName);

  mtabSize=ncpParent->mtab->mtabSize+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods)+
    sizeof(NOMObject*);/* numStaticMethods is correct here!
                          NOT numStaticMethods-1!
                          entries[0] in fact contains the 
                          class pointer not a method
                          pointer. */  
  ulMemSize+=mtabSize; /* add place for new procs and the new class pointer */
  ulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size */

  DBG_NOMBUILDCLASS(TRUE, "  %s: mtabSize will be: %d, ulParentDataSize is: %d\n"
                    , __FUNCTION__, mtabSize, ulParentDataSize);

  /* Alloc private class struct using NOMCalloc. */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, ulMemSize))==NULL)
    return NULL;
#if 0
  //Moved to addMethodAndDataToThisPrivClassStruct()
  /* Get mem for method thunking code */
  if(0!=sci->ulNumStaticMethods){
    nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
    if(!nClass->mThunk) {
      NOMFree(nClass);
      return NULL;
    }
  }
#endif

  /* Add class struct of this class.
     This includes 
     -resolving the new method adresses
     -add the parent mtab info to the new built mtab
  */
  if(!addMethodAndDataToThisPrivClassStruct( nClass, ncpParent, sci)){
    NOMFree(nClass);
    return NULL;
  };
  
  DBG_NOMBUILDCLASS(TRUE, "%s:  mtab: %x\n", __FUNCTION__, nClass->mtab);

  /*
    Note: We don't create a class object here so the following isn't done:
          sci->cds->classObject=nomClass;
  */

  /* Resolve ovverrides if any */
  priv_resolveOverrideMethods(nClass, sci);

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;
  nClass->mtab->nomClassObject=nomClass; /* Class object (metaclass). We build a normal class here. */
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;
#ifndef _MSC_VER
#warning !!!!! Change this when nomId is a GQuark !!!!!
#endif
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+ulParentDataSize; /* Size of instance data of this class and all
                                                                            parent classes. This isn't actually allocated for this class
                                                                            object but the entry is used when creating the objects. */
  fillCClassDataStructParentMtab(sci, nClass, nomClass);

  DBG_NOMBUILDCLASS(TRUE, "New NOMClassPriv*: %x\n", nClass);

  _nomSetObjectCreateInfo(nomClass, nClass, NULL);

  /* Mark the class as using nomUnInit() if any parent did that. We just have to
     check the flag and the flag of the parent class. This information is important
     because if this method is overriden the garbage collector has to run a finalizer
     on the object when collecting memory so the object may do its uninit stuff. */
  priv_checkForNomUnInitOverride( nClass,  ncpParent);

  DBGBUILDNOMCLASS_LEAVE
  return nClass;
}


NOMClass * NOMLINK priv_buildWithExplicitMetaClass(glong ulReserved,
                                                    nomStaticClassInfo *sci,
                                                    gulong majorVersion,
                                                    gulong minorVersion)
{
  NOMClass *nomClass, *nomClassMeta;
  
  if(NULL==NOMClassMgrObject)
    return NULL;

  /* Search for meta class. */
#ifndef _MSC_VER
#warning !!!!! Change this when nomID is a GQuark !!!!!
#endif
  nomClassMeta=_nomFindClassFromName(NOMClassMgrObject, *sci->nomExplicitMetaId, majorVersion, minorVersion, NULL);

    DBG_NOMBUILDCLASS(TRUE, "%x %s %s\n", nomClassMeta, *sci->nomClassId, *sci->nomExplicitMetaId);

  if(!nomClassMeta)
    return NULL;

  /* Create a new class object. We create a copy here because we may change the mtab entries
     through overriding or two classes may use the same meta class but have different
     sizes, methods etc. I wonder how IBM SOM manages to use the same metaclass
     for different classes without (apparently) copying it for different uses... */
  if((nomClass=(NOMClass*)NOMCalloc(1, _nomGetSize(nomClassMeta, NULL)))==NULL)
    return NULL;

  /* Maybe we should just copy the whole struct here? */
  nomClass->mtab=nomClassMeta->mtab;
#ifndef _MSC_VER
#warning !!!!! No call of _nomSetInstanceSize  !!!!!
#warning !!!!! No call of _nomSetObjectsSCI   !!!!!
#endif
#if 0
  /* Set object data */
  _nomSetInstanceSize(nomClass, _nomGetSize(nomClassMeta));
  /* Save objects sci pointer. We need it when we create instances  */
  _nomSetObjectsSCI(somClass, sci);
#endif

  /* Update static class data of class to be build */
  sci->nomCds->nomClassObject=nomClass;

  /* Now we have a meta class. Create the NOMClassPriv* holding the mtab for the class
     to be built (which is not a meta class). */
  priv_buildPrivClassStruct(ulReserved, sci,
                            majorVersion, minorVersion,
                            nomClass);
  
  DBG_NOMBUILDCLASS(TRUE, "New class Object (child of NOMClass): %x \n", nomClass);

  /* nomClassReady() is called in nomBuildClass() */
  return nomClass;
}

/*
  This function climbs the chain of parents and looks for a parent introducing
  an explicit metaclass. If found, this metaclass is returned otherwise NOMClass.
 */
static
NOMClass * NOMLINK priv_findExplicitMetaClassFromParents(nomStaticClassInfo *sci)
{
  int a;

  if(1==sci->ulNumParentsInChain)
    {
      /* One parent only. That must be NOMObject and NOMObject has NOMClass
         as metaclass. */
      return pGlobalNomEnv->defaultMetaClass;;
    }

  /* Climb the list of parents... */

  /* NOMClassMgrObject==NULL shouldn't ever happen here! */
  g_assert(NOMClassMgrObject);

  for(a=sci->ulNumParentsInChain-1;a>=0; a--)
    {
      NOMObject *nObject;
      nObject=_nomFindClassFromName(NOMClassMgrObject, sci->chrParentClassNames[a], 0, 0, NULL);

      DBG_NOMBUILDCLASS( TRUE , "   Parent %d: %s, class object: %x\n" ,
                         a, sci->chrParentClassNames[a], nObject);
      if(nObject){
        DBG_NOMBUILDCLASS( TRUE , "  Class object name: %s\n" , nObject->mtab->nomClassName);
        if(strcmp(nObject->mtab->nomClassName, "NOMClass"))
          return (NOMClass*)nObject; /* Not NOMClass so return */
      } /* if(nObject) */
    } /* for() */
  return pGlobalNomEnv->defaultMetaClass;
}

/*
  This function is called when a class for a given sci should be build with a parent
   which isn't derived from NOMClass. In that case some NOMClass ccild is the class to
   be used for the class object. We have to climb the parent list of this object class
   to check if one parent introduced an explicit metaclass. If yes, we have to use that
   one as the metaclass. If no parent did that we just use NOMClass. 
   This doesn't mean we reuse the found structs directly. Instead a new copy is created
   so individula overriding is possible.
*/
static
NOMClass * NOMLINK priv_buildWithNOMClassChildAsMeta(gulong ulReserved,
                                                     nomStaticClassInfo *sci,
                                                     long majorVersion,
                                                     long minorVersion)
{
  NOMClass  *nomClass, *nomClassDefault;
  NOMClassPriv *nClass;

#ifdef DEBUG_NOMBUILDCLASS
# ifndef _MSC_VER
#warning !!!!! Change this when nomId is a GQuark !!!!!
# endif
  nomPrintf("\n%d: Entering %s to build %s\n", __LINE__, __FUNCTION__, *sci->nomClassId);
#endif

  /* Search parents for a an explicit metaclass. If no explicit metaclass return
     NOMClass. */
  nomClassDefault=priv_findExplicitMetaClassFromParents(sci);// this gives a NOMClass* not a NOMClassPriv*

  /**** Create the meta class object ****/
  //nomClassDefault=pGlobalNomEnv->defaultMetaClass; // this gives a NOMClass* not a NOMClassPriv*

  if(!nomClassDefault)
    return NULL;

  /* Found NOMClass object */

  //nomPrintf("_nomGetSize(): %d\n", _nomGetSize(nomClassParent, NULL));

  /* Create an object */
  if((nomClass=(NOMClass*)NOMCalloc(1, _nomGetSize(nomClassDefault, NULL)))==NULL)
    return NULL;

  nomClass->mtab=nomClassDefault->mtab;

#ifndef _MSC_VER
#warning !!!!! _nomSetInstanceSize() not called here !!!!!
#warning !!!!! _nomSetObjectsSCI() not called here !!!!!
#endif
#if 0
  /* Set object data */
  _nomSetInstanceSize(somClass, _nomGetSize(nomClassDefault));
  /* Save objects sci pointer. We need it when we create instances  */
  _nomSetObjectsSCI(nomClass, sci);
#endif

  /* Update static class data of the class to be built */
  sci->nomCds->nomClassObject=nomClass;

  /* Now we have a meta class. Create the NOMClassPriv* holding the mtab for the class
     to be built. */
  nClass=priv_buildPrivClassStruct(ulReserved, sci,
                                   majorVersion, minorVersion,
                                   nomClass);

#ifdef DEBUG_NOMBUILDCLASS
  DBG_NOMBUILDCLASS(TRUE, "%s: New class Object (NOMClass): %x NOMClassPriv*: %x ulClassFlags: %x\n",
                    __FUNCTION__, nomClass, nomClass->mtab->nomClsInfo,
                    ((NOMClassPriv*)nomClass->mtab->nomClsInfo)->ulClassFlags);
  //_dumpMtab(nomClass->mtab);
  //_dumpObjShort(nomClass);
#endif

  /* nomClassReady() is called in nomBuildClass(), so don't call it here. Same goes for _nomInit(). */
  return nomClass;
}


/*
  nomClass:  NOMClass
  nClass: NOMClassPriv for NOMClass
  ulSize: size of a NOMClass object

  This function is used to add a meta class to NOMOBject which was built before.
*/
NOMClass*  createNOMObjectClassObjectAndUpdateNOMObject(NOMClass* nomClass, NOMClassPriv* nClass, gulong ulSize)
{
  NOMClassPriv *ncp;
  NOMClass  *nomObjClass;
  
  DBGBUILDNOMCLASS_ENTER
    
  /* The NOMClassPriv for NOMObject */
  ncp= pGlobalNomEnv->ncpNOMObject;  

  /* Allocate a class object for NOMObject for creating NOMObject instances. */
  if((nomObjClass=(NOMClass*)NOMCalloc(1, ulSize))==NULL) {
    /* We panic here for the simple reason that without a working NOMObject the whole object system
       will not work. */
    g_error("No memory for building the class object _NOMObject for NOMObject.");
    return NULL;
  }

  nomObjClass->mtab=nomClass->mtab;         /* Now it's an object */

  _nomSetObjectCreateInfo(nomObjClass, pGlobalNomEnv->ncpNOMObject, NULL); /* This NOMClassPriv holds all info to build 
                                                                                    instances of NOMObject (not that anybody
                                                                                    should do that but... */
#ifndef _MSC_VER
#warning !!!!!!!!!!!!! _somSetObjectsSCI() not called!!!!!!!!!!!!!!!!
#warning !!!!!!!!!!!!! _somSetClassData() not called!!!!!!!!!!!!!!!!
#endif
#if 0
  _somSetObjectsSCI(nomObjClass, ncp->sci);
  _somSetClassData(somObjClass, scp->sci->cds);
#endif

  /* Update NOMObject data */
  ncp->mtab->nomClassObject=nomObjClass;         /* This is the real NOMClass pointer, not a pointer to NOMClassPriv */
  ncp->mtab->entries[0]=(void*)nomObjClass;

  /* Put it into the class data of NOMObject */
  ncp->sci->nomCds->nomClassObject=nomObjClass;

#ifdef DEBUG_BUILDNOMOBJECT
    nomPrintf("%d: metaclass for NOMObject created: %x\n",__LINE__, nomObjClass);
#endif
    DBGBUILDNOMCLASS_LEAVE
    return nomObjClass;
}

/**
   Build a class for NOM from the data generated from the IDL compiler and
   compiled into the DLL.
 */
NOMEXTERN NOMClass * NOMLINK nomBuildClass(gulong ulReserved,
                                           nomStaticClassInfo *sci,
                                           gulong ulMajorVersion,
                                           gulong ulMinorVersion)
{
  NOMClass *nomClass;
  NOMClassPriv *nClass;
  NOMClassPriv *ncpParent;
  gulong ulParentDataSize=0;
  gulong mtabSize;
  gulong ulMemSize=0;
  int a;

#ifdef DEBUG_NOMBUILDCLASS
  nomParentMtabStructPtr pParentMtab;
  nomMethodTabs psmTab;
  /* Print some info for debbuging */
  nomPrintf("\n%d: Entering %s to build class %s. ---> NOMClassManagerObject: 0x%x\n",
            __LINE__, __FUNCTION__, *sci->nomClassId, NOMClassMgrObject);
  nomPrintf("%d: cds: 0x%x nomClassObject: 0x%x\n", __LINE__, sci->nomCds, sci->nomCds->nomClassObject);
#endif

  /* Check if already built */
  if(sci->nomCds->nomClassObject) {
    DBG_NOMBUILDCLASS(TRUE, "Class %s already built. returning 0x%x\n", *sci->nomClassId, sci->nomCds->nomClassObject);
    return (sci->nomCds)->nomClassObject; /* Yes,return the object */
  }

  /* Do we want to build again NOMObject the mother of all classes?
     This happens because every class automatically tries to build the parents
     (which includes NOMObject somewhere). NOMObject doesn't have a class object
     yet when created
     so we have to check here the global pointer to the NOMObject private data.
  */

  if(!strcmp(*sci->nomClassId, "NOMObject")){
    if(pGlobalNomEnv->ncpNOMObject!=NULL){
      DBG_NOMBUILDCLASS(TRUE, "Class %s already built. returning 0x%x\n",
                        *sci->nomClassId, sci->nomCds->nomClassObject);
      /* FIXME: this seems to be broken!! */
      return NULL; /* NOMObject already built */
    }
  }

#ifdef _DEBUG_NOMBUILDCLASS
  DBG_NOMBUILDCLASS(TRUE, "Dumping sci:\n");
  _dumpSci(sci);
#endif

  if(sci->ulNumStaticMethods!=0 && !sci->nomSMethods){
    nomPrintf("  !!! %s line %d: sci->nomSMethods is NULL for %s !!!\n", __FUNCTION__, __LINE__, *sci->nomClassId);
    return NULL;
  }
  /* Fill static classdata with the info from nomStaticMethodDesc array. This means
     putting the method adresses into the classdata struct.
     Be aware that the order of the methods in the somStaticMethod_t array is not
     necessarily in the order as specified in the releaseorder list. But each array member
     (which is a nomStaticMethodDesc) contains the address of the correct
     field in the static classdata struct. */
  /* This also resolves methods for objects not only class objects. Be careful when
     removing here. You have to correct priv_buildObjecttemplate(). */
  /* Insert class data structure method tokens */
  for(a=0;a<sci->ulNumStaticMethods;a++) {
    *sci->nomSMethods[a].nomMAddressInClassData=(void*)sci->nomSMethods[a].nomMethod; /*  Address to place the resolved function address in see *.ih files. */
#ifdef DEBUG_NOMBUILDCLASS
    nomPrintf("  static method: %s, %lx\n", *sci->nomSMethods[a].chrMethodDescriptor, sci->nomSMethods[a].nomMethod);
    nomPrintf("%d: %d: method: %x %s (classdata addr. %x) (Fill static class struct with procs)\n", 
              __LINE__, a, sci->nomSMethods[a].nomMethod,  *sci->nomSMethods[a].nomMethodId, sci->nomSMethods[a].nomMAddressInClassData);
#endif
  }

#ifdef DEBUG_NOMBUILDCLASS
  nomPrintf("%d: Dumping the filled classdata structure:\n", __LINE__);
  _dumpClassDataStruct(sci->nomCds, sci->ulNumStaticMethods);
#endif

  /* Do we want to build NOMObject the mother of all classes? */
  if(!strcmp(*sci->nomClassId, "NOMObject")){

    DBG_NOMBUILDCLASS(TRUE, "Trying to build  %s\n", *sci->nomClassId);

    priv_buildNOMObjectClassInfo(ulReserved, sci, /* yes */
                                 ulMajorVersion, ulMinorVersion);
    return NULL; /* We can't return a NOMClass for NOMObject because NOMClass isn't built yet. */
  }

  /* Do we want to build NOMClass? */
  if(!strcmp(*sci->nomClassId, "NOMClass"))
    return priv_buildNOMClass(ulReserved, sci, /* yes */
                              ulMajorVersion, ulMinorVersion);

  /* Get parent class */
  if(sci->nomIdAllParents) {
#ifdef DEBUG_NOMBUILDCLASS
# ifndef _MSC_VER
#warning !!!!! Change this when nomId is a GQuark !!!!!
# endif
    nomPrintf("%d: About to search parent %s (%d)...\n", __LINE__, **(sci->nomIdAllParents), sci->ulNumParentsInChain);
    nomPrintf("%d: About to search parent %s (%s)...\n", __LINE__, **(sci->nomIdAllParents),
	      sci->chrParentClassNames[0]);
#endif

    ncpParent=priv_getClassFromName(sci->chrParentClassNames[sci->ulNumParentsInChain-1]);

#if 0
    ncpParent=priv_getClassFromName(**(sci->nomIdAllParents));
    ncpParent=priv_findPrivClassInGlobalClassListFromName(pGlobalNomEnv,
                                                          **(sci->nomIdAllParents)); /* This may also return a class not derived
                                                                                        from NOMClass */
#endif
    if(!ncpParent)
      return NULL; /* Every class except NOMObject must have a parent!! */
  }/* nomIdAllParents */
  else
    return NULL; /* Every class except NOMObject must have a parent!! */

  if(!ncpParent)
    return NULL; /* Every class except NOMObject must have a parent!! */

#ifdef DEBUG_NOMBUILDCLASS
  /* Do some debugging here... */
  DBG_NOMBUILDCLASS(TRUE, "Found parent private class info struct. Dumping parentMTabStruct...\n", NULL);
  pParentMtab=&ncpParent->parentMtabStruct;
  nomPrintf("     parent class: %s (priv %x), pParentMtab: %x, pParentMtab->mtab %x, next: %x\n",
            ncpParent->mtab->nomClassName,
            ncpParent, pParentMtab, pParentMtab->mtab, pParentMtab->next);
  /* climb parent list */
  psmTab=pParentMtab->next;
  while(psmTab) {
    nomPrintf("     next class: %s, next: %x\n", psmTab->mtab->nomClassName, psmTab->next);
    psmTab=psmTab->next;
  }
#endif /* DEBUG_NOMBUILDCLASS */


  /* Check if parent is a class object (derived from NOMClass). */
  if(!priv_nomIsA((NOMObject*)ncpParent, pGlobalNomEnv->defaultMetaClass)) {
    /* No, parent of class to build is normal object so we have to use either an explicit meta class if given or
       another NOMClass derived class for the class object. */

    DBG_NOMBUILDCLASS(TRUE, "Class %x (ncpParent->mtab->nomClassName: %s) is not a NOMClass\n",
                      ncpParent, ncpParent->mtab->nomClassName);

    if(sci->nomExplicitMetaId)
      {
        /* The explicit metaclass is created at this point. Now it will be filled
           with the info how to create objects. */

        DBG_NOMBUILDCLASS(TRUE, "sci->nomExplicitMetaId is set. Calling priv_buildWithExplicitMetaClass().\n", "");
    
        nomClass= priv_buildWithExplicitMetaClass(ulReserved, sci,
                                                  ulMajorVersion, ulMinorVersion);
        if(nomClass){
          CORBA_Environment * tempEnv=nomCreateEnvNoObjectCheck();

          DBG_NOMBUILDCLASS(TRUE, "%s: class is 0x%x\n", nomClass->mtab->nomClassName, nomClass);
#if 0
          /* Mark the class as using nomUnInit() if any parent did that. We just have to
             check the flag and the flag of the parent class. This information is important
             because if this method is overriden the garbage collector has to run a finalizer
             on the object when collecting memory so the object may do its uninit stuff. */
          priv_checkForNomUnInitOverride( (NOMClassPriv*)nomClass->mtab->nomClsInfo,  ncpParent);
#endif     
          /* Make sure the env is marked that we don't chek the object pointer. This would fail
             because the class isn't registered yet. */
          _nomInit((NOMObject*)nomClass, tempEnv);
          _nomClassReady(nomClass, tempEnv);
        }

        return nomClass;
      }/* nomExplicitMetaId */
    else {
      /* Use NOMClass derived class as meta class. We have to climb the parent list of this object class
         to check if one parent introduced an explicit metaclass. If yes, we have to use that one as
         the metaclass. If no parent did that we just use NOMClass. 
         The following call will create the class object and will also fill in the necessary object info for
         creating instances. */
      nomClass= priv_buildWithNOMClassChildAsMeta(ulReserved, sci,
                                                  ulMajorVersion, ulMinorVersion);

      if(nomClass){
        CORBA_Environment * tempEnv=nomCreateEnvNoObjectCheck();
        /* Make sure the env is marked that we don't chek the object pointer. This would fail
           because the class isn't registered yet. */
        _nomInit((NOMObject*)nomClass, tempEnv);
        _nomClassReady(nomClass, tempEnv);
      }
      return nomClass;
    }
  }/* NOMClass derived? */

  /* Child of some NOMClass */

  /**** From this point we are building a new class object (derived from NOMClass ****/
  ulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct */

  /* Calculate size of new class object */
  DBG_NOMBUILDCLASS(TRUE, "Parent class 0x%x (ncpParent->mtab->nomClassName: %s) is a NOMClass (or derived)\n",
                    ncpParent, ncpParent->mtab->nomClassName);
  DBG_NOMBUILDCLASS(TRUE, "      ncParent->mtab->mtabSize: %d\n", ncpParent->mtab->mtabSize);

  mtabSize=ncpParent->mtab->mtabSize+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods)+sizeof(NOMClass*);/* ulNumStaticMethods is correct here!
                                                                                                        NOT numStaticMethods-1!
                                                                                                        entries[0] in fact contains the 
                                                                                                        class pointer not a method
                                                                                                        pointer. */
  ulMemSize+=mtabSize; /* add place for new procs and the new class pointer */
  ulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size */
  
  DBG_NOMBUILDCLASS(TRUE, "%s mtabSize is: %d, ulParentDataSize is: %d\n", 
                    *sci->nomClassId, mtabSize, ulParentDataSize);

  /* Alloc class struct using NOMCalloc. */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, ulMemSize))==NULL)
    return NULL;

#if 0
  //Moved to addMethodAndDataToThisPrivClassStruct()
  if(0!=sci->ulNumStaticMethods){
    /* Get mem for method thunking code */
    nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
    if(!nClass->mThunk) {
      NOMFree(nClass);
      return NULL;
    }
  }
#endif

  nClass->ulClassSize=sci->ulInstanceDataSize+ulParentDataSize;

  if((nomClass=(NOMClass*)NOMCalloc(1, sci->ulInstanceDataSize+ulParentDataSize))==NULL) {
    //NOMFree(nClass->mThunk);
    NOMFree(nClass);
    return NULL;
  }

  nClass->ulPrivClassSize=ulMemSize;

  /* Add class struct of this class. This includes resolving the method adresses. */
  if(!addMethodAndDataToThisPrivClassStruct( nClass, ncpParent, sci)){
    NOMFree(nClass);
    NOMFree(nomClass);
    return NULL;
  };

  /* Resolve overrides if any */
  priv_resolveOverrideMethods(nClass, sci);

  nomClass->mtab=nClass->mtab;  

  DBG_NOMBUILDCLASS(TRUE,"mtab: %x, nClass: 0x%x, nomClass: 0x%x\n", nClass->mtab, nClass, nomClass);

  sci->nomCds->nomClassObject=nomClass; /* Put class pointer in static struct */

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;
  nClass->mtab->nomClassObject=nomClass;
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+ulParentDataSize; /* Size of instance data of this class and all
                                                                            parent classes. This isn't actually allocated
                                                                            for this class object but the entry is used
                                                                            when creating the objects. */
  fillCClassDataStructParentMtab(sci, nClass, nomClass);

  /* Thunking see above */
  nClass->thunk[1]=(ulParentDataSize<<8)+0x05; //0x00000405
  sci->ccds->instanceDataToken=&nClass->thunk;

  /* Set this class size into instance var */
#ifndef _MSC_VER
#warning !!!!! No call of _nomSetInstanceSize() here !!!!!
#endif
  // _nomSetInstanceSize(nomClass, sci->ulInstanceDataSize+ulParentDataSize);

  DBG_NOMBUILDCLASS(TRUE, "New class ptr (class object): %x (NOMClassPriv: %x) for %s\n"
                    , nomClass, nClass, *sci->nomClassId);

  if(nomClass){
    CORBA_Environment * tempEnv=nomCreateEnvNoObjectCheck();

    /* Mark the class as using nomUnInit() if any parent did that. We just have to
       check the flag and the flag of the parent class. This information is important
       because if this method is overriden the garbage collector has to run a finalizer
       on the object when collecting memory so the object may do its uninit stuff. */
    priv_checkForNomUnInitOverride( (NOMClassPriv*)nomClass->mtab->nomClsInfo,  ncpParent);
    
    /* Make sure the env is marked that we don't chek the object pointer. This would fail
       because the class isn't registered yet. */
    _nomInit(nomClass, tempEnv);
    _nomClassReady(nomClass, tempEnv);
  }
  return nomClass;
};






/*********************************************************************************************************************/
/*     Unused stuff */
/*********************************************************************************************************************/
#if 0
#include <cwsomcls.h>
#include <somclassmanager.h>
/********************************************************/
/*   Toolkit functions, exported                        */
/********************************************************/

/*
  Caller is responsible for freeing the returned somId with SOMFree.
  Note: this is not the internal id (hash) of a string!

  FIXME:
  This function must be checked if it's correct.
 */

somId SOMLINK somIdFromString (string aString)
{
  /* This call automatically registers the ID with the runtime */
  somIdItem *sid;
  somId sID;

  if(!aString)
    return NULL;

  /* Do we already have an ID at all? */
  sid=priv_findSomIdInList(pGlobalSomEnv, aString);

  if(sid) {
    sID=SOMMalloc(sizeof(void*));
    if(!sID)
      return NULL;

    *sID=(char*)sid;
    return sID;
  }

  /* No somId registered  yet, so create one */
  if((sid=(somIdItem*)SOMCalloc(1, sizeof(somIdItem)))==NULL)
    return NULL;

  sid->idString=SOMMalloc(strlen(aString)+1);
  if(!sid->idString)
    {
      SOMFree(sid);
      return NULL;
    }

  sid->id=calculateNameHash(aString);

  strcpy(sid->idString, aString);
  if(!priv_addSomIdToIdList(pGlobalSomEnv, sid)) {
    SOMFree(sid->idString);
    SOMFree(sid);
    return NULL;
  }

  sID=SOMMalloc(sizeof(void*));
  if(!sID)
    return NULL;

  *sID=(char*)sid;
  return sID;
}

/*
  FIXME: semaphores!!!!!
 */
/*
  This function tries to find the class introducing the method 'sid' at first.
 */
static SOMClassPriv* priv_getOverrideClass(somId *sid)
{
  char* chrPtr;
  SOMClassPriv *scp;
  gulong ulLen;
  char *chrMem;

  if(!sid)
    return NULL;

  if((chrPtr=strchr(**sid, ':'))==NULL)
    return NULL;

  /* Create local copy */
  ulLen=strlen(**sid);
  if(ulLen>5000) /* prevent stack overflow in case of error */
    return NULL;

  chrMem=alloca(ulLen);
  strcpy(chrMem, **sid);

  if((chrPtr=strchr(chrMem, ':'))==NULL)
    return NULL; /* How should that happen, but anyway... */

  *chrPtr=0; /* Now we have separated the class name */
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: searching override for %s\n", __LINE__, __FUNCTION__, chrMem);
#endif
  scp=priv_findPrivClassInGlobalClassList(pGlobalSomEnv, chrMem);
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x\n", __LINE__, __FUNCTION__, scp);
#endif
  if(!scp)
    return NULL;
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x (SOMClassPriv) ->%x (SOMClass)\n", __LINE__, __FUNCTION__, scp, scp->mtab->classObject);
#endif
  return scp;
}

/*
  FIXME: semaphores!!!!!
 */
/*
  This function tries to find the class introducing the method 'sid' at first.
  It returns a SOMClass not a SOMClassPriv.
 */
static SOMClass* priv_getOverrideSOMClass(somId *sid)
{
  char* chrPtr;
  SOMClassPriv *scp;
  gulong ulLen;
  char *chrMem;

  if(!sid)
    return NULL;

  if((chrPtr=strchr(**sid, ':'))==NULL)
    return NULL;

  /* Create local copy */
  ulLen=strlen(**sid);
  if(ulLen>5000) /* prevent stack overflow in case of error */
    return NULL;

  chrMem=alloca(ulLen);
  strcpy(chrMem, **sid);

  if((chrPtr=strchr(chrMem, ':'))==NULL)
    return NULL; /* How should that happen, but anyway... */

  *chrPtr=0; /* Now we have separated the class name */
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: searching override for %s\n", __LINE__, __FUNCTION__, chrMem);
#endif
  scp=priv_findPrivClassInGlobalClassList(pGlobalSomEnv, chrMem);
  somPrintf("%d: %s: found %x (SOMClassPriv)\n", __LINE__, __FUNCTION__, scp);
  if(!scp)
    return NULL;
#ifdef DEBUG_SOMBUILDCLASS
  somPrintf("%d: %s: found %x (SOMClassPriv) ->%x\n", __LINE__, __FUNCTION__, scp, scp->mtab->classObject);
#endif
  return scp->mtab->classObject;
}

/*
  This function finds the class which introduced the method (methodId) to be overriden. It gets the index
  in the mtab of that class and returns it. Using this index the correct method address is taken
  from the mtab of the parent class of the class which wants to override a method (sClass). By using the
  parent instead the original class (introducing this method in the beginning) any overriding
  done in a class subclassing the introducing class is automatically taken into account.
 */
/*
  FIXME: semaphores ????
 */
static gulong priv_getIndexOfMethodToBeOverriden(somId *methodId )
{
  return 0;
}

/*
  Class format:

  struct _SOMClass {
  struct somMethodTabStruct  *mtab;
  struct somClassInfo;
  struct somMethodTabListStruct;
  struct somParentMtabStruct;               Struct to place parent mtab pointer
  gulong  thunk[3];                       Assembler thunking code
  somMethodTabStruct mtabStruct;         See beloe
  ClassDataStruct class1;
  ClassDataStruct class2;
  ClassDataStruct class3;
  ...
  };
  
  -- Object Instance Structure:
  
  struct somMethodTabStruct;
  typedef struct SOMAny_struct {
  struct somMethodTabStruct  *mtab;
  integer4 body[1];
  } SOMAny;

  The following struct was built by CW
  struct somClassInfo
  {
  SOMClass        *classObject;       <- This is a pointer to the SOMObject class object in SOM.
  }

typedef struct somMethodTabStruct {
    SOMClass        *classObject;
    somClassInfo    *classInfo; 
    char            *className;
    long            instanceSize;
    long            dataAlignment;
    long            mtabSize;
    long            protectedDataOffset; / from class's introduced data
    somDToken       protectedDataToken;
    somEmbeddedObjStruct *embeddedObjs;
    / remaining structure is opaque /
    somMethodProc* entries[1];           <-- I found that this isn't correct (or I misseed something in the includes). When dumping a mtab
                                             the following has this structure:
                                   
                                             SOMClass         *classObject; /The first class object (SOMObject)
                This is basically a copy ->  somMethodProc*   firstMethod_1;
                of the ClassDataStruct       somMethodProc*   secondMethod_1;
                                             ...
                                             SOMClass         *classObject; /The second class object
                ClassDataStruct of 2.    ->  somMethodProc*   firstMethod_2; 
                class                        somMethodProc*   secondMethod_2;
                        
} somMethodTab, *somMethodTabPtr;
*/

/*
  Build a "SOMObject class" usable for building other class objects e.g. SOMClass
  (base for all class objects). Right after SOMObject is constructed, it will be used only
  for holding the created mtab structure which contains all the resolved procedure
  pointers. This mtab is referenced by the SOMObject and thus by SOMClass (and all classes
  to come).
  SOMObject will get a full featured class object later when SOMClass is built.


  Is this still correct???:

  Within this function a "real" SOMObject will be created which will be used when
  creating normal objects derived from SOMObject but not from a class object.
  This is necessary because the class structure and the object structure are different
  so we need two different templates. The class created by this function is only used
  for holding data necessary for building SOMClass. This class pointer will not be used
  for anything else.
 */
SOMClass * SOMLINK priv_buildSOMObject(long inherit_vars,
                                       somStaticClassInfo *sci,
                                       long majorVersion,
                                       long minorVersion)
{
  guint8 * mem;
  SOMClassPriv *sClass; /* This struct holds our private data. A pointer will be in mtab->classInfo */
  SOMClass *somClass;   /* A real SOMClass pointer */
  int a;
  gulong mtabSize;
  gulong ulMemSize=0;
  gulong ulParentDataSize=0;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("%d: Entering %s to build a temporary SOMObject class object\n", __LINE__, __FUNCTION__);
  somPrintf("%d: Entering %s to build the mtab for SOMObjects (pEnv->mtabSOMObject)\n", __LINE__, __FUNCTION__);
  _dumpSci(sci);
#endif

  /* Note: SOMObject has no parents */
  return NULL;
  
  /* ulMemsize will be the size of our private class structure SOMClassPriv */
  ulMemSize=sizeof(SOMClassPriv)-sizeof(somMethodTab); /* start size class struct without the somMethodTab
                                                          holding the method pointers. The size of this
                                                          somMethodTab will be added later. */

  /* Calculate the size of the method tab to be added to the size of the private class struct */
  mtabSize=sizeof(somMethodTab)+sizeof(somMethodProc*)*(sci->numStaticMethods);/* numStaticMethods is correct here! NOT 
                                                                                  numStaticMethods-1! entries[0] in fact
                                                                                  contains the class pointer not a method
                                                                                  pointer. */
  ulMemSize+=mtabSize; /* Add size of base mtab struct */

  /* Alloc private class struct using SOMCalloc. */
  if((sClass=(SOMClassPriv*)SOMCalloc(1, ulMemSize))==NULL)
    return NULL;

  /* Get mem for method thunking code. This assembler code is needed so the indirect
     jump to the methods from the object pointer which is known does work. For each class
     an individual thunking code must be calculated because the number of instance
     variables is not defined. */
  sClass->mThunk=SOMMalloc(sizeof(cwMethodThunk)*sci->numStaticMethods);
  if(!sClass->mThunk) {
    SOMFree(sClass);
    return NULL; 
  }

  /* The size of each instance of this class. A SOM object has a method tab pointer
     at the beginning followed by the instance variables. */
  sClass->ulClassSize=sci->instanceDataSize+sizeof(somMethodTab*);

  sClass->sci=sci;                                   /* Save static class info for internal use          */
  sClass->ulPrivClassSize=ulMemSize;                 /* Internal housekeeping. Not needed by SOM methods */
  memcpy(sClass->thunk, thunk, sizeof(thunk));       /* Copy assembler thunking code for instance data   */

  /* Fill all the pointers to methodtable we need in the *private* structure */
  sClass->mtab=(somMethodTab*)&sClass->thisMtab;            /* create the mtab pointer and store it           */
  sClass->mtabList.mtab= (somMethodTab*)&sClass->thisMtab;  /* thisMtab is the position where the mtab starts */
  sClass->parentMtabStruct.mtab=(somMethodTab*)&sClass->thisMtab;

  /* And now the real SOMClass struct which will be seen by the user. A SOMClass has a mTab pointer
     at the beginning and the instance data following. */
  if((somClass=(SOMClass*)SOMCalloc(1, sci->instanceDataSize+sizeof(somMethodTab*)))==NULL) {
    SOMFree(sClass->mThunk);
    SOMFree(sClass);
    return NULL;
  }
  somClass->mtab=sClass->mtab;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("mtab: %x sClass: %x, somClass: %x\n", sClass->mtab, sClass, somClass);
#endif

  /*
    FIXME: this somClass must be deleted after updating with the real metaclass!
   */
  /*
    We don't have a class object yet...
  */
  // sci->cds->classObject=somClass; /* Put class pointer in static struct */

  /* Copy class data. This goes at the address of "somMethodProc* entries[0]".
     Entries[] contain copies of the ClassDataStruct and thus the proc addresses of the static methods.
     We don't use the static classDataStruct directly because subclasses will override the proc addresses. */
  mem=(char*)&(somClass->mtab->entries[0]); /* Target address.entries[0] will contain the class pointer */

  /* Add class struct of this class. This includes the proc adresses. */
  if(sci->numStaticMethods) {
#ifdef DEBUG_BUILDSOMOBJECT
    somPrintf("copy: %d (classptr+numProcs*procpointersize) from %x (cds, classDataStruct) to %x\n", 
              sizeof(SOMClass*)+sci->numStaticMethods*sizeof(somMethodProc*),
              sci->cds, mem);
#endif
    /* Copy classDataStruct with the resolved proc addresses */
    memcpy( mem, sci->cds, sizeof(SOMClass*)+sci->numStaticMethods*sizeof(somMethodProc*));

    /* Now finally put the thunking in so the procedures are resolved correctly. */
    for(a=0;a<sci->numStaticMethods;a++) {
      gulong ulOffset;

      memcpy(&sClass->mThunk[a], mThunkCode, sizeof(mThunkCode));               /* Copy method thunking code template  */
      ulOffset=(gulong)((char*)(mem+sizeof(SOMClass*))-(char*)somClass->mtab);   /* Skip priv class data pointer        */
      sClass->mThunk[a].thunk[2]=((ulOffset+a*sizeof(somMethodProc*))<<8)+0xa2; /* Calculate offset for assembler code */
#ifdef DEBUG_BUILDSOMOBJECT
      somPrintf(" %d: %d : Thunk offset: %d (0x%x) -> address will be: %x\n",
                __LINE__, a, ulOffset, ulOffset, mem+ulOffset+a*sizeof(somMethodProc*) );
#endif
      /* Put thunking code address into CClassStruct */
      sci->cds->tokens[a]=(void*)&sClass->mThunk[a];
    } /* for */
  } /* if(numStaticMethods) */

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  sClass->mtab->mtabSize=mtabSize;     /* This mtab is the same as the one used in the real SOMClass */

#if 0
  /* We don't have a class object yet. */
  sClass->mtab->classObject=somClass; /* This is the real SOMClass pointer, not a pointer to SOMClassPriv */
#endif

  sClass->mtab->classInfo=(somClassInfo*)sClass;  /* FIXME: I think I may just use this undocumented field for the private data. */
  sClass->mtab->className=*sci->classId;
  sClass->mtab->instanceSize=sci->instanceDataSize+sizeof(somMethodTabPtr); /* sizeof(methodTabStruct*) + size of instance data of this class
                                                                               and all parent classes. This is SOMObject so we have no parents. */
  /*
    FIXME:   
    The following is not yet initialized (and the previous may be buggy...) */
  //    long	    dataAlignment;
  //    long	    protectedDataOffset; /* from class's introduced data */
  //    somDToken	    protectedDataToken;
  //    somEmbeddedObjStruct *embeddedObjs;

  sci->ccds->parentMtab=&sClass->parentMtabStruct;  /* Insert pointer into CClassDataStructure */

  /* Fill somParentMtabStruct in CClassDataStructure */
  sci->ccds->parentMtab->mtab=sClass->mtab;         /* This class mtab                               */
  sci->ccds->parentMtab->next=NULL;                 /* We dont have parents because we are SOMObject */
  sci->ccds->parentMtab->classObject=somClass;      /* SOMClass* Class object, this means ourself    */
  sci->ccds->parentMtab->instanceSize=sClass->mtab->instanceSize;
  /* C Class data structure */

  /* Thunking code see above. Adjust the offset to the instance data so the right
     variables are accessed. The offset is different depending on the number of parent
     classes (which isn't fix) and the number of parent instance vars (which isn't known
     at compile time) */
  sClass->thunk[1]=(ulParentDataSize<<8)+0x05; //0x00000405
  sci->ccds->instanceDataToken=&sClass->thunk;

#ifdef DEBUG_BUILDSOMOBJECT
  somPrintf("New class ptr (temp. class object for SOMObject): %x (SOMClassPriv: %x) for %s\n",
            somClass, sClass, *sci->classId);

  somPrintf("%d: Dumping the filled classdata structure:\n", __LINE__);
  _dumpClassDataStruct(sci->cds, sci->numStaticMethods);
#endif
#ifdef DEBUG_OBJECTS
  _dumpMTabListPrivClass(sClass);
#endif

  /* SOMObject is special because it's root of normal classes and meta classes. Because of this it
     isn't insert into the normal private meta class list. We also use this info to check if SOMObject
     was already built in other places. */
  pGlobalSomEnv->scpSOMObject=sClass;

  priv_addPrivClassToGlobalClassList(pGlobalSomEnv, sClass);

#ifdef DEBUG_OBJECTS
  _dumpObj(somClass);
#endif

  /*
    We don't run the somInit() method here because the SOMObject class isn't yet completely built.
    First a SOMClass meta class must be created because SOMObject needs a SOMClass as the class object like
    any other class. In case some code in somInit() tries to access the class object initialization would
    fail. In the function building the root SOMClass a metaclass for SOMObject will be created and attached.

    Not running somInit() here shouldn't be a problem because we know what we are doing ;-).
    If there will ever be the need for special initialization for this very first SOMObject we think again...

    In any case fiddling with somInit() in SOMObject and SOMClass is a dangerous thing because at least one of the
    two classes won't be completely built at that point in time (because SOMClass is a subclass of SOMObject
    and SOMObject needs a SOMClass as metaclass).
 
    _somInit(somClass);
  */
  return somClass;
}
#endif
