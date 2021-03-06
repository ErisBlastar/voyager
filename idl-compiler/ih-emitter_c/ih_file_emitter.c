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

#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 

#define INCL_FILE
#include "parser.h"

extern GScanner *gScanner;

static void emitIHFileHeader(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/*\n * This file was generated by the NOM IDL compiler for Voyager - DO NOT EDIT!\n");
  fprintf(fh, " *\n *\n * And remember, phase 3 is near...\n */\n");
  fprintf(fh, "/*\n * %s\n */\n", pif->chrSourceFileName);

  /* Protective #ifndef for whole file */
  fprintf(fh, "#ifndef %s_IH\n#define %s_IH\n\n", pif->chrName, pif->chrName);

  /* The *.h of this class contains some declarations we need */
  fprintf(fh, "#include \"%s.h\"\n\n", pif->chrFileStem);

  fprintf(fh, "#ifndef NOMCOMPILER\n");

}


static void emitInstanceVariables(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  int a;
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray=pif->pInstanceVarArray;;

  fprintf(fh, "/*\n * Instance variables for %s\n */\n", pif->chrName);
  fprintf(fh, "typedef struct {\n");

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHODPARAM piv=(PMETHODPARAM)g_ptr_array_index(pArray, a);

            fprintf(fh, "    %s", piv->chrType);
            for(b=0;b<piv->uiStar;b++)
              fprintf(fh, "*");
            fprintf(fh, "  %s;\n", piv->chrName);
    }
#ifdef _MSC_VER
  if(!a)
    fprintf(fh, "  int iDummy;\n"); /* HACK ALERT! */
#endif
  fprintf(fh, "} %sData;\n\n", pif->chrName);
}

static void emitGetDataMacros(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray=pif->pInstanceVarArray;
  int a;

  fprintf(fh, "/*\n * Get data macros for %s\n */\n", pif->chrName);
  fprintf(fh, "typedef %sData* NOMLINK nomTP_%s_DataThunk(void*);\n",
          pif->chrName , pif->chrName);
  fprintf(fh, "typedef nomTP_%s_DataThunk *nomTD_%s_DataThunk;\n",
          pif->chrName , pif->chrName);

  fprintf(fh, "#define %sGetData(nomSelf) \\\n", pif->chrName);
  fprintf(fh, "    (((nomTD_%s_DataThunk)(%sCClassData.instanceDataToken))(nomSelf))\n",
          pif->chrName , pif->chrName);

  for(a=0;a<pArray->len;a++)
    {
      PMETHODPARAM piv=(PMETHODPARAM)g_ptr_array_index(pArray, a);
      
      fprintf(fh, "#define  _%s (nomThis->%s)\n", piv->chrName, piv->chrName);
    }
  fprintf(fh, "\n");
}


static void emitIHClassDataStructs(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n\n", pif->chrName);

  fprintf(fh, "/*** Class data structures ***/\n");
  fprintf(fh, "NOMDLLEXPORT struct %sClassDataStructure %sClassData = {0};\n", pif->chrName, pif->chrName );
  fprintf(fh, "NOMDLLEXPORT struct %sCClassDataStructure %sCClassData = {0};\n\n",
          pif->chrName, pif->chrName);
}

#if 0
/* Function to check if an object is valid before calling a method on it */
#ifdef NOM_NO_PARAM_CHECK /* Disabled by now because not working */
NOMEXTERN gboolean NOMLINK objectCheckFunc_WPRootFolder(WPRootFolder *nomSelf, gchar* chrMethodName)
{
if(!nomIsObj(nomSelf) || !_nomIsANoClsCheck(nomSelf , WPRootFolderClassData.classObject, NULL))
  {
  nomPrintObjectPointerError(nomSelf, "WPRootFolder", chrMethodName);
  g_message("Note that NULL is returned for the call (if the method returns a value). This may not be correct. Use the NOMPARMCHECK() macro to specify default return values for methods.");
  return FALSE;
  }
  return TRUE;
}
#endif
#endif


