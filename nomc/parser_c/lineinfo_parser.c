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
* Portions created by the Initial Developer are Copyright (C) 2007-2008
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
  Parse the line information included by the gcc preprocessor. We use that
  to know the current source file and to calculate a correction value for
  the line numbers so that error messages show the correct source file line.
 */

#ifdef __OS2__
# include <os2.h>
#endif /* __OS2__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include "parser.h"

extern GScanner *gScanner;

/*
  Current token is the first integer.

  LI:= INT STRING INT     // Line info from the preprocessor
 */
void parsePreprocLineInfo(void)
{
  GTokenValue value;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  /* Line number */
  value=gScanner->value;
  //pParseInfo->uiLineCorrection=g_scanner_cur_line(gScanner)-value.v_int+1;
  pParseInfo->uiLineCorrection=value.v_int-g_scanner_cur_line(gScanner)-1;
  
  //g_message("%s: %d %d", __FUNCTION__, g_scanner_cur_line(gScanner), value.v_int);
  
  TST_NEXT_TOKEN_NOT_OK(G_TOKEN_STRING, "Trying to parse preprocessor line information.");


#if 0
  if(!matchNext(G_TOKEN_STRING))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_STRING,
                            NULL,
                            NULL,
                            NULL,
                            "Trying to parse preprocessor line information.",
                            TRUE); /* is_error */
      exit(1);
    }
#endif

  value=gScanner->value;

  /* Root source file? */
  if(!pParseInfo->chrRootSourceFile)
    pParseInfo->chrRootSourceFile=g_strdup(value.v_string);

  /* Current source file */
  if(pParseInfo->chrCurrentSourceFile)
    g_free(pParseInfo->chrCurrentSourceFile);

  pParseInfo->chrCurrentSourceFile=g_strdup(value.v_string);

  /* Trailing file include level info isn't used for now. Note that for the root
     level no trailing int is following. */
  matchNext(G_TOKEN_INT);
#if 0
  if(!matchNext(G_TOKEN_INT))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_INT,
                            NULL,
                            NULL,
                            NULL,
                            "Trying to parse preprocessor line information.",
                            TRUE); /* is_error */
      exit(1);
    }
#endif
}
