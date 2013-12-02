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
#endif 
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 

#define INCL_FILE
#include "parser.h"

extern GScanner *gScanner;

#if 0
static void emitNewMethods(PPARSEINFO pLocalPI, PINTERFACE pif)
{
  int a;
  GPtrArray *pArray;
  FILE* fh=pLocalPI->outFile;

  pArray=pif->pMethodArray;

  for(a=0;a<pArray->len;a++)
    {
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

      /* Do return type */
      fprintf(fh, "NOMDLLEXPORT NOM_Scope ");
      emitReturnType(pLocalPI, pif, pm);

      fprintf(fh, " NOMLINK impl_%s_%s(%s* nomSelf,\n", pif->chrName,  pm->chrName, pif->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "    CORBA_Environment *ev)\n");
      fprintf(fh, "{\n");
      fprintf(fh, "  /* %sData* nomThis = %sGetData(nomSelf); */\n", pif->chrName, pif->chrName);
      if(queryMessageReturnsAValue(pm)){
        fprintf(fh, "  ");
        emitReturnType(pLocalPI, pif, pm);
        fprintf(fh, " nomRetval;\n\n");
        fprintf(fh, "#warning In impl_%s_%s(): return value must be customized!\n",  pif->chrName,  pm->chrName);
        fprintf(fh, "  return nomRetval;\n");
      }
      else
        fprintf(fh, "\n");
      fprintf(fh, "}\n\n");
    }
};

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
      fprintf(fh, "    CORBA_Environment *ev)\n");

      fprintf(fh, "{\n");
      fprintf(fh, "  /* %sData* nomThis = %sGetData(nomSelf); */\n\n", pif->chrName, pif->chrName);

      fprintf(fh, "  /* call parent */\n");
      if(queryMessageReturnsAValue(pm))
        fprintf(fh, "return ");
      else
        fprintf(fh, "  ");
      fprintf(fh, "%s_%s_parent(nomSelf,\n", pif->chrName,  pm->chrName);
      /* Do parameters */
      emitMethodParams(pLocalPI, pif, pm->pParamArray);
      fprintf(fh, "    ev);\n");
      fprintf(fh, "}\n\n");
    }
};


void emitCFile(GPtrArray* pInterfaceArray)
{
  int a;
  PPARSEINFO pLocalPI=(PPARSEINFO)gScanner->user_data;

  for(a=0;a<pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pLocalPI->pInterfaceArray, a); 

      /* Only interfaces from the file given on the command line */
      if(!strcmp(pif->chrSourceFileName, pLocalPI->chrRootSourceFile))
        {
          /* Only interfaces which are fully defined. No forwarder */
          if(!pif->fIsForwardDeclaration)
            {
              gchar*  chrTemp;

              chrTemp=g_strconcat(pif->chrFileStem, "-template.c", NULL);
              
              //printInterface(pif);              
              if((pLocalPI->outFile=openOutfile(gScanner, chrTemp))!=NULL)
                {
                  emitCFileHeader(pLocalPI, pif);
                  emitNewMethods(pLocalPI, pif);
                  emitOverridenMethods(pLocalPI, pif);
                  closeOutfile(pLocalPI->outFile);
                  pLocalPI->outFile = NULL;
                }
              g_free(chrTemp);
            }/* fIsForwardDeclaration */
        }
    }
}
#endif


/*
 \param pArray Pointer to the list of parameters.
 */
void emitMethodParams(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;
  
  for(a=0;a<pArray->len;a++)
  {
    int b;
    gchar *chrType;
    PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);
    
    chrType= pm->chrType;
    
    /* For old IDL files using orbit IDL compiler */
    if(!strcmp(pm->chrType, "string"))
      chrType="CORBA_char*";
    else if(!strcmp(pm->chrType, "long"))
      chrType="CORBA_long";
    else if(!strcmp(pm->chrType, "unsigned long"))
      chrType="CORBA_unsigned_long";
    else if(!strcmp(pm->chrType, "boolean"))
      chrType="CORBA_boolean";
    
    switch(pm->uiDirection)
    {
      case PARM_DIRECTION_IN:
        g_printf(" const %s", chrType);
        break;
      case PARM_DIRECTION_OUT:
        g_printf(" %s*", chrType);
        break;
      case PARM_DIRECTION_INOUT:
        g_printf( " %s*", chrType);
        break;
      default:
        
        break;
    }
    for(b=0;b<pm->uiStar;b++)
      g_printf( "*");
    g_printf(" %s,", pm->chrName);
  }
}


/**
 The real stuff is in emitter.c!
 */
void emitReturnType(PPARSEINFO pLocalPI, PINTERFACE pif, PMETHOD pm)
{
  FILE* fh=pLocalPI->outFile;
  int b;
  gchar* chrType;
  
  chrType= pm->mpReturn.chrType;
  
  /* Support for orbit IDL files */
  if(!strcmp(pm->mpReturn.chrType, "long"))
    chrType="CORBA_long";
  else if(!strcmp(pm->mpReturn.chrType, "boolean"))
    chrType="CORBA_boolean";
  else if(!strcmp(pm->mpReturn.chrType, "unsigned long"))
    chrType="CORBA_unsigned_long";
  
  g_printf("%s", chrType);
  for(b=0;b<pm->mpReturn.uiStar;b++)
    g_printf( "*");
}


void emitMethodImplHeader(PINTERFACE pif, PMETHOD pm)
{
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data; 
  
  if(!pif->fIsForwardDeclaration)
  {
    g_printf("\n");
    moveToLine(pParseInfo->uiTokenLine);

    g_printf("NOMDLLEXPORT NOM_Scope ");
    emitReturnType(pParseInfo, pif, pm);
    g_printf(" NOMLINK impl_%s_%s(%s* nomSelf, ", pif->chrName, pm->chrName, pif->chrName);
    emitMethodParams(pParseInfo, pif, pm->pParamArray);
    g_printf(" CORBA_Environment *ev)");    
#if 0
    if((pParseInfo->outFile=openOutfile(gScanner, "-"))!=NULL)
    {
      g_print("ok");

      closeOutfile(pParseInfo->outFile);
      pParseInfo->outFile = NULL;

    }
#endif
    
  }
#if 0
  /* Do return type */
  fprintf(fh, "NOMDLLEXPORT NOM_Scope ");
  emitReturnType(pLocalPI, pif, pm);
  
  fprintf(fh, " NOMLINK impl_%s_%s(%s* nomSelf,\n", pif->chrName,  pm->chrName, pif->chrName);
  /* Do parameters */
  emitMethodParams(pLocalPI, pif, pm->pParamArray);
  fprintf(fh, "    CORBA_Environment *ev)\n");
  fprintf(fh, "{\n");
  fprintf(fh, "  /* %sData* nomThis = %sGetData(nomSelf); */\n", pif->chrName, pif->chrName);
  if(queryMessageReturnsAValue(pm)){
    fprintf(fh, "  ");
    emitReturnType(pLocalPI, pif, pm);
    fprintf(fh, " nomRetval;\n\n");
    fprintf(fh, "#warning In impl_%s_%s(): return value must be customized!\n",  pif->chrName,  pm->chrName);
    fprintf(fh, "  return nomRetval;\n");
  }
  else
    fprintf(fh, "\n");
  
  fprintf(fh, "}\n\n");
#endif
}
