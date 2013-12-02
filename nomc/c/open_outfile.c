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

typedef struct
{
  FILE* pFile;
  gchar* chrOutFileName;
}OPENFILEINFO, *POPENFILEINFO;

GArray *pOpenFileArray=NULL;

/**
   This function opens a file at the location specified by the arguments for the compiler.
   To support several class definitions in one output file this function maintains a list of
   once opened files. If the same file is asked again, the file will be opened for appending.
   Otherwise a new file is created.

   \param gScanner A GScanner struct used during parsing.
   \param chrOutName The filename with extension added to the path build from the command
   line arguments when calling the compiler

   \Returns A filehandle or NULL in case of error.
 */
FILE* openOutfile(GScanner *gScanner, gchar* chrOutName)
{
  gchar*  chrTemp;
  FILE* pFile;
  int a;
  OPENFILEINFO oi;

  PARSEINFO *parseInfo=(PPARSEINFO)gScanner->user_data;

  if(!pOpenFileArray)
    pOpenFileArray=g_array_new(FALSE, TRUE, sizeof(OPENFILEINFO));
  
  chrTemp=g_build_filename(parseInfo->chrOutfilePath, chrOutName, NULL);
  if(!chrOutName)
    {
      g_message("No output file name. Check if a filestem is specified in the IDL file.");
      exit(1);
    }

  for(a=0;a<pOpenFileArray->len;a++)
    {
      POPENFILEINFO poi=g_array_index(pOpenFileArray, POPENFILEINFO, a);
      if(!strcmp(poi->chrOutFileName, chrTemp))
        {
          g_free(chrTemp);
          if((poi->pFile=fopen(poi->chrOutFileName, "a"))==NULL)
            {
              g_message("Can't open output file %s", poi->chrOutFileName);
              exit(1);
            }
          return poi->pFile;
        }
    }

  if((pFile=fopen(chrTemp, "w"))==NULL)
    {
      g_message("Can't open output file %s", chrTemp);
      g_free(chrTemp);
      exit(1);
    }
  oi.pFile=pFile;
  oi.chrOutFileName=chrTemp;
  g_array_append_val(pOpenFileArray, oi);

  return pFile;
}

/**
   Close the file opened using openOutFile(). Note that you have to close the
   file before working on another interface because otherwise reopening will fail.
 */
void closeOutfile(FILE* pFile)
{
  int a;

  fclose(pFile);
  for(a=0;a<pOpenFileArray->len;a++)
    {
      POPENFILEINFO poi=g_array_index(pOpenFileArray, POPENFILEINFO, a);
      if(poi->pFile==pFile)
        {
          poi->pFile=NULL;
          break;
        }
    }
}

