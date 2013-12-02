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
#ifdef __OS2__
# include <os2.h>
#endif /* __OS2__ */

#include <stdlib.h>

#include <glib.h> 
#include <glib/gprintf.h> 
#include "parser.h"

extern GScanner *gScanner;

/* In override_parser.c */
extern PINTERFACE findInterfaceForMethod(PINTERFACE pStartInterface, gchar* chrMethodName);


static PMETHOD createMethodStruct()
{
  PMETHOD pMethod=g_malloc0(sizeof(METHOD));

  pMethod->pParamArray=g_ptr_array_new();

  return pMethod;
}

/*
  Parse one or more method parameters. Note that the check for '(' and ')' is
  done in the calling function.

  MPARAMS:= DIRECTION TS G_TOKEN_IDENTIFIER
          | MPARAMS ',' MPARAMS 
 */
static void parseMethodParams(PMETHOD pMethod)
{
  GTokenValue value;
  PMETHODPARAM pParam;
  PSYMBOL pCurSymbol;

  do{ 
    pParam=g_malloc0(sizeof(METHODPARAM));

    /* Direction */
    if(!matchNextKind(KIND_DIRECTION)) /* Be aware that we don't compare types here */
      {
#if 0
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Error in method declaration, direction (in|out|inout) is not specified.",
                              TRUE); /* is_error */
        cleanupAndExit(1);
#endif
        pParam->uiDirection=PARM_DIRECTION_IN;        
      }
    else
    {
      value=gScanner->value;
      pCurSymbol=value.v_symbol;
      switch(pCurSymbol->uiSymbolToken)
      {
        case IDL_SYMBOL_IN:
          pParam->uiDirection=PARM_DIRECTION_IN;
          break;
        case IDL_SYMBOL_OUT:
          pParam->uiDirection=PARM_DIRECTION_OUT;
          break;
        case IDL_SYMBOL_INOUT:
          pParam->uiDirection=PARM_DIRECTION_INOUT;
          break;
        default:
          break;
      }
    }
    
    parseTypeSpec(pParam);
#if 0
    /* Typespec */
    if(matchNextKind(KIND_TYPESPEC)) /* Be aware that we don't compare types here */
      parseTypeSpec(pParam);
    else
      {
        getNextToken(); /* Make sure error references the correct token */
        g_scanner_unexp_token(gScanner,
                              G_TOKEN_IDENTIFIER,
                              NULL,
                              NULL,
                              NULL,
                              "Error in method declaration, Unknown type specification.",
                              TRUE); /* is_error */
        cleanupAndExit(1);
      }
#endif
    //pParam->chrType=getTypeSpecStringFromCurToken();
    //g_printf("%s %d", __FUNCTION__, __LINE__);
    //printToken(curToken);
    exitIfNotMatchNext(G_TOKEN_IDENTIFIER, "Error in method declaration. Wrong parameter name.");

    value=gScanner->value;
    pParam->chrName=g_strdup(value.v_identifier);

    g_ptr_array_add(pMethod->pParamArray , (gpointer) pParam);
  }while(matchNext(','));
}




static void doSingleMethodParam(void)
{
  GTokenValue value;
  PSYMBOL pCurSymbol;

  PMETHODPARAM pParam;
  pParam=g_malloc0(sizeof(METHODPARAM));

  /* Do direction */
  /* Direction */
  if(!matchNextKind(KIND_DIRECTION)) /* Be aware that we don't compare types here */
  {
    pParam->uiDirection=PARM_DIRECTION_IN;        
  }
  else
  {
    value=gScanner->value;
    pCurSymbol=value.v_symbol;
    switch(pCurSymbol->uiSymbolToken)
    {
      case IDL_SYMBOL_IN:
        pParam->uiDirection=PARM_DIRECTION_IN;
        break;
      case IDL_SYMBOL_OUT:
        pParam->uiDirection=PARM_DIRECTION_OUT;
        break;
      case IDL_SYMBOL_INOUT:
        pParam->uiDirection=PARM_DIRECTION_INOUT;
        break;
      default:
        break;
    }
  }
  
  
  /* Do typespec */
  
  /* Do identifier */
  exitIfNotMatchNext(G_TOKEN_IDENTIFIER, "Error in method implementation.");
  
  /* The current token is the identifier */
  value=gScanner->value;
  pParam->chrName=g_strdup(value.v_identifier);
  
  
}

