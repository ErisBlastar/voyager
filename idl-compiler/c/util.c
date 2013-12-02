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
#include <stdlib.h>
#include <string.h>

#include <glib.h> 
#include <glib/gprintf.h> 

#define INCL_FILE
#include "parser.h"

extern GScanner *gScanner;

/**
   Helper function which scans the array of parent interfaces and returns the structure of the
   interface which introduced the given method. 

   \Param pif Pointer to an interface which parents should be scanned. 
   \PARAM chrName Name of the method.

   \Returns If no interface with a method with that name can be found NULL is returned otherwise a
   pointer to the interface structure..
 */
PINTERFACE findInterfaceFromMethodName(PINTERFACE pif, gchar* chrName)
{
  PINTERFACE pifParent=pif;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  if(!pParseInfo|| !pifParent)
    return NULL;

  while((pifParent=getParentInterface(pifParent))!=NULL)
    {
      int a;
      GPtrArray *pArray=pifParent->pMethodArray;
      gboolean fFound=FALSE;

      for(a=0;a<pArray->len && !fFound; a++){
        PMETHOD pm=(PMETHOD)g_ptr_array_index(pArray, a);

        if(!strcmp(pm->chrName, chrName))
          fFound=TRUE;
      }/* for() */

      if(fFound)
        break;
    }/* while() */
  return pifParent;
}

/**
   Helper function which scans the array of parent interfaces and returns the info structure of the
    given method. 

   \Param pif Pointer to an interface which parents should be scanned. 
   \PARAM chrName Name of the method.

   \Returns If no info structure for a method with that name can be found NULL is returned otherwise a
   pointer to the METHOD structure..
 */
PMETHOD findMethodInfoFromMethodName(PINTERFACE pif, gchar* chrName)
{
  PMETHOD pm;
  PINTERFACE pifParent=pif;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  if(!pParseInfo|| !pifParent)
    return NULL;

  while((pifParent=getParentInterface(pifParent))!=NULL)
    {
      int a;
      GPtrArray *pArray=pifParent->pMethodArray;
      gboolean fFound;

      fFound=FALSE;
      for(a=0;a<pArray->len && !fFound; a++){
        pm=(PMETHOD)g_ptr_array_index(pArray, a);

        if(!strcmp(pm->chrName, chrName))
          fFound=TRUE;
      }/* for() */

      if(fFound)
        break;

      pm=NULL;
    }/* while() */
  return pm;
}

/**
   Helper function which scans the array of known interfaces and returns the interface
   structure for the given name. 

   \PARAM chrName Name of the interface.
   \Returns If no interface with that name can be found NULL is returned otherwise a
   pointer to the interface structure..
 */
PINTERFACE findInterfaceFromName(gchar* chrName)
{
  int a;
  PPARSEINFO pParseInfo=(PPARSEINFO)gScanner->user_data;

  for(a=0;a<pParseInfo->pInterfaceArray->len;a++)
    {
      PINTERFACE pif=g_ptr_array_index(pParseInfo->pInterfaceArray, a);
      if(!strcmp(chrName, pif->chrName))
        return pif;
    }

  return NULL;
}


/**
   Returns the interface structure (holding all the interface information) of the
   parent of an interface.

   \Param pif Pointer to an interface structure.
   \Returns The interface data structure of the parent interface or NULL if the
   interface has no parent.

 */
PINTERFACE getParentInterface(PINTERFACE pif)
{
  if(pif->chrParent==NULL)
    return NULL;

  return findInterfaceFromName(pif->chrParent);
}

gboolean queryMessageReturnsAValue(PMETHOD pm)
{
  if(pm->mpReturn.uiStar)
    return TRUE;

  if(!strcmp(pm->mpReturn.chrType, "void"))
    return FALSE;

  return TRUE;
}

