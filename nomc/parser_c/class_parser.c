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
* Portions created by the Initial Developer are Copyright (C) 2008
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
  Main file containing the class parser. Whenever a valid keyword is found
  a specialized parser function in another source file is called from here.
 */
#ifdef __OS2__
# include <os2.h>
#endif /* __OS2__ */

#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 
#include "parser.h"

extern GScanner *gScanner;


static PINTERFACE createInterfaceStruct()
{
  PINTERFACE pInterface;
  
  pInterface=(PINTERFACE)g_malloc0(sizeof(INTERFACE));

  //pInterface->pInstanceVarArray=g_ptr_array_new();
  pInterface->pMethodArray=g_ptr_array_new();
  //  pInterface->pOverrideArray=g_ptr_array_new();
  
  return pInterface;
}



static void registerInterface(void)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  PSYMBOL pNewSymbol=g_malloc0(sizeof(SYMBOL));
  
  //g_message("In %s for %s", __FUNCTION__, pParseInfo->pCurInterface->chrName);
  
  pParseInfo->pCurInterface->pSymbolIFace=pNewSymbol;
  
  if(!strcmp(pParseInfo->chrRootSourceFile, pParseInfo->pCurInterface->chrSourceFileName))
    pParseInfo->pCurInterface->fIsInRootFile=TRUE;
  
  g_ptr_array_add(pParseInfo->pInterfaceArray, (gpointer) pParseInfo->pCurInterface);
  
  /* Any found interface is registered as a new type so it can be
   used in other classes. */
  pNewSymbol->chrSymbolName=g_strdup(pParseInfo->pCurInterface->chrName); /* We create a copy here because
   when cleaning up the symbol space
   the string will be freed. */
  pNewSymbol->uiKind=KIND_TYPESPEC;
  pNewSymbol->uiSymbolToken=IDL_SYMBOL_REGINTERFACE;
  g_tree_insert(pParseInfo->pSymbolTree, pNewSymbol, pNewSymbol->chrSymbolName);
  g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pNewSymbol->chrSymbolName,
                             pNewSymbol);
  /* For legacy support and convenience we automatically register a pointer type
   to the interface. */
  pNewSymbol=g_malloc0(sizeof(SYMBOL));
  pParseInfo->pCurInterface->pSymbolIFacePtr=pNewSymbol;
  pNewSymbol->uiKind=KIND_TYPESPEC;
  pNewSymbol->uiSymbolToken=IDL_SYMBOL_REGINTERFACE;
  pNewSymbol->chrSymbolName=g_strconcat("P", pParseInfo->pCurInterface->chrName, NULL);
  g_tree_insert(pParseInfo->pSymbolTree, pNewSymbol, pNewSymbol->chrSymbolName);
  g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pNewSymbol->chrSymbolName,
                             pNewSymbol);
  //g_message("%s: %s", __FUNCTION__, pNewSymbol->chrSymbolName);
}

static void deRegisterInterface(PINTERFACE pif)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;  

  /* Remove the interface from our list */
  g_ptr_array_remove(pParseInfo->pInterfaceArray, (gpointer) pif);
  
  /* Any found interface was registered as a new type so it can be
   used in other classes. */
  g_tree_remove(pParseInfo->pSymbolTree, pif->pSymbolIFace);
  
  g_scanner_scope_remove_symbol(gScanner, ID_SCOPE, pif->pSymbolIFace->chrSymbolName);
  /* For legacy support and convenience we automatically registered a pointer type
   to the interface. */
  g_tree_remove(pParseInfo->pSymbolTree, pif->pSymbolIFacePtr);
  g_scanner_scope_remove_symbol(gScanner, ID_SCOPE, pif->pSymbolIFacePtr->chrSymbolName);
  /* We don't clean up. Looking at the whole mess with string dupes and stuff in side the
   structs I just decided to use a GC instead... */
}



/*
  Function to parse the body of a class.
  Current token is '{'.

  CBODY:= '}'   //This is an empty class body
         | CLASSMETHODS '}'
 
 */
static void parseCBody(void)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  while(g_scanner_peek_next_token(gScanner)!= G_TOKEN_EOF && g_scanner_peek_next_token(gScanner)!='}')
  {
    PSYMBOL pCurSymbol;
    GTokenValue value;
    
    /* Method implementations must start with "public" which is registered as a symbol. Here we check if
     the token is a symbol. */
    exitIfNotMatchNext(G_TOKEN_SYMBOL, "Method implementation must start with 'public'.");

    value=gScanner->value;
    pCurSymbol=value.v_symbol;

    /* Check if token is "public". */
    if(!pCurSymbol || pCurSymbol->uiSymbolToken!=NOMC_SYMBOL_PUBLIC)
    {
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL, NULL, NULL,
                            "'impl'.",
                            TRUE); /* is_error */
      cleanupAndExit(1);
    }

    /* Get name, parameters and stuff. Print the body. */
    parseClassMethod();
  };

  exitIfNotMatchNext('}',  "No closing of 'class' section.");
}




/*
  Current token is CLASSIDENT.

  CLASSBODY:=  '{' CBODY
             | '{' CBODY ';'

*/
static void parseClassBody(void)
{
  
  exitIfNotMatchNext('{',  "No opening brace for class body.");
    
  parseCBody();
    
  /* Remove a terminating ';' from the input if present. */
  matchNext(';');
}