/*
 
 MPARAMS :=  ˚)˚
           | MPARAM
           | MPARAM ',' MPARAMS 
 */
static void doMethodParams(PMETHOD pMethod)
{
  
  while(g_scanner_peek_next_token(gScanner)!= G_TOKEN_EOF && g_scanner_peek_next_token(gScanner)!=')')
  {
    doSingleMethodParam();
    
    if(!matchNext(','))
      break;
  }
  exitIfNotMatchNext(')',  "No closing of method parameter list.");
}

/*
 
 MPARAM :=  TYPESPEC IDENT 
 
 */


/*
 Current token is the one just before the typespec.
 
 METHOD := TYPESPEC IDENT '(' MPARAMS

 */
static void parseMethod(PMETHOD pMethod)
{
  GTokenValue value;
  PINTERFACE pif;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  /********* Parse typespec *******************************************/  


  /********* Method identifier ****************************************/  

  exitIfNotMatchNext(G_TOKEN_IDENTIFIER, "Error in method implementation.");

  /* The current token is the identifier */
  value=gScanner->value;
  pMethod->chrName=g_strdup(value.v_identifier);
  
  /* Now check if the method was introduced by some parent. The interface struct contains
   the parent name if any and the function will follow the chain of parents. */
  if((pif=findInterfaceForMethod(pParseInfo->pCurInterface, pMethod->chrName))!=NULL)
  {
    gchar* chrMessage;
    chrMessage=g_strdup_printf("A method '%s' is already present in interface '%s'.",
                               pMethod->chrName, pif->chrName);
    
    g_scanner_unexp_token(gScanner,
                          G_TOKEN_IDENTIFIER,
                          NULL, NULL, NULL,
                          chrMessage,
                          TRUE); /* is_error */
    cleanupAndExit(1);
  }
  
  /* Check if the method is defined at all */
  if(NULL==findMethodInfoFromMethodNameWithCurrent(pParseInfo->pClassDefinition, pMethod->chrName))
  {
    gchar* chrMessage;
    chrMessage=g_strdup_printf("Method '%s' is not defined for the current class or one of the parents.",
                               pMethod->chrName);
    
    g_scanner_unexp_token(gScanner,
                          G_TOKEN_IDENTIFIER,
                          NULL, NULL, NULL,
                          chrMessage,
                          TRUE); /* is_error */
    cleanupAndExit(1);
    
  }

  /****** Method parameters ***************************************************/  
  exitIfNotMatchNext('(', "Error in method implementation.");

  /* Do parameters */
  
}

/*
 
 CLASSMETHODS:=  CLASSMETHOD
               | CLASSMETHOD CLASSMETHODS
 
 CLASSMETHOD := IMPL METHOD
 
 */
static void parseSingleClassMethod(void)
{
  GTokenValue value;
  PSYMBOL pCurSymbol;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  PMETHOD pMethod=createMethodStruct();

  g_ptr_array_add( pParseInfo->pCurInterface->pMethodArray, (gpointer) pMethod);

  /* Method implementations must start with "impl" which is registered as a symbol. Here we check if
   the token is a symbol. */
  exitIfNotMatchNext(G_TOKEN_SYMBOL, "Method implementation must start with 'impl'.");
  
  value=gScanner->value;
  pCurSymbol=value.v_symbol;
  
  /* Check if symbol is "impl". */
  if(!pCurSymbol || pCurSymbol->uiSymbolToken!=NOMC_SYMBOL_PUBLIC)
  {
    g_scanner_unexp_token(gScanner,
                          G_TOKEN_SYMBOL,
                          NULL, NULL, NULL,
                          "'impl'.",
                          TRUE); /* is_error */
    cleanupAndExit(1);
  }
  
  /* Parse method */
  parseMethod(pMethod);
}

