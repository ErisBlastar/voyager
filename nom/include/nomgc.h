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
* Portions created by the Initial Developer are Copyright (C) 2005-2006
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

#ifndef NOMGC_H_INCLUDED
#define NOMGC_H_INCLUDED

typedef struct _REGDLL
{
  GSList*      dllList;
#ifdef __OS2__
  qsPtrRec_t * pMainAnchor;
#endif
}REGDLL,*HREGDLL;

/* Garbage collector */
#ifdef __OS2__
void _System nomInitGarbageCollection(void* pMemPtr); //The parameter will go away
#else
void nomInitGarbageCollection(void* pMemPtr); //The parameter will go away
#endif 
NOMEXTERN HREGDLL NOMLINK nomBeginRegisterDLLWithGC(void);
NOMEXTERN void NOMLINK nomEndRegisterDLLWithGC(const HREGDLL hRegisterDLL );
NOMEXTERN gboolean NOMLINK nomRegisterDLLByName(const HREGDLL hRegisterDLL, const char* chrDLLName);
NOMEXTERN void NOMLINK  nomRegisterDataAreaForGC(char* pStart, char* pEnd);
#if 0
NOMEXTERN gboolean NOMLINK nomRegisterDLLByHandle(const HREGDLL hRegisterDLL, const gpointer pDLLHandle);
#endif

NOMEXTERN gboolean NOMLINK nomQueryUsingNameIsDLLRegistered(const gchar *chrName);

#endif /* NOMGC_H_INCLUDED */




