/*
  \param pArray Pointer to the list of parameters.
 */
static void emitMethodParamStrings(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);

      switch(pm->uiDirection)
        {
        case PARM_DIRECTION_IN:
          fprintf(fh, "  \"%s", pm->chrType);
          break;
        case PARM_DIRECTION_OUT:
          fprintf(fh, "  \"%s*", pm->chrType);
          break;
        case PARM_DIRECTION_INOUT:

          break;
        default:
          fprintf(fh, "  \"%s*", pm->chrType);
          break;
        }
      for(b=0;b<pm->uiStar;b++)
        fprintf(fh, "*");
      fprintf(fh, "\",\n");      
    }
#ifdef _MSC_VER
  if(!a)
    fprintf(fh, "  NULL\n");
#endif
}

static void emitNewMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray;
  int a;

  pArray=pif->pMethodArray;

  for(a=0;a<pArray->len;a++)
    {
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

      fprintf(fh, "/*\n * New method: %s\n */\n", pm->chrName);
      fprintf(fh, "#if !defined(_decl_impl_%s_%s_)\n", pif->chrName, pm->chrName);
      fprintf(fh, "#define _decl_impl_%s_%s_ 1\n", pif->chrName, pm->chrName);
      fprintf(fh, "NOMDLLEXPORT NOM_Scope ");
      emitReturnType(pLocalPI, pif, pm);
      fprintf(fh, " NOMLINK impl_%s_%s(%s *nomSelf,\n", pif->chrName, pm->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " CORBA_Environment *ev);\n");
      fprintf(fh, "static char* nomIdString_%s_%s = nomMNDef_%s_%s;\n",
              pif->chrName, pm->chrName, pif->chrName, pm->chrName);
      fprintf(fh, "static char* nomFullIdString_%s_%s = nomMNFullDef_%s_%s;\n",
              pif->chrName, pm->chrName, pif->chrName, pm->chrName);

      fprintf(fh, "static nomParmInfo nomParm_%s_%s = {\n", pif->chrName, pm->chrName);
      fprintf(fh, "  %d,  /* Number of parameters */\n", pm->pParamArray->len);

      fprintf(fh, "  \"");
      emitReturnType(pLocalPI, pif, pm);
      fprintf(fh, "\", /* Return type */\n  {\n");
      emitMethodParamStrings(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "  }\n};\n");

      fprintf(fh, "#endif /* _decl_impl_%s_%s_ */\n\n", pif->chrName, pm->chrName);
    }
  fprintf(fh, "\n");
}

static void emitOverridenMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray;
  int a;

  pArray=pif->pOverrideArray;

  for(a=0;a<pArray->len;a++)
    {
      POVERMETHOD pom=(POVERMETHOD)g_ptr_array_index(pArray, a);
      /* Method information */
      PMETHOD pm=findMethodInfoFromMethodName(pif, pom->chrName);
      /* Pointer to interface which introduced the method */
      PINTERFACE pifIntroduced=findInterfaceFromMethodName(pif, pom->chrName);

      fprintf(fh, "/*\n * Overriden method: %s \n */\n",  pom->chrName);
      fprintf(fh, "#ifndef _decl_impl_%s_%s_\n", pif->chrName, pom->chrName);
      fprintf(fh, "#define _decl_impl_%s_%s_\n", pif->chrName,  pom->chrName);

      if(!pm || !pifIntroduced)
        {
          g_message("Can't get information about method \"%s\" from parent classes while overriding.", pom->chrName);
          exit(1);
        }

      fprintf(fh, "NOMDLLEXPORT NOM_Scope ");
      emitReturnType(pLocalPI, pif, pm);
      fprintf(fh, " NOMLINK impl_%s_%s(%s* nomSelf,\n", pif->chrName, pom->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " CORBA_Environment *ev);\n");
      fprintf(fh, "static char* nomIdString_%s_%s = \"%s:%s\";\n",
              pif->chrName, pom->chrName, pifIntroduced->chrName, pom->chrName);
      fprintf(fh, "static nomMethodProc* %s_%s_parent_resolved;\n",
              pif->chrName, pom->chrName);
      fprintf(fh, "#define %s_%s_parent(nomSelf,",
              pif->chrName, pom->chrName);
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev) \\\n");
      fprintf(fh, "        (((nomTD_%s_%s) \\\n", pifIntroduced->chrName, pom->chrName);

      fprintf(fh, "        %s_%s_parent_resolved)((%s*)nomSelf,",
              pif->chrName, pom->chrName, pifIntroduced->chrName);
      emitMethodParamsNoTypes(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, " ev))\n");
      fprintf(fh, "#endif /* _decl_impl_%s_%s_ */\n\n", pif->chrName, pom->chrName);
    }
};

