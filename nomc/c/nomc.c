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
#ifdef __OS2__
# define INCL_DOSPROCESS
# define INCL_DOS
# define INCL_DOSPROFILE
# define INCL_DOSERRORS
# include <os2.h>
#endif /* __OS2__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_IO_H
# include <io.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "nom.h"
#include "nomtk.h"

#define INCL_FILE
#include "parser.h"

static gchar* chrOutputDir="";
static gboolean fOptionEmitH=FALSE;
static gboolean fOptionEmitIH=FALSE;
static gboolean fOptionEmitC=FALSE;

/* Command line options */
static GOptionEntry gOptionEntries[] =
{
  {"directory", 'd', 0, G_OPTION_ARG_FILENAME, &chrOutputDir, "Output directory", NULL},
  {"emit-h", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitH, "Emmit a header file (*.h)", NULL},
  {"emit-ih", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitIH, "Emmit an include header (*.ih)", NULL},
  {"emit-c", 0, 0, G_OPTION_ARG_NONE, &fOptionEmitC, "Emmit an implementation template (*.c)", NULL},
  {NULL}
};



/* Symbols defined for our IDL language.
   Keep this in synch with the defined enums in parser.h! */
SYMBOL idlSymbols[]={
  {"interface", IDL_SYMBOL_INTERFACE, KIND_UNKNOWN},
  {"class", NOMC_SYMBOL_CLASS, KIND_UNKNOWN}, /* 71 */
  {"public", NOMC_SYMBOL_PUBLIC, KIND_UNKNOWN},
  {"NOMINSTANCEVAR", IDL_SYMBOL_INSTANCEVAR, KIND_UNKNOWN},
  {"override", IDL_SYMBOL_OVERRIDE2, KIND_UNKNOWN},
  {"NOMREGISTEREDIFACE", IDL_SYMBOL_REGINTERFACE, KIND_TYPESPEC},
  {"NOMCLASSNAME", IDL_SYMBOL_CLSNAME, KIND_UNKNOWN},
  {"NOMMETACLASS", IDL_SYMBOL_OLDMETACLASS, KIND_UNKNOWN},
  {"metaclass", IDL_SYMBOL_METACLASS, KIND_UNKNOWN},
  {"filestem", IDL_SYMBOL_FILESTEM, KIND_UNKNOWN},
  {"native", IDL_SYMBOL_NATIVE, KIND_UNKNOWN},
  {"gulong", IDL_SYMBOL_GULONG, KIND_TYPESPEC},
  {"gint", IDL_SYMBOL_GINT, KIND_TYPESPEC},
  {"gpointer", IDL_SYMBOL_GPOINTER, KIND_TYPESPEC},
  {"gboolean", IDL_SYMBOL_GBOOLEAN, KIND_TYPESPEC},
  {"gchar", IDL_SYMBOL_GCHAR, KIND_TYPESPEC},
  {"void", IDL_SYMBOL_VOID, KIND_TYPESPEC},

  {"boolean", IDL_SYMBOL_BOOLEAN, KIND_TYPESPEC},
  {"string", IDL_SYMBOL_STRING, KIND_TYPESPEC},
  {"long", IDL_SYMBOL_LONG, KIND_TYPESPEC},
  {"unsigned", IDL_SYMBOL_LONG, KIND_TYPESPEC},

  {"in", IDL_SYMBOL_IN, KIND_DIRECTION},
  {"out", IDL_SYMBOL_OUT, KIND_DIRECTION},
  {"inout", IDL_SYMBOL_INOUT, KIND_DIRECTION},
  {"define", IDL_SYMBOL_DEFINE, KIND_UNKNOWN},
  {"ifdef", IDL_SYMBOL_IFDEF, KIND_UNKNOWN},
  {"endif", IDL_SYMBOL_ENDIF, KIND_UNKNOWN},
  {NULL, 0, KIND_UNKNOWN}
},*pSymbols=idlSymbols;

GScanner *gScanner;

/* Holding the current state of parsing and pointers to necessary lists.
   Referenced by gScanner-user_data. */
PARSEINFO parseInfo={0};
//PPARSEINFO pParseInfo=&parseInfo; /* This pointer will go away, don't use */


/**
   Helper function which returns a copy of the typespec string of the current token.
   That is e.g. 'gint' or 'gpointer'. Note that this function is only called when the
   current token is indeed a type specification in the IDL file.
 */
gchar* getTypeSpecStringFromCurToken(void)
{
  GTokenValue value;

  value=gScanner->value;

  switch(gScanner->token)
    {
    case G_TOKEN_IDENTIFIER:
      return g_strdup(value.v_identifier);
      break;
    case G_TOKEN_SYMBOL:
      {
        /* It's one of our symbols. */
        PSYMBOL pCurSymbol;

        pCurSymbol=value.v_symbol;
        return g_strdup(pCurSymbol->chrSymbolName);
        break;
      }
    default:
      g_scanner_unexp_token(gScanner,
                            G_TOKEN_SYMBOL,
                            NULL,
                            NULL,
                            NULL,
                            "",
                            TRUE); /* is_error */

      break;
    }
  return "unknown";
}


