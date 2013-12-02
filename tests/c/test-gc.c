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
# define INCL_DOSPROCESS
# define INCL_DOS
# define INCL_DOSPROFILE
# define INCL_DOSERRORS
# include <os2.h>
#endif /* __OS2__ */

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

#include <glib.h> 
#include <glib/gprintf.h>

#include "nom.h"
#include "nomtk.h"
#include "nomgc.h"

#define MEMSIZE   17
#define NUMALLOCS 25000*2

/**
  
 */
int main(int argc, char **argv)
{
  int a;
  
  /* GSlice isn't GC aware */
  g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);
  
  nomInitGarbageCollection(NULL);


  g_message("------------------------------------------------------------------");

  for(a=1; a<=100;a++)
  {
    int b;
    
    g_message("Loop: %d", a);
    //printf("Loop: %d\n", a);
    
    /*  */
    for(b=1; b<=NUMALLOCS;b++)
	{
      gpointer gp;

      gp=g_malloc(MEMSIZE*a);
      g_assert(gp);      
    }
    g_message("Allocated %d chunks of %d bytes of memory -> %d bytes", NUMALLOCS, MEMSIZE*a, NUMALLOCS*MEMSIZE*a);
  }
  
  g_message("               --> OK");
  
  return 0;
};