static void emitOverridenMethodTable(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray;
  int a;

  fprintf(fh, "/* Table of the overriden methods by this class */\n");
  fprintf(fh, "static nomOverridenMethodDesc nomOverridenMethods%s[] = {\n", pif->chrName);

  pArray=pif->pOverrideArray;

  for(a=0;a<pArray->len;a++)
    {
      POVERMETHOD pom=(POVERMETHOD)g_ptr_array_index(pArray, a);
      /* Method information */
      PMETHOD pm=findMethodInfoFromMethodName(pif, pom->chrName);

      if(!pm)
        {
          g_message("Can't get information about method \"%s\" from parent classes while overriding.", pom->chrName);
          exit(1);
        }

      fprintf(fh, "  {\n");
      fprintf(fh, "    &nomIdString_%s_%s,\n", pif->chrName,  pom->chrName);
      fprintf(fh, "    (nomMethodProc*) impl_%s_%s,\n", pif->chrName,  pom->chrName);
      fprintf(fh, "    &%s_%s_parent_resolved\n", pif->chrName,  pom->chrName);
      fprintf(fh, "  },\n");
    }
#ifdef _MSC_VER
  if(!a)
    fprintf(fh, "  { NULL, NULL, NULL }\n");
#endif
  fprintf(fh, "};\n\n");
}


static void emitStaticMethodTable(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray;
  int a;

  pArray=pif->pMethodArray;

  fprintf(fh, "/* Description of the static methods introduced by this class */\n");
  fprintf(fh, "static nomStaticMethodDesc nomStaticMethods%s[] = {\n", pif->chrName);

  for(a=0;a<pArray->len;a++)
    {
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

      fprintf(fh, "{\n");
      fprintf(fh, "  &%sClassData.%s,\n", pif->chrName,  pm->chrName);
      fprintf(fh, "  (nomID)&nomIdString_%s_%s,\n", pif->chrName,  pm->chrName);
      fprintf(fh, "  &nomFullIdString_%s_%s,  /* char *chrMethodDescriptor */\n", pif->chrName,  pm->chrName);
      fprintf(fh, "  (nomMethodProc*)  impl_%s_%s,\n", pif->chrName,  pm->chrName);
      fprintf(fh, "  &nomParm_%s_%s\n", pif->chrName,  pm->chrName);
      fprintf(fh, "},\n");
    }
#ifdef _MSC_VER
  if(!a)
    fprintf(fh, "  { NULL, NULL, NULL, NULL, NULL }\n");
#endif
  fprintf(fh, "};\n\n");
}


static void emitMetaClass(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  if(pif->chrMetaClass)
    {
      fprintf(fh, "/* The meta class for this class */\n");
      fprintf(fh, "static char * nomIdStringMetaClass_%s = \"%s\";\n\n", pif->chrName, pif->chrMetaClass);
    }
}