/**
   This is the root parse function. Here starts the fun. When a token is found in the
   input stream which matches one of the known token types the respective parsing function
   is called for further processing. In case of an error the parsing function in question
   prints an error which describes the problem and exits the application.

   This function scans the input until EOF is hit.
 */
void parseIt(void)
{

  while(g_scanner_peek_next_token(gScanner) != G_TOKEN_EOF) {
    GTokenType token;
    GTokenValue value;

    parseInfo.fPrintToken=FALSE;
    
   // g_printf(":: %d %d\n", gScanner->position, parseInfo.uiCurPos);
    getNextTokenFromStream();
   // g_printf("::: %d %d\n", gScanner->position, parseInfo.uiCurPos);

    printToFile(gScanner->token); 
    
    token=gScanner->token;
    value=gScanner->value;

    //printToFile(token);
    
    switch(token)
    {
      case '#':
        parseInfo.fPrintToken=TRUE;
        printToFile(token);
        parseHash(); /* e.g. for correct line numbers */
        break;
      case G_TOKEN_SYMBOL:
        {
          PSYMBOL pCurSymbol=value.v_symbol;
          switch(pCurSymbol->uiSymbolToken)
          {
            case NOMC_SYMBOL_CLASS:
              parseClass(token);
              break;
            case IDL_SYMBOL_INTERFACE:
              parseInterface(token);
              break;                
#if 0
            case IDL_SYMBOL_NATIVE:
              parseNative();
              break;
            case IDL_SYMBOL_CLSNAME:
              parseClassName();
              break;
            case IDL_SYMBOL_OLDMETACLASS:
              parseOldMetaClass();
              break;
#endif
            default:
              break;
            }
          break;
        }
#if 0
      case G_TOKEN_IDENTIFIER:
        g_message("Token: %d (G_TOKEN_IDENTIFIER)\t\t%s", token, value.v_identifier);
        break;
      case G_TOKEN_STRING:
        g_message("Token: %d (G_TOKEN_STRING)\t\t\t%s", token, value.v_string);
        break;
      case G_TOKEN_LEFT_PAREN:
        g_message("Token: %d (G_TOKEN_LEFT_PAREN)\t\t(", token);
        break;
      case G_TOKEN_RIGHT_PAREN:
        g_message("Token: %d (G_TOKEN_RIGHT_PAREN)\t\t)", token);
        break;
      case ':':
        g_message("Token: %d (colon)\t\t:", token);
        break;
      case ';':
        g_message("Token: %d (semicolon)\t\t\t;", token);
        break;
      case '/':
        g_message("Token: %d (slash)\t\t\t/ %s", token, value.v_comment);
        break;
      case G_TOKEN_COMMA:
        g_message("Token: %d (G_TOKEN_COMMA)\t\t\t,", token);
        break;
      case G_TOKEN_INT:
        g_message("Token: %d (G_TOKEN_INT)\t\t\t%ld", token, value.v_int);
        break;
      case IDL_SYMBOL_DEFINE:
        g_message("Token: %d (IDL_SYMBOL_DEFINE)\t\t\t", token);
        break;
      case IDL_SYMBOL_IFDEF:
        g_message("Token: %d (IDL_SYMBOL_IFDEF)\t\t\t", token);
        break;
      case IDL_SYMBOL_ENDIF:
        g_message("Token: %d (IDL_SYMBOL_ENDIF)\t\t\t", token);
        break;
#endif
      default:
        printToken(token);
        break;
      }
    //    printToken(token);

  }
}

/**
   Support function to show help for the IDL compiler. gContext must be valid.
*/
static void outputCompilerHelp(GOptionContext *gContext, gchar* chrExeName)
{
  GError *gError = NULL;
  int argc2=2;
  char *helpCmd[]={"","--help"};
  char** argv2=helpCmd;
  helpCmd[0]=chrExeName;

  g_printf("The output filename is specified in the IDL file using the 'filestem' keyword.\n\
-If no directory is specified the output name is built from the current directory\n\
 path and the given filename.\n\
-If the directory is a relative path the output name is built from the current\n\
 directory path, the given directory name (or path) and the filename.\n\
-If the directory is a full path the output name is built from the directory\n\
 path and the given filename.\n\n\
Note that an emitter specific extension will always be appended to the output\n\
filename\n\n");

  /* This prints the standard option help to screen. */
  g_option_context_parse (gContext, &argc2, &argv2, &gError);
}

/*
  Compare function for the tree holding our private symbols.
 */
static gint funcSymbolCompare(gconstpointer a, gconstpointer b)
{
  if(a==b)
    return 0;

  if(a<b)
    return -1;

  return 1;
};

/**
   Message output handler for the scanner. The default handler isn't used because the preprocessor
   mangles all include files together and thus the line numbers are not as expected by the user.
   This function prints the error messages with corrected linenumbers and the source file name
   in which to find the problem.
 */
