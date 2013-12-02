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
 * Portions created by the Initial Developer are Copyright (C) 2006-2007
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
#ifndef NOM_NOMString_IMPLEMENTATION_FILE
#define NOM_NOMString_IMPLEMENTATION_FILE
#endif

#if __OS2__
#define INCL_DOS
#include <os2.h>
#endif /* __OS2__ */

#include <nom.h>
#include <nomtk.h>

#include "nomstring.ih"


NOMDLLEXPORT NOM_Scope NOMString* NOMLINK impl_NOMString_assign(NOMString* nomSelf,
                                                                const NOMString* nomString,
                                                                CORBA_Environment *ev)
{
  /* NOMStringData* nomThis = NOMStringGetData(nomSelf); */
  
  NOMString_assignString(nomSelf, NOMString_queryString(nomString, NULL), NULL);
  return nomSelf;
}


/* Assign a C string to this NOMString. An initially created NOMString object is empty. */
NOMDLLEXPORT NOM_Scope NOMString* NOMLINK impl_NOMString_assignString(NOMString* nomSelf,
                                                                      const CORBA_char* chrString,
                                                                      CORBA_Environment *ev)
{
  NOMStringData* nomThis = NOMStringGetData(nomSelf);

  g_string_assign(_gString, chrString); /* This copies the input string */
  return nomSelf;
}


/* Returns the C string held by this NOMString. */
NOMDLLEXPORT NOM_Scope string NOMLINK impl_NOMString_queryString(NOMString* nomSelf,
                                                                 CORBA_Environment *ev)
{
  NOMStringData* nomThis = NOMStringGetData(nomSelf);
  
  return _gString->str;
}


static PNOMString NOMLINK _appendString(NOMString* nomSelf, const CORBA_char * chrString,
                                        CORBA_Environment *ev)
{
  NOMStringData* nomThis=NOMStringGetData(nomSelf);
  
  g_string_append(_gString, chrString);
  
  return nomSelf;
}


NOMDLLEXPORT NOM_Scope NOMString* NOMLINK impl_NOMString_append(NOMString* nomSelf,
                                                                const NOMString* nomString,
                                                                CORBA_Environment *ev)
{
  /* NOMStringData* nomThis = NOMStringGetData(nomSelf); */

  return _appendString(nomSelf, NOMString_queryString(nomString, NULL), NULL);
}


static PNOMString NOMLINK _prependString(NOMString* nomSelf, const CORBA_char * chrString, CORBA_Environment *ev)
{
  NOMStringData* nomThis=NOMStringGetData(nomSelf);
  g_string_prepend(_gString, chrString);
  
  return nomSelf;
}


NOMDLLEXPORT NOM_Scope NOMString* NOMLINK impl_NOMString_prepend(NOMString* nomSelf,
    const NOMString* nomString,
    CORBA_Environment *ev)
{
  /* NOMStringData* nomThis = NOMStringGetData(nomSelf); */

  return _prependString(nomSelf, NOMString_queryString(nomString, NULL), NULL);
}


NOMDLLEXPORT NOM_Scope gulong NOMLINK impl_NOMString_length(NOMString* nomSelf,
                                                            CORBA_Environment *ev)
{
  NOMStringData* nomThis = NOMStringGetData(nomSelf);

  return _gString->len;
}


NOMDLLEXPORT NOM_Scope PNOMString NOMLINK impl_NOMString_truncate(NOMString* nomSelf,
                                                                  const gulong ulNewLen,
                                                                  CORBA_Environment *ev)
{
  NOMStringData* nomThis = NOMStringGetData(nomSelf);
  
  g_string_truncate(_gString, ulNewLen);
  
  return nomSelf;
}


NOMDLLEXPORT NOM_Scope NOMString* NOMLINK impl_NOMString_copy(NOMString* nomSelf,
                                                              CORBA_Environment *ev)
{
  /* NOMStringData* nomThis = NOMStringGetData(nomSelf); */
  NOMString* nomRetval;
  
  /* We don't know which class we're actually. So we can't just create a new NOMString here.
   It is possible that we are called by a subclass. So use the instance method new() which takes
   any subclassing into account. */
  nomRetval=(PNOMString)NOMString_new(nomSelf, NULL);
  
  NOMString_assign(nomRetval, nomSelf, ev);
  
  return nomRetval;
}


NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMString_nomInit(NOMString* nomSelf,
                                                           CORBA_Environment *ev)
{
  NOMStringData* nomThis = NOMStringGetData(nomSelf);

  /* call parent */
  NOMString_nomInit_parent(nomSelf, ev);

  /* Alloc a zero length GString */
  _gString=g_string_new("");
}

