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
* Portions created by the Initial Developer are Copyright (C) 2005-2008
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
/** \file
    This File contains the necessary thunks for accessing methods and data.
 */

#ifdef __OS2__
# include <os2.h>
#endif /* __OS2__ */


#include <glib.h>


/*
  Thunking code to get the instance var address from an object pointer pushed
  on the stack. The following translates into this assembler code:

  MOV EAX,DWORD PTR [ESP+4] ;Move object ptr into EAX
  ADD EAX, +4
  RET
*/
/* When changing the size of this array make sure to change the app. defines in thunk.h */
gulong thunk[]={0x0424448b, 0x00000405, 0x0000c300};

/*
MOV ECX,DWORD PTR [ESP+4] : move object pointer from stack in ECX
MOV EDX,DWORD PTR [ECX]   : move [ECX] in EDX -> mtab in EDX
JMP DWORD PTR [EDX+0ACh]  : JMP to address pointing to by EDX+0ACh
 */
/* When changing the size of this array make sure to change the app. defines in thunk.h */
gulong mThunkCode[]={0x04244c8b, 0xff00518b, 0x0000aca2 , 0x16000000};


