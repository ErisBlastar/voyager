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
* Portions created by the Initial Developer are Copyright (C) 2007
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
# define INCL_DOSPROCESS
# define INCL_DOS
# define INCL_DOSPROFILE
# define INCL_DOSERRORS
# include <os2.h>
#endif /* __OS2__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_IO_H
# include <io.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#include <glib.h> 
#include <glib/gprintf.h>

#include "nom.h"
#include "nomtk.h"
#include "nomgc.h"

#include "aclass.h"
#include "bclass.h"

/* For unit test */
#include "nomarray.h"
#include "nomstring.h"
#include "nommethod.h"
#include "nomtestresult.h"
#include "testnomobject.h"
#include "testnomclassmgr.h"


#define ULONG_TESTVALUE_1         0xffeeddcc
#define ULONG_TESTVALUE_2         0x55aa1122
#define ULONG_TESTVALUE_BCLASS_1  0x50403020
#define ULONG_TESTVALUE_BCLASS_2  0xf0e0d0c0

AClass*  createAClassObject()
{
  AClass*  aClass;
  
  aClass=AClassNew();
  
  if(nomIsObj(aClass))
    g_message("AClass creation\t\t\t\tOK\n");
  else
    g_message("AClass creation\t\t\t\t\tFAILED\n");
  return aClass;
}

AClass*  createBClassObject()
{
  BClass*  bClass;
  
  bClass=BClassNew();
  
  if(nomIsObj(bClass))
    g_message("BClass creation\t\t\t\tOK\n");
  else
    g_message("BClass creation\t\t\t\t\tFAILED\n");
  return bClass;
}

void tstAClassInstanceVarInitValues(AClass * aObject)
{
  gulong ulRC;

  g_message("================================================================");
  g_message("===== Testing init values of instance variables. Must be 0 =====");
  g_message("================================================================");
  
  ulRC=_tstQueryUlongVar1(aObject, NULL);
  g_message("Calling tstQueryUlongVar1():\t%ld\t\t%s", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);
  
  ulRC=_tstQueryUlongVar2(aObject, NULL);
  g_message("Calling tstQueryUlongVar2():\t%ld\t\t%s\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);
}


void tstBClassInstanceVarInitValues(BClass * aObject)
{
  gulong ulRC;

  g_message("================================================================");
  g_message("===== Testing init values of instance variables. Must be 0 =====");
  g_message("================================================================");
  
  ulRC=_tstQueryBClassUlongVar1(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar1():\t%ld\t\t%s", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);
  
  ulRC=_tstQueryBClassUlongVar2(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar2():\t%ld\t\t%s\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);
}


void tstSetAClassInstanceVar(AClass * aObject)
{
  gulong ulRC;

  g_message("========================================================");
  g_message("===== Testing setting of AClass instance variables =====");
  g_message("========================================================");
  /* Set 1. value */
  _tstSetUlongVar1(aObject, ULONG_TESTVALUE_1, NULL);
  ulRC=_tstQueryUlongVar1(aObject, NULL);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);

  ulRC=_tstQueryUlongVar2(aObject, NULL);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t\t%s\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);

  /* Set 2. value */
  _tstSetUlongVar2(aObject, ULONG_TESTVALUE_2, NULL);
  ulRC=_tstQueryUlongVar1(aObject, NULL);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);

  ulRC=_tstQueryUlongVar2(aObject, NULL);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t%s\n\n", ulRC, (ULONG_TESTVALUE_2!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_2==ulRC);
}