void funcMsgHandler(GScanner *gScanner, gchar *message, gboolean error)
{
  g_printf("%s:%d: error: %s (%d %d)\n",
           parseInfo.chrCurrentSourceFile ? parseInfo.chrCurrentSourceFile : "<null>", /* glib doesn't check for NULL like printf. */
           g_scanner_cur_line(gScanner)-parseInfo.uiLineCorrection, message,
           g_scanner_cur_line(gScanner), parseInfo.uiLineCorrection);
}

/**
   Main entry point for the compiler.
 */
int main(int argc, char **argv)
{
  int a;
  int fd;
  /* Vars for filename building */
  char* chrOutputFileName="";

  GError *gError = NULL;
  GOptionContext* gContext;

  /* Parse command line options */
  gContext = g_option_context_new ("file");
  g_option_context_add_main_entries (gContext, gOptionEntries, NULL);

  if(!g_option_context_parse (gContext, &argc, &argv, &gError))
    {
      outputCompilerHelp(gContext, argv[0]);
    }
#if 0
  /* Correct emitter options? Exactly one emitter must be specified. */
  a=0;
  if(fOptionEmitH)
    a++;
  if(fOptionEmitIH)
    a++;
  if(fOptionEmitC)
    a++;

  if(!a){
    g_printf("An emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }
  if(a>1){
    g_printf("Only one emitter must be specified.\n\n");
    outputCompilerHelp(gContext, argv[0]);
  }
#endif
  g_option_context_free(gContext);


  if(argc<2)
    {
      g_printf("No input file name given.\n\nUse %s --help for options.\n\n", argv[0]);
      return 1;
    }

#if 0
  for(a=0; a<argc; a++)
    {
      g_message("arg %d: %s", a, argv[a]);
    }
#endif

  /*** Create output path name ****/
  if(g_path_is_absolute(chrOutputDir))
    chrOutputFileName=chrOutputDir;
  else
    {
      /* Yes this is a memory leak but I don't care */
      chrOutputFileName=g_build_filename(g_get_current_dir(), chrOutputDir, NULL);
    }

  g_message("Output path: %s", chrOutputFileName);
  parseInfo.chrOutfilePath=chrOutputFileName;

  /* Open input */
  if(!strcmp(argv[1], "-"))
    fd=0; /* Read from stdin */
  else
    fd=open(argv[1], O_RDONLY);

  if(-1==fd)
    {
      g_message("Can't open input file %s", argv[1]);
      exit(1);
    }

  g_printf("\n");

  gScanner=g_scanner_new(NULL);
  gScanner->user_data=(gpointer)&parseInfo;

  gScanner->msg_handler=funcMsgHandler;
  parseInfo.pInterfaceArray=g_ptr_array_new();

  g_scanner_input_file(gScanner, fd);
  gScanner->config->case_sensitive=TRUE;
  /* No single line comments */
  gScanner->config->skip_comment_single=FALSE;
  gScanner->config->cpair_comment_single="";
  gScanner->config->scan_float=FALSE;
  /* This string is used in error messages of the parser */
  gScanner->input_name=IDL_COMPILER_STRING;

  g_scanner_set_scope(gScanner, ID_SCOPE);
  /* Load our own symbols into the scanner. We use the default scope for now. */
  parseInfo.pSymbolTree=g_tree_new((GCompareFunc) funcSymbolCompare);
  while(pSymbols->chrSymbolName)
    {
#ifndef _MSC_VER
# warning !!! Create a copy here so it is the same as with new symbols added later.
#endif
      g_scanner_scope_add_symbol(gScanner, ID_SCOPE, pSymbols->chrSymbolName,
                                 pSymbols);
      g_tree_insert(parseInfo.pSymbolTree, pSymbols, pSymbols->chrSymbolName);
      pSymbols++;
    }

  if((parseInfo.outFile=openOutfile(gScanner, "out.txt"))!=NULL)
    parseIt();

#if 0
  /* Write the output file */
  if(fOptionEmitH)
    emitHFile(parseInfo.pInterfaceArray);
  else if(fOptionEmitIH)
    emitIHFile(parseInfo.pInterfaceArray);
  else if(fOptionEmitC)
    emitCFile(parseInfo.pInterfaceArray);
#endif
  g_scanner_destroy(gScanner);
  if(0!=fd)
    close(fd); /* We read from stdin */
  if(parseInfo.outFile)
    fclose(parseInfo.outFile);
  return 0;
}

#if 0
  /* We are a folder somwhere in the chain */
  nomRetval=WPFileSystem_wpQueryFileName((WPFileSystem*)wpParent, bFullPath, NULLHANDLE);

  nomRetval=wpParent.wpQueryFileName(bFullPath, NULLHANDLE);

  nomPath=NOMPathNew();
  nomPath= (PNOMPath) NOMPath_assignCString(nomPath, _pszFullPath, ev);

  return (PNOMPath)NOMPath_appendPath(nomRetval, nomPath, NULLHANDLE);

#endif









