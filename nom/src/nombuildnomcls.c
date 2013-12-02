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

#include <glib.h>
#define SOM_NO_OBJECTS  /* Otherwise som.h includes the IBM SOM classes */

/* For somToken etc. */
#include <nom.h>
#include <nomtk.h>
#include <nomobj.h>

/********************************************************/

/* Define if you want to have messages from nomBuildClass() and friends */
//#define DEBUG_NOMBUILDCLASS
/* Define if you want to have messages from building NOMObject */
//#define DEBUG_BUILDNOMOBJECT
/* Define if you want to have messages from building NOMClass */
//#define DEBUG_BUILDNOMCLASS

#ifdef DEBUG_BUILDNOMCLASS
    #define BUILDNOMCLASS_ENTER nomPrintf("\n%d: *** entering %s...\n",__LINE__,  __FUNCTION__);
    #define BUILDNOMCLASS_LEAVE nomPrintf("%d: *** Leaving %s...\n\n",__LINE__,  __FUNCTION__);
    #define DBG_BUILDNOMCLASS(a, b,...)   if(a) nomPrintf("%d: " b , __LINE__,  __VA_ARGS__);
#else
    #define BUILDNOMCLASS_ENTER
    #define BUILDNOMCLASS_LEAVE
#   ifdef _MSC_VER 
void _inline DBG_BUILDNOMCLASS(gboolean a, const char *msg, ...)
{
    /* sorry, nothing here. */
}
#   else
    #define DBG_BUILDNOMCLASS(a, b,...)
#   endif
#endif

/********************************************************/

/********************************************************/

extern PNOM_ENV pGlobalNomEnv;
extern gulong thunk[];
extern gulong mThunkCode[];

/********************************************************/

/*
  Create the SOMClassPriv structure and fill it with info
  from the sci. This is a general function when building classes.

  This function does

  -allocate the memory for the struct depending on the sci info
  -creates the mtab in this memory
  -fills all the necessary pointers in the mtab
  -fills the method addresses in the mtab

  It does not insert a class object pointer!

 */
static NOMClassPriv *buildNOMClassPrivStruct(nomStaticClassInfo *sci, NOMClassPriv *ncpParent)
{
  gulong gulParentDataSize=0;
  gulong mtabSize;
  gulong gulMemSize=0;
  NOMClassPriv *nClass;

BUILDNOMCLASS_ENTER

  /* ulMemsize will be the size of our private class structure SOMClassPriv */
  gulMemSize=sizeof(NOMClassPriv)-sizeof(nomMethodTab); /* start size class struct. somMethodTab will be calculated later */

  /* Calculate size of new class object */
  DBG_BUILDNOMCLASS(TRUE, "ncParent->mtab->mtabSize: %d\n", ncpParent->mtab->mtabSize);

  mtabSize=ncpParent->mtab->mtabSize+sizeof(nomMethodProc*)*(sci->ulNumStaticMethods)+sizeof(NOMClass*);/* numStaticMethods is correct here!
                                                                                                           NOT numStaticMethods-1!
                                                                                                           entries[0] in fact contains the 
                                                                                                           class pointer not a method
                                                                                                           pointer. */
  gulMemSize+=mtabSize; /* add space for new procs and the new class pointer in addition to parent stuff */

  gulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size. This is the mtab pointer + instance vars */

  DBG_BUILDNOMCLASS(TRUE,"mtabSize is: %d, ulParentDataSize is: %d (instance vars + mtab ptr)\n",
                    mtabSize, gulParentDataSize);
  DBG_BUILDNOMCLASS(TRUE, "sci->numStaticMethods: %d\n", sci->ulNumStaticMethods);

  /* Alloc private class struct using SOMCalloc. */
  if((nClass=(NOMClassPriv*)NOMCalloc(1, gulMemSize))==NULL)
    return NULL;

  /* Get mem for method thunking code. This assembler code is needed so the indirect
     jump to the methods from the object pointer which is known does work. For each class
     an individual thunking code must be calculated because the number of instance
     variables is not defined. */
#if 0
  //Moved to addMethodAndDataToThisPrivClassStruct()
  if(0!=sci->ulNumStaticMethods){
    nClass->mThunk=NOMMalloc(sizeof(nomMethodThunk)*sci->ulNumStaticMethods);
    if(!nClass->mThunk) {
      NOMFree(nClass);
      return NULL;
    }
  }
#endif
  /* The size of each instance of this class. A NOM object has a method tab pointer
     at the beginning followed by the instance variables. */
  nClass->ulClassSize=sci->ulInstanceDataSize+gulParentDataSize;
  nClass->ulPrivClassSize=gulMemSize; /* This will not be seen by any user */

  /* Add class struct of this class and the parent one.
     This includes resolving the method adresses of our new class
     (parent ones are already resolved) including adding the thunking
     code . This is essentially done by just copying the parents
     mtab-entries[] to our new one before adding our own methods.
     sci will be saved in nClass->sci */
  //#warning !!!!! Move mem alloc for thunking into this func !!!!!
  if(!addMethodAndDataToThisPrivClassStruct( nClass, ncpParent, sci)){
    NOMFree(nClass);
    return NULL;
  };

  /**********************************/
  /*     Fill methodtable mtab      */
  /**********************************/
  nClass->mtab->mtabSize=mtabSize;
  nClass->mtab->nomClsInfo=(nomClassInfo*)nClass;  /* Hold a pointer to the private data that is this NOMClassPriv */
#ifndef _MSC_VER
#warning !!!!! Change this when nomId is a GQuark !!!!!
#endif
  nClass->mtab->nomClassName=*sci->nomClassId;
  nClass->mtab->ulInstanceSize=sci->ulInstanceDataSize+gulParentDataSize; /* Size of instance data of this class and all
                                                                             parent classes + size of mtab pointer. */

  //#warning !!!!!!!!! No overrride methods yet!!
  /* Resolve ovverrides if any */
  priv_resolveOverrideMethods(nClass, sci);

  BUILDNOMCLASS_LEAVE
  return nClass;
}


