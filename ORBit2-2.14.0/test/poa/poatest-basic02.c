/*
 * CORBA POA tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Mark McLoughlin <mark@skynet.ie>
 */

/*
 * Test 2 : poatest-basic02.c
 *     o Root POA.
 *     o implicitly activated object with servant_to_reference.
 */

#include <stdio.h>
#include <stdlib.h>

#include <orbit/orbit.h>

#include "poatest-basic-shell.h"

static void
poatest_test_impl (PortableServer_Servant servant, CORBA_Environment *ev) { }

PortableServer_ServantBase__epv base_epv = {
	NULL,     /* _private    */
	NULL,     /* finalize    */
	NULL      /* default_POA */
};

POA_poatest__epv poatest_epv = {
	NULL,               /* _private */
	poatest_test_impl   /* test     */
};

POA_poatest__vepv poatest_vepv = {
	&base_epv,     /* _base_epv */
	&poatest_epv   /* poatest_epv  */
};

POA_poatest poatest_servant = {
	NULL,           /* _private */
	&poatest_vepv   /* vepv     */
};

poatest
poatest_run (PortableServer_POA rootpoa,
	     PortableServer_POAManager rootpoa_mgr)
{
	CORBA_Environment  ev;
	poatest            poatest_obj;

 	CORBA_exception_init( &ev );
 
	/*
	 * Initialise the servant.
	 */
	POA_poatest__init (&poatest_servant, &ev);
	if (POATEST_EX (&ev)) {
		POATEST_PRINT_EX ("POA_poatest__init : ", &ev);
		return CORBA_OBJECT_NIL;
	}

	/*
	 * Imlicitly activate the Object.
	 * See sections 11.2.7 and 11.3.8.21.
	 */
	poatest_obj = PortableServer_POA_servant_to_reference (rootpoa, &poatest_servant, &ev);
	if (POATEST_EX (&ev)) {
		POATEST_PRINT_EX ("servant_to_reference : ", &ev);
		return CORBA_OBJECT_NIL;
	}

	/*
	 * Activate the POAManager. POA will now accept requests
	 */
	PortableServer_POAManager_activate (rootpoa_mgr, &ev);
	if (POATEST_EX (&ev)) {
		POATEST_PRINT_EX ("POAManager_activate : ", &ev);
		return CORBA_OBJECT_NIL;
	}

	return poatest_obj;
}