/*
 Parse the class name. If we already encountered this class the name is registered as a
 symbol. IF this is the first time the name is an identifier.
 
 Note that the current token is the 'class' keyword.
 
 CLASSIDENT:=  G_TOKEN_IDENTIFIER
             | IDL_SYMBOL_INTERFACE
 */
static gchar* parseClassIdent(void)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  
  if(matchNext(G_TOKEN_IDENTIFIER))
  {
    /* Save interface info */
    GTokenValue value=gScanner->value;

    return g_strdup(value.v_identifier);
  }  
  else
  {
    PSYMBOL pCurSymbol;
    GTokenValue value;

    /* If the interface name is a symbol, it means the interface was
     already registered before. Maybe because of a forward statement.
     We will check that in the function which called us. */    
    exitIfNotMatchNext(G_TOKEN_SYMBOL, "Keyword 'class' must be followed by an identifier");
    
    /* Check if it's one of our interface symbols */
    value=gScanner->value;
    pCurSymbol=value.v_symbol;
    if(IDL_SYMBOL_REGINTERFACE!=pCurSymbol->uiSymbolToken)
    {
      /* No, some other symbol */
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL, NULL, NULL,
                            "Keyword 'class' is not followed by a valid identifier.",
                            TRUE); /* is_error */
      cleanupAndExit(1);
    }
    
    /* Save interface name */
    return g_strdup(pCurSymbol->chrSymbolName);    
  }
}


static gchar* parseParentClassIdent(void)
{
  g_message("Line %d: Error in class declaration",  g_scanner_cur_line(gScanner));
  cleanupAndExit(0);
  
  return NULL;
}


static void doForwardClassDeclaration(void)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  PINTERFACE pif;
  
  /* Check if we already have a (maybe forward) declaration */
  pif=findInterfaceFromName(pParseInfo->pCurInterface->chrName);
  if(pif)
  {
    /* One forward declaration is enough... */
    g_free(pParseInfo->pCurInterface->chrName);
    g_free(pParseInfo->pCurInterface);
  }
  else
  {
    pParseInfo->pCurInterface->chrSourceFileName=g_strdup(pParseInfo->chrCurrentSourceFile);
    pParseInfo->pCurInterface->fIsForwardDeclaration=TRUE;
    /* It's save to register the interface right here even if the struct is almost empty. 
     If anything goes wrong later we will exit anyway. */
    registerInterface();  
  }
}


static void doClassDeclaration(void)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  
  PINTERFACE pif;
  gchar *chrTemp=pParseInfo->pCurInterface->chrName;
  
  /* Check if we already have a (maybe forward) declaration */
  pif=findInterfaceFromName(pParseInfo->pCurInterface->chrName);
  if(pif)
  {
    if(pif->fIsForwardDeclaration)
    {
      /* Remove the forward declaration and insert the real thing afterwards. */
      deRegisterInterface(pif);
    }
    else
    {
      /* It˚s the declaration from the *.h file. Save a pointer to this information. */
      pParseInfo->pClassDefinition=pif;
      deRegisterInterface(pif);
    }
  }
  
  pParseInfo->pCurInterface->chrName=chrTemp;
  pParseInfo->pCurInterface->chrSourceFileName=g_strdup(pParseInfo->chrCurrentSourceFile);
  
  /* It's save to register the interface right here even if the struct is almost empty. 
   If anything goes wrong later we will exit anyway. */
  registerInterface();  
  
  /* The class definition in *.nom files does not contain all the stuff an interface may define. We use the found
   interface to fill the gaps. If we don˚t have an interface something went wrong and we quit. */
  if(!pParseInfo->pClassDefinition)
  {
    g_message("Line %d: Error during class parsing. No class definition found. MAke sure you included the *.ih file.",  g_scanner_cur_line(gScanner));
    cleanupAndExit(0);
  }
  
  pParseInfo->pCurInterface->chrParent=g_strdup(pParseInfo->pClassDefinition->chrParent);
  
  /* Note: We don˚t support subclasses yet. */
  if(matchNext(':'))
  { 
    parseParentClassIdent();
  }  
    parseClassBody();
}

/*
  Parse a class declaration. The current token is the 'class' keyword.

  class:=  CLASSIDENT ';'                            // Forward declaration
         | CLASSIDENT ':' PARENTCLASSIDENT CLASSBODY // Subclass (not used yet!)
         | CLASSIDENT CLASSBODY
 */
void parseClass(GTokenType token)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;
  pParseInfo->pCurInterface=createInterfaceStruct();

  /* Get the class name. */
  pParseInfo->pCurInterface->chrName=parseClassIdent();
  
   /* Check for forward declaration */
  if(matchNext(';'))
    {
      doForwardClassDeclaration();
    }
  else
    {
      /* This is the real thing. */
      doClassDeclaration();
    }/* not forward declaration */
  
#if 0
  g_printf("\n\n");
  /* In printdata.c */  
  printAllInterfaces();
  g_printf("\n\n");
  printInterface(pParseInfo->pCurInterface);
#endif
}

