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
#include <stdlib.h>

#include <glib.h> 
#include "parser.h"

extern GScanner *gScanner;
extern PPARSEINFO pParseInfo;

/**
   Parse the metaclass name for this class.

   The current token is the 'metaclass' keyword.

   MC:= G_TOKEN_SYMBOL '=' G_TOKEN_IDENTIFIER ';'
 */

void parseMetaClass(void)
{
  GTokenValue value;

  if(!matchNext('='))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ';',
                            NULL,
                            NULL,
                            NULL,
                            "Error in metaclass definition.",
                            TRUE); /* is_error */
      exit(1);
    }

  /* Get the identifier */
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Class name is not a valid identifier.",
                            TRUE); /* is_error */
      exit(1);
    }
  value=gScanner->value;
  pParseInfo->pCurInterface->chrMetaClass=g_strdup(value.v_identifier);

  if(!matchNext(';'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ';',
                            NULL,
                            NULL,
                            NULL,
                            "Error in metaclass definition.",
                            TRUE); /* is_error */
      exit(1);
    }
}