/*
  Build NOMClass. This function will only called once when bootstrapping the object system.

  NOMClass the root of all Meta classes. This will also become the meta class of NOMObject, we
  already created. This is done by updating NOMObject when all the structures for NOMClass
  are created and filled with info.
 */
NOMClass * NOMLINK priv_buildNOMClass(gulong ulReserved,
                                      nomStaticClassInfo *sci,
                                      gulong majorVersion,
                                      gulong minorVersion)
{
  NOMClassPriv *nClass;
  gulong ulParentDataSize=0;
  NOMClassPriv *ncpParent;
  NOMClass *nomClass;

#ifdef DEBUG_BUILDNOMCLASS
  nomParentMtabStructPtr pParentMtab;
  nomMethodTabs psmTab;
  nomPrintf("\n%d: Entering %s\n", __LINE__, __FUNCTION__);
  _dumpSci(sci);
#endif

  /* We don't support replacing NOMObject so we don't have to search
     the class list for the current parent but just take the NOMClassPriv
     pointer of NOMObject we saved in the NOM env. Be aware that this structure
     doesn't have a class object pointer yet. We won't use it but just
     as a remark if you want to screw this code... */
  if(pGlobalNomEnv->ncpNOMObject)
    ncpParent=pGlobalNomEnv->ncpNOMObject;
  else{
    g_error("No NOMObject while trying to build NOMClass."); /* This will result in a termination of the app! */
    return NULL;                                       /* NOMClass *must* have an object as parent! 
                                                                We won't reach this point */ 
  }

#ifdef DEBUG_BUILDNOMCLASS
  pParentMtab=&ncpParent->parentMtabStruct;
  nomPrintf("    %d:  parent priv class: %s (%x), pParentMtab->mtab %x, next: %x\n",
            __LINE__, ncpParent->mtab->nomClassName,
            ncpParent, pParentMtab->mtab, pParentMtab->next);
  /* climb parent list */
  psmTab=pParentMtab->next;
  while(psmTab) {
    nomPrintf("     next class: %s\n", psmTab->mtab->nomClassName);
    psmTab=psmTab->next;
  }
#endif

  /* Build the NOMClassPriv for NOMClass */
  if((nClass=buildNOMClassPrivStruct(sci, ncpParent))==NULL)
    return NULL;

  ulParentDataSize=ncpParent->mtab->ulInstanceSize; /* Parent instance size. This is the mtab pointer + instance vars */
  //nomPrintf("%s line %d: SOMClassPriv: %x    ulParentDataSize  %d\n", __FILE__, __LINE__, nClass, sci->ulInstanceDataSize+ulParentDataSize);

  /* And now the NOMClass struct. A NOMClass has a mTab pointer at the beginning and the instance data
     following (including the parent instance data).*/
  if((nomClass=(NOMClass*)NOMCalloc(1, sci->ulInstanceDataSize+ulParentDataSize))==NULL) {
    NOMFree(nClass->mThunk);
    NOMFree(nClass);
    return NULL;
  }

  nomClass->mtab=nClass->mtab; /* Now it's an object */

  nClass->mtab->nomClassObject=nomClass;
  sci->nomCds->nomClassObject=nomClass; /* Put class pointer in static struct. Meta class of SOMClass is SOMClass */
  *nClass->entries0=nomClass;
  /* Mark that we are a metaclass */
  //nClass->ulIsMetaClass=1;
  nClass->ulClassFlags|=NOM_FLG_IS_METACLASS;

  fillCClassDataStructParentMtab(sci, nClass, nomClass);

  /* 
     Note:

     SOMClass is almost ready now. What's missing is the class object for SOMObject (which is SOMClass
     derived from). The class object pointer must be found in mtab->entries[0]. We have to create
     a class object for SOMObject now, update it properly put the pointer into our mtab and update
     the already built SOMObject.
  */


   /* The NOMClass is built. Insert it as meta class into the NOMObject class. 
      The instance vars must be properly setup otherwise this object won't be
      able to create instances. Put it as the class object into the static
      class data sturcture of NOMObject. Update the mtab of our just built
      SOMClass. */
  nomClass->mtab->entries[0]=
    (nomMethodProc*) createNOMObjectClassObjectAndUpdateNOMObject(nomClass, nClass,
                                                                  sci->ulInstanceDataSize+ulParentDataSize);

  pGlobalNomEnv->nomObjectMetaClass=(NOMClass*)nomClass->mtab->entries[0];

    DBG_BUILDNOMCLASS(TRUE, "mtab: %x New class ptr (class object SOMClass): %x (SOMClassPriv: %x) for %s\n",
                      nomClass->mtab, nomClass, nClass, *sci->nomClassId);

  pGlobalNomEnv->defaultMetaClass=nomClass;

#ifndef _MSC_VER
#warning !!!!! _nomSetInstanceSize() not called !!!!!
#endif
#if 0
  /* Set this class size into instance var */
  _somSetInstanceSize(somClass, sClass->ulClassSize /* sci->instanceDataSize+ulParentDataSize*/ ); /* This includes exactly one mtab pointer */
#endif

  /* Run initialization code if any */
  _nomInit(nomClass, NULL);
  return nomClass;
}





