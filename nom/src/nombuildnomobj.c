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
# define INCL_DOSERRORS
# include <os2.h>
#endif /* __OS2__ */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#define SOM_NO_OBJECTS  /* Otherwise som.h includes the IBM SOM classes */

/* For nomToken etc. */
#include <nom.h>
#include <nomtk.h>
#include <nomobj.h>
#include <thunk.h>

/********************************************************/

/* Define if you want to have messages from building NOMObject */
//#define DEBUG_BUILDNOMOBJECT

/********************************************************/

/********************************************************/

extern PNOM_ENV pGlobalNomEnv;


/********************************************************/


/*
  Create the NOMClassPriv structure *only* for NOMObject and fill it with info
  from the sci.

  This function does

  -allocate the memory for the struct depending on the sci info
  -creates the mtab in this memory
  -fills all the necessary pointers in the mtab
  -fills the method addresses in the mtab

  It does not insert a class object pointer!

 */
static NOMClassPriv *buildNOMClassPrivStructForNOMObject(nomStaticClassInfo *sci)
{

  NOMClassPriv *nClass; /* This struct holds our private data. A pointer will be in mtab->nomClsInfo */

  gulong mtabSize;
  gulong ulMemSize=0;
  guint8 * mem;
  int a;

  /* ulMemsize will be the size of our private class structure NOMClassPriv */
  ulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct without the nomMethodTab
                                                          holding the method pointers. The size of this
                                                          nomMethodTab will be added later. */
  
  /* Calculate the size of the method tab to be added to the size of the private class struct */
  mtabSize=sizeof(nomMethodTab)+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods);/* ulNumStaticMethods is correct here! NOT 
                                                                                    ulNumStaticMethods-1! entries[0] in fact
                                                                                    contains the class pointer not a method
                                                                                    pointer. */
  ulMemSize+=mtabSize; /* Add size of base mtab struct */
  
  /* Alloc private class struct using NOMCalloc. */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, ulMemSize))==NULL)
    return NULL;
  

  /* The size of each instance of this class. A SOM object has a method tab pointer
     at the beginning followed by the instance variables. */
  nClass->ulClassSize=sci->ulInstanceDataSize+sizeof(nomMethodTab*);  
  nClass->sci=sci;                                   /* Save static class info for internal use          */
  nClass->ulPrivClassSize=ulMemSize;                 /* Internal housekeeping. Not needed by NOM methods */


  /* Fill all the pointers to methodtable we need in the *private* structure */
  nClass->mtab=(nomMethodTab*)&nClass->thisMtab;            /* create the mtab pointer and store it           */
  nClass->mtabList.mtab= (nomMethodTab*)&nClass->thisMtab;  /* thisMtab is the position where the mtab starts */
  nClass->parentMtabStruct.mtab=(nomMethodTab*)&nClass->thisMtab;


  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;        /* This mtab is the same as the one used in the public NOMClass */
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;        /* Hold a pointer to the private data that is this NOMClassPriv */
#ifndef _MSC_VER
#warning !!!!! Change this when nomId is a GQuark !!!!!
#endif
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+sizeof(nomMethodTabPtr); /* sizeof(methodTabStruct*) + size of instance data of this class
                                                                                   and all parent classes. This is NOMObject so we have no parents. */
  
  /* Get mem for method thunking code. This assembler code is needed so the indirect
     jump to the methods from the object pointer which is known does work. For each class
     an individual thunking code must be calculated because the number of instance
     variables is not defined. */
  if(0!=sci->ulNumStaticMethods){
    nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
    if(!nClass->mThunk) {
      NOMFree(nClass);
      return NULL; 
    }
  }

  memcpy(nClass->thunk, thunk, sizeof(thunk));       /* Copy assembler thunking code for instance data   */
 
  /* Copy class data. This goes at the address of "nomMethodProc* entries[0]".
     Entries[] contain copies of the ClassDataStruct and thus the proc addresses of the static methods.
     We don't use the static classDataStruct directly because subclasses will override the proc addresses. */
  mem=(char*)&(nClass->mtab->entries[0]); /* Target address.entries[0] will contain the class pointer */
  /* Add class struct of this class. This includes the proc adresses. */
  if(sci->ulNumStaticMethods) {
#ifdef DEBUG_BUILDNOMOBJECT
    nomPrintf("copy: %d (classptr+numProcs*procpointersize) from %x (cds, classDataStruct) to %x\n", 
              sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*),
              sci->nomCds, mem);
