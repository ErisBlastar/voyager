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
          fprintf(fh, "    const %s", chrType);
          break;
        case PARM_DIRECTION_OUT:
          fprintf(fh, "    %s*", chrType);
          break;
        case PARM_DIRECTION_INOUT:
          fprintf(fh, "    %s*", chrType);
          break;
        default:

          break;
        }
      for(b=0;b<pm->uiStar;b++)
        fprintf(fh, "*");
      fprintf(fh, " %s,\n", pm->chrName);
    }
}


/*
 \param pArray Pointer to the list of parameters.
 */
void emitMethodParamsForNOMCompiler(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;
  
  for(a=0;a<pArray->len;a++)
  {
    int b;
    gchar *chrType;
    PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);
    
    chrType= pm->chrType;

#if 0
    /* For old IDL files using orbit IDL compiler */
    if(!strcmp(pm->chrType, "string"))
      chrType="CORBA_char*";
    else if(!strcmp(pm->chrType, "long"))
      chrType="CORBA_long";
    else if(!strcmp(pm->chrType, "unsigned long"))
      chrType="CORBA_unsigned_long";
    else if(!strcmp(pm->chrType, "boolean"))
      chrType="CORBA_boolean";
#endif
    
    switch(pm->uiDirection)
    {
      case PARM_DIRECTION_IN:
        fprintf(fh, " in %s", chrType);
        break;
      case PARM_DIRECTION_OUT:
        fprintf(fh, " out  %s", chrType);
        break;
      case PARM_DIRECTION_INOUT:
        fprintf(fh, " inout %s", chrType);
        break;
      default:
      
        break;
    }
    for(b=0;b<pm->uiStar;b++)
      fprintf(fh, "*");
    fprintf(fh, " %s", pm->chrName);
    if(a<pArray->len-1)
      fprintf(fh, ",");
      
  }
}


/*
  \param pArray Pointer to the list of parameters.
 */
void emitMethodParamsNoTypes(PPARSEINFO pLocalPI, PINTERFACE pif, GPtrArray *pArray)
{
  FILE* fh=pLocalPI->outFile;
  int a;

  for(a=0;a<pArray->len;a++)
    {
      PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);
      fprintf(fh, " %s,", pm->chrName);
    }
}

/**
   This function emits the return type of a method. It translates CORBA identifiers
   into another representation e.g. long->glong. At the moment it's used to be compatible
   with old source written using the orbit IDL compiler.
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

  fprintf(fh, "%s", chrType);
  for(b=0;b<pm->mpReturn.uiStar;b++)
    fprintf(fh, "*");
}


void emitReturnTypeForNOMCompiler(PPARSEINFO pLocalPI, PINTERFACE pif, PMETHOD pm)
{
  FILE* fh=pLocalPI->outFile;
  int b;
  gchar* chrType;
  
  chrType= pm->mpReturn.chrType;

#if 0
  /* Support for orbit IDL files */
  if(!strcmp(pm->mpReturn.chrType, "long"))
    chrType="CORBA_long";
  else if(!strcmp(pm->mpReturn.chrType, "boolean"))
    chrType="CORBA_boolean";
  else if(!strcmp(pm->mpReturn.chrType, "unsigned long"))
    chrType="CORBA_unsigned_long";
#endif
  
  if(!strcmp(pm->mpReturn.chrType, "boolean"))
    chrType="gboolean";
  
  fprintf(fh, "%s", chrType);
  for(b=0;b<pm->mpReturn.uiStar;b++)
    fprintf(fh, "*");
}