void tstSetBClassInstanceVar(BClass * aObject)
{
  gulong ulRC;

  g_message("========================================================");
  g_message("===== Testing setting of BClass instance variables =====");
  g_message("========================================================");
  /* Set 1. value */
  _tstSetBClassUlongVar1(aObject, ULONG_TESTVALUE_BCLASS_1, NULL);

  /* AClass */
  ulRC=_tstQueryUlongVar1(aObject, NULL);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);
  ulRC=_tstQueryUlongVar2(aObject, NULL);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_2!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_2==ulRC);

  /* BClass*/
  ulRC=_tstQueryBClassUlongVar1(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_BCLASS_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_BCLASS_1==ulRC);

  ulRC=_tstQueryBClassUlongVar2(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar2():\t0x%lx\t\t\t%s\n\n", ulRC, (0!=ulRC ? "FAILED" : "OK"));
  g_assert(0==ulRC);



  /* Set 2. value */
  _tstSetBClassUlongVar2(aObject, ULONG_TESTVALUE_BCLASS_2, NULL);

  /* AClass */
  ulRC=_tstQueryUlongVar1(aObject, NULL);
  g_message("Calling tstQueryUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_1==ulRC);
  ulRC=_tstQueryUlongVar2(aObject, NULL);
  g_message("Calling tstQueryUlongVar2():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_2!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_2==ulRC);

  /* BClass*/
  ulRC=_tstQueryBClassUlongVar1(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar1():\t0x%lx\t\t%s", ulRC, (ULONG_TESTVALUE_BCLASS_1!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_BCLASS_1==ulRC);

  ulRC=_tstQueryBClassUlongVar2(aObject, NULL);
  g_message("Calling tstQueryBClassUlongVar2():\t0x%lx\t\t%s\n\n", ulRC, (ULONG_TESTVALUE_BCLASS_2!=ulRC ? "FAILED" : "OK"));
  g_assert(ULONG_TESTVALUE_BCLASS_2==ulRC);

}


static void printTestResults(NOMArray* nArray)
{
  int a;
  
  for(a=0; a < NOMArray_length(nArray, NULL) ; a++)
  {
    g_message("Test '%s()': %s", _queryString(NOMTestResult_queryName(NOMArray_queryObjectAtIdx(nArray, a, NULL), NULL), NULL),
              NOMTestResult_success(NOMArray_queryObjectAtIdx(nArray, a, NULL), NULL) ? "Ok" : "Not ok");
  }
}


/**
   Main entry point for the idl compiler.
 */
int main(int argc, char **argv)
{
  NOMClassMgr *NOMClassMgrObject;
  HREGDLL hReg=NULL;
  AClass*  aObject;
  BClass*  bObject;

  /* Unit test */
  NOMArray* nArray;
  TestNomObject* tstNomObject;
  TestNOMClassMgr* tstNOMClassMgr;
  int a;

#if 0
  /* Preload the DLL otherwise it won't be found by the GC registering function */
  if((rc=DosLoadModule(uchrError, sizeof(uchrError),"nobjtk.dll", &hModuleGC))!=NO_ERROR)
    {
      printf("DosLoadmodule for nobjtk.dll failed with rc=0x%x because of module %s.\n", (int)rc, uchrError);
      return 1;
    };
  fprintf(stderr, "DLL handle for nobjtk.dll is: 0x%x\n", (int)hModuleGC);
#endif
  nomInitGarbageCollection(NULL);

  /* Register DLLs with the garbage collector */
  hReg=nomBeginRegisterDLLWithGC();
  if(NULL==hReg)
    return 1;

#if 0
  //g_assert(nomRegisterDLLByName(hReg, "GLIB2.DLL" ));
  //g_assert(nomRegisterDLLByName(hReg, "GOBJECT2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GMODULE2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GDK2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GDKPIX2.DLL"));
  g_assert(nomRegisterDLLByName(hReg, "GTK2.DLL" ));
  g_assert(nomRegisterDLLByName(hReg, "ATK.DLL" ));
#endif
  g_assert(nomRegisterDLLByName(hReg, "NOBJTK.DLL"));

  nomEndRegisterDLLWithGC(hReg);

  g_message("NOM test application started.");

  /* Init NOM */
  NOMClassMgrObject=nomEnvironmentNew();
  
  g_message("\n");
  g_message("================================================================");
  g_message("=====          Testing AClass, child of NOMObject          =====");
  g_message("================================================================");
  /* Try to create an AClass object */
  aObject=createAClassObject();
  g_assert(aObject);

  /* -- Call methods on the object --- */
  tstAClassInstanceVarInitValues(aObject);
  tstSetAClassInstanceVar(aObject);

  g_message("================================================================");
  g_message("=====          Testing BClass, child of AClass             =====");
  g_message("================================================================");
  /* Try to create an AClass object */
  bObject=createBClassObject();
  g_assert(bObject);

  tstAClassInstanceVarInitValues(bObject);
  tstBClassInstanceVarInitValues(bObject);

  tstSetAClassInstanceVar(bObject);
  tstSetBClassInstanceVar(bObject);

  nomPrintf("\n");
  g_message("================================================================");
  g_message("=====          Testing NOMObject base class                =====");
  g_message("================================================================");
  tstNomObject=TestNomObjectNew();
  _setClassMgrObject(tstNomObject, NOMClassMgrObject, NULL);
  nArray=_runTests(tstNomObject, NULL);
  nomPrintf("\n");
  printTestResults(nArray);

  nomPrintf("\n");
  g_message("================================================================");
  g_message("=====          Testing NOMClassMgr base class              =====");
  g_message("================================================================");
  tstNOMClassMgr=TestNOMClassMgrNew();
  _setClassMgrObject(tstNOMClassMgr, NOMClassMgrObject, NULL);
  nArray=_runTests(tstNOMClassMgr, NULL);
  nomPrintf("\n");
  printTestResults(nArray);
  
  return 0;
};








