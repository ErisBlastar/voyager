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
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h>
#include "parser.h"


extern GScanner *gScanner;
//extern PINTERFACE pCurInterface;
extern PPARSEINFO pParseInfo;

PINTERFACE findInterfaceForMethod(PINTERFACE pStartInterface, gchar* chrMethodName)
{
  int a;

  while(pStartInterface)
    {
      for(a=0;a<pStartInterface->pMethodArray->len;a++)
        {
          PMETHOD pm=(PMETHOD)g_ptr_array_index(pStartInterface->pMethodArray, a);
          if(!strcmp(pm->chrName, chrMethodName))
            return pStartInterface;
        }
      if(pStartInterface->chrParent)
        pStartInterface=findInterfaceFromName(pStartInterface->chrParent);
      else
        pStartInterface=NULL;
    }

  return NULL;
}

/*
  Parse override. Note that 'NOMOVERRIDE' is the current symbol..

  OM:= IDL_SYMBOL_OVERRIDE '(' IDENT ')' ';'
 */
void parseOverrideMethod(void)
{
  GTokenValue value;
  POVERMETHOD pOMethod=g_malloc0(sizeof(OVERMETHOD));
  PINTERFACE pif;

  if(!matchNext('('))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            '(',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMOVERRIDE()",
                            TRUE); /* is_error */
      exit(1);
    }

  /* This is the method we actually try to override */
  if(!matchNext(G_TOKEN_IDENTIFIER))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMOVERRIDE(). Identifier expected.",
                            TRUE); /* is_error */
      exit(1);
    }
  value=gScanner->value;
  pOMethod->chrName=g_strdup(value.v_identifier);

  /* Now check if the method was introduced by some parent */
  if((pif=findInterfaceForMethod(pParseInfo->pCurInterface, pOMethod->chrName))==NULL)
    {
      g_printf("%s:%d: error: Method '%s' was not introduced by some parent interface.\n",
               pParseInfo->chrCurrentSourceFile, g_scanner_cur_line(gScanner)-pParseInfo->uiLineCorrection,
               pOMethod->chrName);
      exit(1);
    }
  pOMethod->chrIntroducingIFace=pif->chrName; /* No copy of string here. Nobody should free the
                                               interface info under our feet. */

  if(!matchNext(')'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ')',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMOVERRIDE(). Closing ')' is missing.",
                            TRUE); /* is_error */
      exit(1);
    }
  if(!matchNext(';'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ';',
                            NULL,
                            NULL,
                            NULL,
                            "Error in NOMOVERRIDE(). Missing ';' at end of statement.",
                            TRUE); /* is_error */
      exit(1);
    }
  g_ptr_array_add(pParseInfo->pCurInterface->pOverrideArray, (gpointer) pOMethod);
}


/*
  Parse override. Note that the identifier is the current symbol.

  OM:= IDENT ':' IDL_SYMBOL_OVERRIDE2 ';'
 */
void parseOverrideMethodFromIdentifier(void)
{
  GTokenValue value;
  POVERMETHOD pOMethod=g_malloc0(sizeof(OVERMETHOD));
  PINTERFACE pif;

  /* Keep the current identifier. We need to check for existance later if we find
     we are in an override statement. */
  value=gScanner->value;
  pOMethod->chrName=g_strdup(value.v_identifier);

  if(!matchNext(':'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ':',
                            NULL,
                            NULL,
                            NULL,
                            "",
                            TRUE); /* is_error */
      exit(1);
    }

  /* Check for 'override' */
  if(!matchNext(G_TOKEN_SYMBOL))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL,
                            NULL,
                            NULL,
                            "'override' keyword expected.",
                            TRUE); /* is_error */
      exit(1);
    }
  else
    {
      /* Check if symbol is 'override' */
      PSYMBOL pCurSymbol;

      value=gScanner->value;
      pCurSymbol=value.v_symbol;

      if(pCurSymbol->uiSymbolToken != IDL_SYMBOL_OVERRIDE2)
        {
          g_scanner_unexp_token(gScanner,
                                G_TOKEN_SYMBOL,
                                NULL,
                                NULL,
                                NULL,
                                "'override' keyword expected after ':'.",
                                TRUE); /* is_error */
          exit(1);
        }
    }
  
  /* Now check if the method was introduced by some parent */
  if((pif=findInterfaceForMethod(pParseInfo->pCurInterface, pOMethod->chrName))==NULL)
    {
      g_printf("%s:%d: error: Method '%s' was not introduced by some parent interface.\n",
               pParseInfo->chrCurrentSourceFile, g_scanner_cur_line(gScanner)-pParseInfo->uiLineCorrection,
               pOMethod->chrName);
      exit(1);
    }


  pOMethod->chrIntroducingIFace=pif->chrName; /* No copy of string here. Nobody should free the
                                                 interface info under our feet. */

  /* Check for trailing ';' */  
  if(!matchNext(';'))
    {
      getNextToken(); /* Make sure error references the correct token */
      g_scanner_unexp_token(gScanner,
                            ';',
                            NULL,
                            NULL,
                            NULL,
                            "Error in override statement. Trailing ';' is missing.",
                            TRUE); /* is_error */
      exit(1);
    }
  g_ptr_array_add(pParseInfo->pCurInterface->pOverrideArray, (gpointer) pOMethod);
}
