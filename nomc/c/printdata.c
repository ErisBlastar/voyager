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
#endif 
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

#include <glib/gprintf.h>

#include "nom.h"
#include "nomtk.h"

#include "parser.h"

/* The pointer array holding the interfaces we found */
//extern PPARSEINFO pParseInfo;

extern GScanner *gScanner;

#if 0
static void printOverridenMethods(GPtrArray *pArray)
{
  int a;

  for(a=0;a<pArray->len;a++)
    {
      POVERMETHOD pom=(POVERMETHOD)g_ptr_array_index(pArray, a);
      g_printf("\t\tName:\t\t%s\n", pom->chrName);
    }
}

static void printInstanceVars(GPtrArray *pArray)
{
  int a;

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHODPARAM piv=(PMETHODPARAM)g_ptr_array_index(pArray, a);
      g_printf("\t\tType:\t\t%s", piv->chrType);
      for(b=0;b<piv->uiStar;b++)
        g_printf("*");
      g_printf("\n\t\tName:\t\t%s\n\n", piv->chrName);
    }
}
#endif


static void printMethodParams(GPtrArray *pArray)
{
  int a;
  
  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHODPARAM pm=(PMETHODPARAM)g_ptr_array_index(pArray, a);
      switch(pm->uiDirection)
        {
        case PARM_DIRECTION_IN:
          g_printf("\t\t\tDirection:\t%s\n", "in");
          break;
        case PARM_DIRECTION_OUT:
          g_printf("\t\t\tDirection:\t%s\n", "out");
          break;
        case PARM_DIRECTION_INOUT:
          g_printf("\t\t\tDirection:\t%s\n", "inout");
          break;
        default:
          break;
        }
      g_printf("\t\t\tType:\t\t%s", pm->chrType);
      for(b=0;b<pm->uiStar;b++)
        g_printf("*");
      g_printf("\n\t\t\tName:\t\t%s\n", pm->chrName);      
    }
}


static void printMethods(GPtrArray *pArray)
{
  int a;

  for(a=0;a<pArray->len;a++)
    {
      int b;
      PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);
      g_printf("\t\tReturn type:\t%s", pm->mpReturn.chrType);
      for(b=0;b<pm->mpReturn.uiStar;b++)
        g_printf("*");
      g_printf("\n\t\tName:\t\t%s\n", pm->chrName);
      if(pm->pParamArray->len)
        {
          g_printf("\t\tNum parameters:\t%d\n", pm->pParamArray->len);
          printMethodParams(pm->pParamArray);
        }
      else
        g_printf("\t\t(No parameters)\n\n");
    }
}

void printAllInterfaces(void)
{
  int a;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  
  for(a=0;a<pParseInfo->pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pParseInfo->pInterfaceArray, a);
      g_printf("Found Interface:\n");  
      g_printf("\tName:\t\t%s\n", pif->chrName);
      g_printf("\tParent:\t\t%s\n", (pif->chrParent ? pif->chrParent : "No parent"));
      //g_printf("\tMajor:\t\t%ld\n", pif->ulMajor);
      //g_printf("\tMinor:\t\t%ld\n", pif->ulMinor);
      g_printf("\tForward decl.:\t%s\n", (pif->fIsForwardDeclaration ? "Yes" : "No"));
      //g_printf("\tMetaclass:\t%s\n", (pif->chrMetaClass ? pif->chrMetaClass : "None"));
      g_printf("\tSource file:\t%s\n", pif->chrSourceFileName);
      //if(pif->chrFileStem)
        //g_printf("\tFile stem:\t%s\n", pif->chrFileStem);
      /* Print instance vars */
      //g_printf("\tInstance vars:\t%d\n", pif->pInstanceVarArray->len);
      //printInstanceVars(pif->pInstanceVarArray);
      /* Print methods */
      g_printf("\tNew methods:\t%d\n", pif->pMethodArray->len);
      printMethods(pif->pMethodArray);
      /* Print overriden methods */
      //g_printf("\tOverriden methods:\t%d\n", pif->pOverrideArray->len);
      //printOverridenMethods(pif->pOverrideArray);
    }
}

void printInterface(PINTERFACE pif)
{
  g_printf("Found Interface:\n");  
  g_printf("\tName:\t\t%s\n", pif->chrName);
  g_printf("\tParent:\t\t%s\n", (pif->chrParent ? pif->chrParent : "No parent"));
  //g_printf("\tMajor:\t\t%ld\n", pif->ulMajor);
  //g_printf("\tMinor:\t\t%ld\n", pif->ulMinor);
  g_printf("\tForward decl.:\t%s\n", (pif->fIsForwardDeclaration ? "Yes" : "No"));
  //g_printf("\tMetaclass:\t%s\n", (pif->chrMetaClass ? pif->chrMetaClass : "None"));
  g_printf("\tSource file:\t%s\n", pif->chrSourceFileName);
  //if(pif->chrFileStem)
    //g_printf("\tFile stem:\t%s\n", pif->chrFileStem);
  /* Print instance vars */
  //g_printf("\tInstance vars:\t%d\n", pif->pInstanceVarArray->len);
  //printInstanceVars(pif->pInstanceVarArray);
  /* Print methods */
  g_printf("\tNew methods:\t%d\n", pif->pMethodArray->len);
  printMethods(pif->pMethodArray);
  /* Print overriden methods */
  //g_printf("\tOverriden methods:\t%d\n", pif->pOverrideArray->len);
  //printOverridenMethods(pif->pOverrideArray);
}