static void emitParentClasses(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;
  GPtrArray *pArray=g_ptr_array_new();
  PINTERFACE pifParent=pif;
  int a;

  if(pif->chrParent)
    {
      fprintf(fh, "/* Array of parent names (chain of parents) */\n");
      fprintf(fh, "static char* nomParentClassNames%s[]=\n{\n", pif->chrName);
      /* Emit the parents. We have to output them sorted beginning from the
         leftmost parent. */
      while(pifParent->chrParent && (pifParent=findInterfaceFromName(pifParent->chrParent))!=NULL)
        {
          g_ptr_array_add(pArray, (gpointer) pifParent);
        }
      
      for(a=pArray->len-1; a>=0; a--)
        {
          pifParent=(PINTERFACE)g_ptr_array_index(pArray, a);
          fprintf(fh, "  \"%s\",\n", pifParent->chrName);
        }
      g_ptr_array_free(pArray, TRUE);
      fprintf(fh, "};\n\n");
      
      fprintf(fh, "static char * nomIdString_Parent_%s = \"%s\";\n\n", pif->chrParent, pif->chrParent);
      
      fprintf(fh, "/* Array of parent IDs (direct parents, for now NOM only support single inheritance) */\n");
      fprintf(fh, "static nomID nomParentClasses%s[]=\n", pif->chrName);
      fprintf(fh, "{\n");
      fprintf(fh, "  &nomIdString_Parent_%s,\n", pif->chrParent);
      fprintf(fh, "};\n\n");
    }
}

static gulong getNumberOfParentsInChain(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  gulong ulRet=0;
  PINTERFACE pifParent=pif;

  while(pifParent->chrParent && (pifParent=findInterfaceFromName(pifParent->chrParent))!=NULL)
    ulRet++;

  return ulRet;
}

static gulong calculateInstanceDataSize(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  int a;
  gulong ulRet=0;
  GPtrArray *pArray=pif->pInstanceVarArray;
  
  for(a=0;a<pArray->len;a++)
    {
      PMETHODPARAM piv=(PMETHODPARAM)g_ptr_array_index(pArray, a);

      //g_printf("\t\tType:\t\t%s", piv->chrType);

      if(piv->uiStar)
        ulRet+=sizeof(gpointer);

      else if(!strcmp(piv->chrType, "gboolean"))
        ulRet+=sizeof(gboolean);
      else if(!strcmp(piv->chrType, "gpointer"))
        ulRet+=sizeof(gpointer);
      else if(!strcmp(piv->chrType, "gchar"))
        ulRet+=sizeof(gchar);
      else if(!strcmp(piv->chrType, "guchar"))
        ulRet+=sizeof(guchar);
      else if(!strcmp(piv->chrType, "gint"))
        ulRet+=sizeof(gint);
      else if(!strcmp(piv->chrType, "guint"))
        ulRet+=sizeof(guint);
      else if(!strcmp(piv->chrType, "gshort"))
        ulRet+=sizeof(gshort);
      else if(!strcmp(piv->chrType, "gushort"))
        ulRet+=sizeof(gushort);
      else if(!strcmp(piv->chrType, "glong"))
        ulRet+=sizeof(glong);
      else if(!strcmp(piv->chrType, "gulong"))
        ulRet+=sizeof(gulong);
      else if(!strcmp(piv->chrType, "gint8"))
        ulRet+=sizeof(gint8);
      else if(!strcmp(piv->chrType, "gfloat"))
        ulRet+=sizeof(gfloat);
      else if(!strcmp(piv->chrType, "gdouble"))
        ulRet+=sizeof(gdouble);
      else if(!strcmp(piv->chrType, "string"))
        ulRet+=sizeof(gpointer);
      else
        /* Check if it's an interface */
        if(findInterfaceFromName(piv->chrType))
          ulRet+=sizeof(gpointer);
        else{
          g_message("Warning: Unknown type \"%s\". Assuming sizeof(gpointer) for it (%s, %s).",
                    piv->chrType, __FILE__, __FUNCTION__);
          ulRet+=sizeof(gpointer);
        }
    }

  return ulRet;
}