#endif
    /* Copy classDataStruct with the resolved proc addresses */
    memcpy( mem, sci->nomCds, sizeof(NOMClass*)+sci->ulNumStaticMethods*sizeof(nomMethodProc*));
    /* Now finally put the thunking in so the procedures are resolved correctly. */
    for(a=0;a<sci->ulNumStaticMethods;a++) {
      gulong ulOffset;

      memcpy(&nClass->mThunk[a], mThunkCode, sizeof(mThunkCode));               /* Copy method thunking code template  */
      ulOffset=(gulong)((char*)(mem+sizeof(NOMClass*))-(char*)nClass->mtab);     /* Skip priv class data pointer        */
      nClass->mThunk[a].thunk[2]=((ulOffset+a*sizeof(nomMethodProc*))<<8)+0xa2; /* Calculate offset for assembler code */
#ifdef DEBUG_BUILDNOMOBJECT
      nomPrintf(" %d: %d : Thunk offset: %d (0x%x) -> address will be: %x\n",
                __LINE__, a, ulOffset, ulOffset, mem+ulOffset+a*sizeof(nomMethodProc*) );
#endif
      /* Put thunking code address into CClassStruct */
      sci->nomCds->nomTokens[a]=(void*)&nClass->mThunk[a];
    } /* for */
  } /* if(ulNumStaticMethods) */
  return nClass; /* This is not a NOMClass* but a NOMClassPriv* */
}

/*
  Create a NOMClassPriv (which will be put into the classInfo of the mtab) for
  NOMObject. This contains an mtab which is completely built but without
  a pointer to the class object. This pointer will be inserted later when we have
  a NOMClass.

  !!! This function is only called once for building NOMObject !!!
 */
NOMClassPriv * NOMLINK priv_buildNOMObjectClassInfo(gulong ulReserved,
                                                    nomStaticClassInfo *sci,
                                                    gulong majorVersion,
                                                    gulong minorVersion)
{
  NOMClassPriv *nClassPriv; /* This struct holds our private data. A pointer will be in mtab->nomClsInfo */
  gulong ulParentDataSize=0;

#ifdef DEBUG_BUILDNOMOBJECT
  nomPrintf("%d: Entering %s to build the NOMClassPriv for NOMObjects\n", __LINE__, __FUNCTION__);
  _dumpSci(sci);
#endif

  /* Note: NOMObject has no parents */
  if((nClassPriv=buildNOMClassPrivStructForNOMObject(sci))==NULL)
    return NULL;

#ifdef DEBUG_BUILDNOMOBJECT
  nomPrintf("mtab: %x nClassPriv: %x\n", nClassPriv->mtab, nClassPriv);
#endif

  sci->ccds->parentMtab=&nClassPriv->parentMtabStruct;  /* Insert pointer into CClassDataStructure */
  /* Fill somParentMtabStruct in CClassDataStructure */
  sci->ccds->parentMtab->mtab=nClassPriv->mtab;         /* This class mtab                               */
  sci->ccds->parentMtab->next=NULL;                     /* We dont have parents because we are NOMObject */
  sci->ccds->parentMtab->nomClassObject=NULL;     /* NOMClass* Class object. We don't have one yet */
  sci->ccds->parentMtab->ulInstanceSize=nClassPriv->mtab->ulInstanceSize;
  /* C Class data structure */

  /* Thunking code see above. Adjust the offset to the instance data so the right
     variables are accessed. The offset is different depending on the number of parent
     classes (which isn't fix) and the number of parent instance vars (which isn't known
     at compile time) */
  nClassPriv->thunk[1]=(ulParentDataSize<<8)+0x05; //0x00000405
  sci->ccds->instanceDataToken=&nClassPriv->thunk;


#ifdef DEBUG_BUILDNOMOBJECT
  nomPrintf("No class ptr (temp. class object for NOMObject) yet. (NOMClassPriv: %x) for %s\n",
            nClassPriv, *sci->nomClassId);  
  nomPrintf("%d: Dumping the filled classdata structure:\n", __LINE__);
  _dumpClassDataStruct(sci->nomCds, sci->ulNumStaticMethods);
#endif


  /* NOMObject is special because it's root of normal classes and meta classes. Because of this it
     cannot be insert into the class list yet. We have to wait until NOMClass was built.
     We also use this info to check if NOMObject was already built in other places. */
  pGlobalNomEnv->ncpNOMObject=nClassPriv;

  //priv_addPrivClassToGlobalClassList(pGlobalNomEnv, nClassPriv);

  /*
    FIXME: This explanantion isn't correct anymore

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

  /* Run initialization code if any */
  _nomInit((NOMObject*)nClassPriv, NULL);
  return NULL;
}




