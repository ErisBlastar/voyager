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
 * Portions created by the Initial Developer are Copyright (C) 2005-2008
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
#ifndef NOM_TestNOMClassMgr_IMPLEMENTATION_FILE
#define NOM_TestNOMClassMgr_IMPLEMENTATION_FILE
#endif

#if __OS2__
#define INCL_DOS
#include <os2.h>
#endif /* __OS2__ */

#include <nom.h>
#include <nomtk.h>

#include "testnomclassmgr.ih"

NOMDLLEXPORT NOM_Scope CORBA_boolean NOMLINK impl_TestNOMClassMgr_test_nomFindClassFromName(TestNOMClassMgr* nomSelf,
    CORBA_Environment *ev)
{
  /* TestNOMClassMgrData* nomThis = TestNOMClassMgrGetData(nomSelf); */
  CORBA_boolean nomRetval=FALSE;

  for(;;)
  {
    NOMClass* nClassObject;
    
    if(_TestNOMClassMgr!= _nomFindClassFromName(_queryClassMgrObject(nomSelf, NULL),"TestNOMClassMgr", 0, 0, NULL))
      break;
    
    nClassObject=_nomFindClassFromName(_queryClassMgrObject(nomSelf, NULL),"NOMObject", 0, 0, NULL);
    if(!nomIsObj(nClassObject))
      break;
    
    if(0!=strcmp( _nomQueryCreatedClassName(nClassObject, NULL), "NOMObject"))
      break;
      
    nomRetval=TRUE;
    break;
  }
  return nomRetval;
}

NOMDLLEXPORT NOM_Scope void NOMLINK impl_TestNOMClassMgr_setUp(TestNOMClassMgr* nomSelf,
    CORBA_Environment *ev)
{
  /* TestNOMClassMgrData* nomThis = TestNOMClassMgrGetData(nomSelf); */

  /* call parent */
  TestNOMClassMgr_setUp_parent(nomSelf, ev);

}

NOMDLLEXPORT NOM_Scope void NOMLINK impl_TestNOMClassMgr_tearDown(TestNOMClassMgr* nomSelf,
    CORBA_Environment *ev)
{
  /* TestNOMClassMgrData* nomThis = TestNOMClassMgrGetData(nomSelf); */

  /* call parent */
  TestNOMClassMgr_tearDown_parent(nomSelf, ev);

}

