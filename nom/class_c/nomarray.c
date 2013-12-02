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
/** \file nomarray.c 
 
 Implementation file for the NOM class NOMArray.
 */

#ifndef NOM_NOMArray_IMPLEMENTATION_FILE
#define NOM_NOMArray_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
#define INCL_DOS
#include <os2.h>
#endif /* __OS2__ */

#include <nom.h>
#include <nomtk.h>

#include "nomarray.ih"

/**
 \brief This function implements the method length() of class NOMArray which returns the number of elements.
 */
NOMDLLEXPORT NOM_Scope gulong NOMLINK impl_NOMArray_length(NOMArray* nomSelf,
    CORBA_Environment *ev)
{
  NOMArrayData* nomThis = NOMArrayGetData(nomSelf);

  return _pPtrArray->len;
}

/**
 \brief This function implements the method append() of class NOMArray which adds an object to the end of the array.
 */
NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMArray_append(NOMArray* nomSelf,
    const NOMObject* pItem,
    CORBA_Environment *ev)
{
  NOMArrayData* nomThis = NOMArrayGetData(nomSelf);

  g_ptr_array_add(_pPtrArray, (gpointer) pItem);  
}


/**
 \brief This function implements the method queryObjectAtIdx() of class NOMArray.

 Idx is running from 0 to length()-1
 */
NOMDLLEXPORT NOM_Scope NOMObject* NOMLINK impl_NOMArray_queryObjectAtIdx(NOMArray* nomSelf,
                                                                         const gulong guIdx,
                                                                         CORBA_Environment *ev)
{
  NOMArrayData* nomThis = NOMArrayGetData(nomSelf);
  
  return (NOMObject*) g_ptr_array_index(_pPtrArray, guIdx);
}


/**
 \brief This function implements the overriden method nomInit() of class NOMArray which allocates our
 internal array data sructure.
 */
NOMDLLEXPORT NOM_Scope void NOMLINK impl_NOMArray_nomInit(NOMArray* nomSelf,
    CORBA_Environment *ev)
{
  NOMArrayData* nomThis = NOMArrayGetData(nomSelf);

  /* call parent */
  NOMArray_nomInit_parent(nomSelf, ev);
  
  /* */
  _pPtrArray=g_ptr_array_new();
  g_assert(_pPtrArray);
}