static void emitStaticClassInfo(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/* Identify this class */\n");
  fprintf(fh, "static char * nomIdString_%s = \"%s\";\n\n", pif->chrName, pif->chrName);

  fprintf(fh, "static nomStaticClassInfo %sSCI = {\n", pif->chrName);
  fprintf(fh, "  0,               /* Version */\n");
  fprintf(fh, "  %d, /* Number of static methods introduced by this class */\n", pif->pMethodArray->len);
  fprintf(fh, "  %d,               /* Overrides */\n", pif->pOverrideArray->len);
  fprintf(fh, "  %s_MajorVersion,\n", pif->chrName);
  fprintf(fh, "  %s_MinorVersion,\n", pif->chrName);
  fprintf(fh, "  %ld,               /* Instance data size */\n",
          calculateInstanceDataSize(pLocalPI, pif));
  fprintf(fh, "  1,               /* Number of parents (multiple inheritance) */\n");
  fprintf(fh, "  &nomIdString_%s,\n", pif->chrName);
  if(pif->chrMetaClass)
    fprintf(fh, "  &nomIdStringMetaClass_%s,    /* Explicit meta id*/\n", pif->chrName);
  else
    fprintf(fh, "  NULL,    /* Explicit meta id*/\n");
  fprintf(fh, "  (nomClassDataStructure*)&%sClassData,\n", pif->chrName);
  fprintf(fh, "  (nomCClassDataStructure*)&%sCClassData,\n", pif->chrName);
  fprintf(fh, "  (nomStaticMethodDesc*)&nomStaticMethods%s,\n", pif->chrName);
  if(pif->chrParent)
    {
    fprintf(fh, "  nomParentClasses%s,\n", pif->chrName);
    fprintf(fh, "  nomParentClassNames%s, /* Name of all the parent classes in chain */\n", pif->chrName);
    }
  else
    {
      fprintf(fh, "    (void*)NULL,\n");
      fprintf(fh, "    (void*)NULL,\n");
    }

  fprintf(fh, "  %ld,                /* Number of parents in the chain of classes */\n",
          getNumberOfParentsInChain( pLocalPI,  pif));
  fprintf(fh, "  nomOverridenMethods%s,\n", pif->chrName);
  fprintf(fh, "};\n\n");
};