/*
 
 CLASSMETHODS:=  IMPL METHOD
               | IMPL METHOD  CLASSMETHODS
*/
 void parseClassMethods(void)
{
  do
  {
    PSYMBOL pCurSymbol;
    GTokenValue value;

    /* There must be at least one class method */
    parseSingleClassMethod();
    
    /* Any additional methods start with ˚impl˚ */
    if(g_scanner_peek_next_token(gScanner)!=G_TOKEN_SYMBOL)
      break;

    value=gScanner->next_value;
    pCurSymbol=value.v_symbol;

    if(!pCurSymbol || pCurSymbol->uiSymbolToken!=NOMC_SYMBOL_PUBLIC)
      break;
  }while(g_scanner_peek_next_token(gScanner)!= G_TOKEN_EOF);
}

/*
  Current token is the typespec.

  TS:= TYPE_SPEC              //typespec, see elsewhere
    |  TYPE_SPEC '*'

  M:= TS G_TOKEN_IDENTIFIER '(' ')' BLOCK        // method
    | TS G_TOKEN_IDENTIFIER '(' MPARMS ')' BLOCK  // method
 */
void parseClassMethod(void)
{
  GTokenValue value;
  PMETHOD pMethod=createMethodStruct();
  PINTERFACE pif;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  
  /* Do type spec */
  parseTypeSpec(&pMethod->mpReturn);
  
  /* Method name G_TOKEN_IDENTIFIER */  
  exitIfNotMatchNext(G_TOKEN_IDENTIFIER, "Error in method declaration.");
  
  value=gScanner->value;
  pMethod->chrName=g_strdup(value.v_identifier);

  /* Now check if the method was introduced by some parent. The interface struct contains
     the parent name if any and the function will follow the chain of parents. */
  if((pif=findInterfaceForMethod(pParseInfo->pCurInterface, pMethod->chrName))!=NULL)
    {
      gchar* chrMessage;
      chrMessage=g_strdup_printf("A method '%s' is already present in interface '%s'.",
                                 pMethod->chrName, pif->chrName);

      g_scanner_unexp_token(gScanner,
                            G_TOKEN_IDENTIFIER,
                            NULL, NULL, NULL,
                            chrMessage,
                            TRUE); /* is_error */
      cleanupAndExit(1);
    }

  /* Check if the method is defined at all */
  if(NULL==findMethodInfoFromMethodNameWithCurrent(pParseInfo->pClassDefinition, pMethod->chrName))
  {
    gchar* chrMessage;
    chrMessage=g_strdup_printf("Method '%s' is not defined for the current class or one of the parents.",
                               pMethod->chrName);
    
    g_scanner_unexp_token(gScanner,
                          G_TOKEN_IDENTIFIER,
                          NULL, NULL, NULL,
                          chrMessage,
                          TRUE); /* is_error */
    cleanupAndExit(1);
    
  }

  /**** Do parameters if any ****/

  exitIfNotMatchNext('(', "Error in method declaration");

  
  /* No parameters */
  if(matchNext(')'))
  {
 
  }
  else
  {
    /* This parses all the parameters */
    parseMethodParams(pMethod);
    exitIfNotMatchNext(')', "Error in method declaration");
  }

  g_ptr_array_add( pParseInfo->pCurInterface->pMethodArray, (gpointer) pMethod);

  emitMethodImplHeader(pParseInfo->pCurInterface, pMethod);
  
  pParseInfo->fPrintToken=TRUE;
  parseCodeBlock();
  pParseInfo->fPrintToken=FALSE;
}
  
