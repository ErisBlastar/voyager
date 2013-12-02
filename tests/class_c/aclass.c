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
* Portions created by the Initial Developer are Copyright (C) 2005-2006
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
#ifndef NOM_AClass_IMPLEMENTATION_FILE
#define NOM_AClass_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
# define INCL_DOS
# include <os2.h>
#endif /* __OS2__ */

#include "glib.h"
#include "nom.h"
#include <nomtk.h>

#include "aclass.ih"

NOM_Scope void NOMLINK impl_AClass_tstPrintHello(AClass* nomSelf, CORBA_Environment *ev)
{
/* AClassData* nomThis=AClassGetData(nomSelf); */
  g_message("Hello at %x", impl_AClass_tstPrintHello);
}

NOM_Scope gulong NOMLINK impl_AClass_tstQueryUlongVar1(AClass* nomSelf, CORBA_Environment *ev)
{
  AClassData* nomThis=AClassGetData(nomSelf);

  return _ulVar1;
}

NOM_Scope gulong NOMLINK impl_AClass_tstQueryUlongVar2(AClass* nomSelf, CORBA_Environment *ev)
{
  AClassData* nomThis=AClassGetData(nomSelf);

  return _ulVar2;
}

NOM_Scope void NOMLINK impl_AClass_tstSetUlongVar1(AClass* nomSelf, const gulong ulNew, CORBA_Environment *ev)
{
  AClassData* nomThis=AClassGetData(nomSelf);

  g_message("In %s, setting ulVar1 to 0x%lx", __FUNCTION__, ulNew);
  _ulVar1=ulNew;
}

NOM_Scope void NOMLINK impl_AClass_tstSetUlongVar2(AClass* nomSelf, const gulong ulNew, CORBA_Environment *ev)
{
  AClassData* nomThis=AClassGetData(nomSelf);

  g_message("In %s, setting ulVar2 to 0x%lx", __FUNCTION__, ulNew);
  _ulVar2=ulNew;
}