static void emitClassCreationFunc(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "/*** Class creation function ***/\n\n");

  if(pif->chrMetaClass)
    {
      /* Try to get the name of the metaclass include file. */
      PINTERFACE pifMeta=findInterfaceFromName(pif->chrMetaClass);
      if(pifMeta)
        fprintf(fh, "#include \"%s.h\\n", pifMeta->chrFileStem);
      else
        {
          fprintf(fh, "#ifndef %s\n", pif->chrMetaClass);
          fprintf(fh, "#error Your have to include the header file defining class \"%s\" first.\n", pif->chrMetaClass);
          fprintf(fh, "#endif\n\n");
        }
    }
  fprintf(fh, "#include \"nomgc.h\"\n");
  fprintf(fh, "#ifdef NOM_%s_IMPLEMENTATION_FILE\n", pif->chrName);
  fprintf(fh, "NOMDLLEXPORT\n");
  fprintf(fh, "#else\n");
  fprintf(fh, "NOMDLLIMPORT\n");
  fprintf(fh, "#endif\n");
  fprintf(fh, "NOMClass* NOMLINK %sNewClass(gulong ulMajor, gulong ulMinor)\n", pif->chrName);
  fprintf(fh, "{\n");
  fprintf(fh, "  NOMClass* result;\n");

  fprintf(fh, "#ifdef __OS2__\n");
  fprintf(fh, "  gulong ulObj, ulOffset;\n");
  fprintf(fh, "  gchar thePath[CCHMAXPATH];\n");
  fprintf(fh, "  HMODULE hModule;\n");

  fprintf(fh, "  g_assert(DosQueryModFromEIP( &hModule, &ulObj, CCHMAXPATH, ");
  fprintf(fh, "thePath, &ulOffset, (ULONG)%sNewClass)==0);\n", pif->chrName);
  fprintf(fh, "  g_strlcat(thePath, \".DLL\", sizeof(thePath));\n");
  fprintf(fh, "  if(!nomQueryUsingNameIsDLLRegistered(thePath))\n");
  fprintf(fh, "    {\n");
  fprintf(fh, "    HREGDLL hReg=nomBeginRegisterDLLWithGC();\n");
  fprintf(fh, "    g_assert(nomRegisterDLLByName(hReg, thePath));\n");
  fprintf(fh, "    nomEndRegisterDLLWithGC(hReg);\n");
  fprintf(fh, "    }\n");
  fprintf(fh, "#elif defined(_WIN32)\n");
  fprintf(fh, " /* FIXME: check this up with the GC. */\n");
  fprintf(fh, "#elif defined(__APPLE__)\n");
  fprintf(fh, "# warning FIXME: Check out GC/dylib.\n");
  fprintf(fh, "#elif defined(__linux__)\n");
  fprintf(fh, "# warning FIXME: Check out GC/so on linux.\n");
  fprintf(fh, "#elif defined(__FreeBSD__)\n");
  fprintf(fh, "# warning FIXME: Check out GC/so on FreeBSD\n");
  fprintf(fh, "#else\n");
  fprintf(fh, "#error DLL must be registered with the garbage collector!\n");
  fprintf(fh, "#endif\n\n");

  if(pif->chrMetaClass)
    {
      fprintf(fh, "  /* Create the metaclass */\n");
      fprintf(fh, "  %sNewClass(%s_MajorVersion, %s_MinorVersion);\n",
              pif->chrMetaClass, pif->chrMetaClass, pif->chrMetaClass);
    }
  if(pif->chrParent)
    {
      fprintf(fh, "  /* Create the parent class */\n");
      fprintf(fh, "  %sNewClass(%s_MajorVersion, %s_MinorVersion);\n",
              pif->chrParent, pif->chrParent, pif->chrParent);
    }
  fprintf(fh, "  result = nomBuildClass(1, &%sSCI, ulMajor, ulMinor);\n\n", pif->chrName);
  fprintf(fh, "  return result;\n");
  fprintf(fh, "};\n\n");
}

static void emitIHFileFooter(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  FILE* fh=pLocalPI->outFile;

  fprintf(fh, "\n#endif /* NOM_%s_IMPLEMENTATION_FILE */\n", pif->chrName);
  
  fprintf(fh, "#endif /* NOMCOMPILER */\n");

  fprintf(fh, "#endif/* %s_IH */\n", pif->chrName);
}

void emitIHFile(GPtrArray* pInterfaceArray)
{
  int a;
  PPARSEINFO pLocalPI=(PPARSEINFO)gScanner->user_data;

  for(a=0;a<pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pLocalPI->pInterfaceArray, a); 
      if(!strcmp(pif->chrSourceFileName, pLocalPI->chrRootSourceFile))
        {
          /* Only interfaces which are fully defined. No forwarder */
          if(!pif->fIsForwardDeclaration)
            {
              gchar*  chrTemp;

              //printInterface(pif);
              chrTemp=g_strconcat(pif->chrFileStem, ".ih", NULL);
              if((pLocalPI->outFile=openOutfile(gScanner, chrTemp))!=NULL)
                {
                  emitIHFileHeader(pLocalPI, pif);
                  emitInstanceVariables(pLocalPI, pif);
                  emitGetDataMacros(pLocalPI, pif);
                  emitIHClassDataStructs(pLocalPI, pif);
                  emitNewMethods(pLocalPI, pif);
                  emitOverridenMethods(pLocalPI, pif);
                  emitOverridenMethodTable(pLocalPI, pif);
                  emitStaticMethodTable(pLocalPI, pif);
                  emitMetaClass(pLocalPI, pif);
                  emitParentClasses(pLocalPI, pif);
                  emitStaticClassInfo(pLocalPI, pif);
                  emitClassCreationFunc(pLocalPI, pif);
                  
                  emitIHFileFooter(pLocalPI, pif);
                }
              g_free(chrTemp);
            }/* fIsForwardDeclaration */
        }
    }
}


