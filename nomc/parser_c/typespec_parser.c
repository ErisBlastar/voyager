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
# include <os2.h>
#endif /* __OS2__ */

#include <stdio.h>

#include <glib.h> 

#include "parser.h"

extern gchar* getTypeSpecStringFromCurToken(void);

/*
 Parse a typespec e.g. 'gulong' or 'gulong*'.
 
 Current token is the one directly before the typespec.
 Note: only single word typespecs are allowed.
 
 TS:= TYPESPEC
 |  TYPESPEC '*'
 
 */
void parseTypeSpec(PMETHODPARAM pMethodParam)
{
  char *chrTemp;
  
  exitIfNotMatchNextKind(KIND_TYPESPEC, "Expected return type specifier.");
  
  /* Return type name */
  pMethodParam->chrType=getTypeSpecStringFromCurToken();
  
  /* Do we return a pointer (check for '*') */
  while(matchNext('*'))
    pMethodParam->uiStar++;
}

#if 0
/*
  Parse a typespec e.g. 'gulong' or 'gulong*'.

  Current token is the typespec.

  TS:= TYPE_SPEC
    |  TYPE_SPEC TYPE_SPEC      // This is for something like 'unsigned long'
    |  TYPE_SPEC '*'
    |  TYPE_SPEC TYPE_SPEC '*'  // This is for something like 'unsigned long*'
*/
void parseTypeSpec(PMETHODPARAM pMethodParam)
{
  char *chrTemp;

  /* Return type part 1 */
  chrTemp=getTypeSpecStringFromCurToken();

  /* A second typespec part (e.g. 'unsigned long')? */
  if(matchNextKind(KIND_TYPESPEC))
    {
      char *chrTemp2=getTypeSpecStringFromCurToken();
      pMethodParam->chrType=g_strconcat(chrTemp, " ", chrTemp2 ,NULL);
      g_free(chrTemp2);
      g_free(chrTemp);
    }
  else{
    /* Return type  */
    pMethodParam->chrType=chrTemp;
  }

  /* Do we return a pointer (check for '*') */
  while(matchNext('*'))
    pMethodParam->uiStar++;
}
#endif

