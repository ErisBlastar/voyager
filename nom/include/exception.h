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

#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <signal.h>
#include <setjmp.h>

#define TRY(a) if(1==1){\
                  struct sigaction a ##_saOld;    \
                  struct sigaction a ##_saNew;    \
                  jmp_buf a ## _env;              \
                                                  \
                  void a ## _handler(int iSig) {  \
                    /*  printf("%s: %d\n", __FUNCTION__, iSig);*/ \
                    longjmp(a ## _env, 1);        \
                  };                              \
                                                  \
                  a ## _saNew.sa_handler = a ## _handler; \
                  a ## _saNew.sa_flags = SA_SYSV;         \
                  sigemptyset (&a ## _saNew.sa_mask);     \
                  sigaction (SIGSEGV, &a ## _saNew, &a ## _saOld);\
                                                          \
                  if(0==setjmp(a ## _env))

#define CATCH else

#define END_CATCH(a)  sigaction (SIGSEGV, &a ## _saOld, NULL); \
                      } /* if(1==1) */

#ifdef __OS2__
#define LOUD DosBeep(1000, 20); DosBeep(2000, 20); DosBeep(3000, 20);
#else
#define LOUD
#endif

#endif /* EXCEPTION_H_INCLUDED */






