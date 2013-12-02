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

/* in token.c */
void getNextToken(void);
gboolean matchCur(GTokenType token);
gboolean matchNext(GTokenType token);
guint queryCurTokensKind(void);
void printToken(GTokenType token);
gboolean matchNextKind(guint uiKind);


#define IDL_COMPILER_STRING "IDL compiler" /* Used inside warnings and errors */
#define ID_SCOPE 0

/* Holding an overriden method */
typedef struct
{
  gchar* chrName;    /* Name of this instance variable */
  gchar* chrIntroducingIFace;
}OVERMETHOD, *POVERMETHOD;

#define PARM_DIRECTION_IN     1
#define PARM_DIRECTION_OUT    2
#define PARM_DIRECTION_INOUT  3
/* Holding a method parameter */
typedef struct
{
  guint  uiDirection;   /* in|out|inout */
  gchar* chrType;       /* Type e.g. gulong or GtkWidget */
  guint  uiStar;        /* incremented for each '*'      */
  gchar* chrName;
}METHODPARAM, *PMETHODPARAM;

/* Holding a method */
typedef struct
{
  gchar* chrRetType;    /* Type e.g. gulong or GtkWidget */
  gchar* chrName;
  METHODPARAM mpReturn; /* We don't use all fields of this struct */
  GPtrArray *pParamArray;
}METHOD, *PMETHOD;

/* Info about a symbol */
typedef struct
{
  gchar* chrSymbolName;
  guint  uiSymbolToken;
  guint  uiKind;
}SYMBOL, *PSYMBOL;

/* Struct holding all the info of a defined or declared interface */
typedef struct
{
  gchar* chrName;    /* Name of this interface   */
  gchar* chrParent;  /* Name of parent interface */
  gulong ulMajor;    /* Class version            */
  gulong ulMinor;    /* Class version            */
  gboolean fIsForwardDeclaration;
  PSYMBOL pSymbolIFace; /* Found interfaces are registered as a symbol with the parser.
                           This is a pointer to the registered struct holding the necessary
                           info and may be used to deregister a symbol later.*/
  PSYMBOL pSymbolIFacePtr; /* Same as before but for the pointer on an interface which is
                            registered automatically. */
  gboolean fIsInRootFile;
  gchar* chrMetaClass; /* Pointer to metaclass name or NULL*/
  char*  chrFileStem;  /* Holding output filestem */
  char*  chrSourceFileName; /* The preprocessor includes files for us. This is the info
                               about the file this interface is defined in. */
  GPtrArray *pMethodArray;
  GPtrArray *pOverrideArray;
  GPtrArray *pInstanceVarArray;
}INTERFACE,*PINTERFACE;


typedef struct
{
  PINTERFACE pCurInterface;      /* The interface we are currently parsing. This is a working pointer. */
  guint     uiCurSymbolKind;     /* This may be e.g. KIND_TYPESPEC */
  GTree*    pSymbolTree;         /* Our introduced symbols */
  GPtrArray* pInterfaceArray;    /* The pointer array holding the interfaces we found */
  guint     uiLineCorrection;    /* This is the line number put by the preprocessor into
                                    the source file. It's used to calculate proper line numbers
                                    for errors. */
  char*     chrRootSourceFile;   /* File we are intending to parse. Others may get included. */
  char*     chrCurrentSourceFile;/* The preprocessor includes files for us. This is the info
                                    about their name. */
  char*  chrOutfilePath;         /* This is only the path, no filename */
  FILE*     outFile;             /* Output file handle */
}PARSEINFO, *PPARSEINFO;

/* Symbols defined for our IDL language.
   Keep these enums in sync with the order of the idlSymbols struct! */
enum 
{
  IDL_SYMBOL_INTERFACE=G_TOKEN_LAST+1,
  IDL_SYMBOL_CLSVERSION,
  IDL_SYMBOL_INSTANCEVAR,
  IDL_SYMBOL_OVERRIDE,
  IDL_SYMBOL_OVERRIDE2,
  IDL_SYMBOL_REGINTERFACE,  /* Used for registered interfaces */
  IDL_SYMBOL_CLSNAME,
  IDL_SYMBOL_OLDMETACLASS,
  IDL_SYMBOL_METACLASS,
  IDL_SYMBOL_FILESTEM,      /* Followed by the file name for output */
  IDL_SYMBOL_NATIVE,
  /* Some GLib types */
  IDL_SYMBOL_GULONG,           /* 275 */
  IDL_SYMBOL_GINT,
  IDL_SYMBOL_GPOINTER,
  IDL_SYMBOL_GBOOLEAN,
  IDL_SYMBOL_GCHAR,
  IDL_SYMBOL_VOID,
  /* Legacy support */
  IDL_SYMBOL_BOOLEAN,
  IDL_SYMBOL_STRING,
  IDL_SYMBOL_LONG,
  IDL_SYMBOL_UNSIGNED,
  /* Direction of method parameters */
  IDL_SYMBOL_IN,
  IDL_SYMBOL_OUT,
  IDL_SYMBOL_INOUT,
  /* Preprocessor stuff (not yet supported by the compiler) */
  IDL_SYMBOL_DEFINE,
  IDL_SYMBOL_IFDEF,
  IDL_SYMBOL_ENDIF
};

/* Specifies the kind of a token we read. For example we must
   know if a read in indentifier is a type specification when
   parsing method parameters. */
enum
{
  KIND_UNKNOWN =1,
  KIND_IDENTIFIER,
  KIND_TYPESPEC,    /* That's something like 'gint', 'gulong'... */
  KIND_DIRECTION    /* in, out, inout */
};

PINTERFACE findInterfaceFromName(gchar* chrName);

void parseTypeSpec(PMETHODPARAM pMethodParam);
void parseMethod(void);
void parseInstanceVar(void);
void parseInterface(GTokenType token);
void parseClassVersion(void);
void parseClassVersion(void);
void parseOverrideMethod(void);
void parseOverrideMethodFromIdentifier(void);
void parseHash(void);
void parsePreprocLineInfo(void);
void parseMetaClass(void);
void parseFileStem(void);

/* Emitters */
void emitHFile(GPtrArray* pInterfaceArray);
void emitIHFile(GPtrArray* pInterfaceArray);
void emitCFile(GPtrArray* pInterfaceArray);

/* Emitter support function */
void emitMethodParams(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray);
void emitMethodParamsNoTypes(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray);
void emitReturnType(PPARSEINFO pLocalPI, PINTERFACE pif, PMETHOD pm);
void emitMethodParamsForNOMCompiler(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray);
void emitReturnTypeForNOMCompiler(PPARSEINFO pLocalPI, PINTERFACE pif, PMETHOD pm);

/* In printdata.c */
void printInterface(PINTERFACE pif);
void printAllInterfacec(void);

PINTERFACE getParentInterface(PINTERFACE pif);
PINTERFACE findInterfaceFromMethodName(PINTERFACE pif, gchar* chrName);
PINTERFACE findInterfaceFromName(gchar* chrName);
PMETHOD findMethodInfoFromMethodName(PINTERFACE pif, gchar* chrName);
gboolean queryMessageReturnsAValue(PMETHOD pm);

#ifdef INCL_FILE
FILE* openOutfile(GScanner *gScanner, gchar* chrOutName);
void closeOutfile(FILE* pFile);
#endif
