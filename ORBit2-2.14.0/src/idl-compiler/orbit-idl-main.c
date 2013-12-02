/**************************************************************************

    orbit-idl-main.c (Driver program for the IDL parser & backend)

    Copyright (C) 1999 Elliot Lee

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: orbit-idl-main.c,v 1.32 2005/03/10 12:05:21 tml Exp $

***************************************************************************/

#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <libIDL/IDL.h>
#include <popt.h>

#include "orbit-idl2.h"

/* Settings made from the command line (prefaced with cl_) */
#ifdef USE_LIBIDL_CODE
static gboolean cl_disable_stubs = FALSE,
  cl_disable_skels = FALSE,
  cl_disable_common = FALSE,
  cl_disable_headers = FALSE,
  cl_enable_skeleton_impl = FALSE;

static int cl_idlwarnlevel = 2;
static int cl_debuglevel = 0;
static int cl_is_pidl = 0;
static int cl_output_version = 0;
static int cl_disable_idata = 0;
static int cl_enable_imodule = 0;
static int cl_add_imodule = 0;
static gboolean cl_disable_defs_skels = FALSE;
static gboolean cl_showcpperrors = FALSE;
static char *cl_output_lang = "c";
static char *cl_header_guard_prefix = "";
static char *cl_backend_dir = NULL;
static gboolean cl_onlytop = FALSE;
static char *cl_deps_file = NULL;
static char *cl_output_directory = "";
#else
static gboolean cl_disable_stubs = TRUE,
  cl_disable_skels = TRUE,
  cl_disable_common = TRUE,
  cl_disable_headers = TRUE,
  cl_enable_skeleton_impl = FALSE;
/* Voyager: Which file to create */
static gboolean 
  cl_create_headers = FALSE,
  cl_create_ctemplate = FALSE,
  cl_create_ih = FALSE;

static int cl_idlwarnlevel = 2;
static int cl_debuglevel = 0;
static int cl_is_pidl = 0;
static int cl_output_version = 0;
static int cl_disable_idata = 0;
static int cl_enable_imodule = 0;
static int cl_add_imodule = 0;
static gboolean cl_disable_defs_skels = FALSE;
static gboolean cl_showcpperrors = FALSE;
static char *cl_output_lang = "c";
static char *cl_header_guard_prefix = "";
static char *cl_backend_dir = NULL;
static gboolean cl_onlytop = TRUE;
static char *cl_deps_file = NULL;
static char *cl_output_directory = "";
#endif


#define BASE_CPP_ARGS "-D__ORBIT_IDL__ "
static GString *cl_cpp_args;

static char *c_output_formatter = NULL;

/* Callbacks for popt */
static void
cl_libIDL_version_callback(poptContext con, enum poptCallbackReason reason,
			   const struct poptOption *opt, char *arg,
			   void *data)
{
  g_print("libIDL %s (CORBA %s)",
	  IDL_get_libver_string(),
	  IDL_get_IDLver_string());

  exit(0);
}

static void
cl_cpp_callback(poptContext con, enum poptCallbackReason reason,
		const struct poptOption *opt, char *arg,
		void *data)
{
  g_assert(opt!=NULL);

  if(opt->shortName=='D')
    g_string_append(cl_cpp_args, "-D");
  else if(opt->shortName=='I')
    g_string_append(cl_cpp_args, "-I");

  g_string_append(cl_cpp_args, arg);
  g_string_append_c(cl_cpp_args, ' ');
}

static const
struct poptOption cl_libIDL_callback_options[] = {
  {NULL, '\0', POPT_ARG_CALLBACK, (void *)cl_libIDL_version_callback,
   0, NULL, NULL},
  {"libIDL-version", '\0', POPT_ARG_NONE, NULL, 0,
   "Show version of libIDL used.", NULL},
  {NULL, '\0', 0, NULL, 0, NULL, NULL}
};

static const
struct poptOption cl_cpp_callback_options[] = {
  {NULL, '\0', POPT_ARG_CALLBACK, (void *)cl_cpp_callback, 0, NULL, NULL},
  {"define", 'D', POPT_ARGFLAG_ONEDASH | POPT_ARG_STRING, NULL, 0,
   "Define value in preprocessor", NULL},
  {"include", 'I', POPT_ARGFLAG_ONEDASH | POPT_ARG_STRING, NULL, 0,
   "Add search path for include files", NULL},
  {NULL, '\0', 0, NULL, 0, NULL, NULL}
};

#ifdef USE_LIBIDL_CODE
static
struct poptOption options[] = {
  {NULL, '\0', POPT_ARG_INCLUDE_TABLE, &cl_cpp_callback_options, 0, NULL, NULL},
  {NULL, '\0', POPT_ARG_INCLUDE_TABLE, &cl_libIDL_callback_options, 0, NULL, NULL},
  {"version", 'v', POPT_ARG_NONE, &cl_output_version, 0, "Output compiler version and serial", NULL},
  {"lang", 'l', POPT_ARG_STRING, &cl_output_lang, 0, "Output language (default is C)", NULL},
  {"debug", 'd', POPT_ARG_INT, &cl_debuglevel, 0, "Debug level 0 to 4", NULL},
  {"idlwarnlevel", '\0', POPT_ARG_INT, &cl_idlwarnlevel, 0, "IDL warning level 0 to 4, default is 2", NULL},
  {"showcpperrors", '\0', POPT_ARG_NONE, &cl_showcpperrors, 0, "Show CPP errors", NULL},
  {"nostubs", '\0', POPT_ARG_NONE, &cl_disable_stubs, 0, "Don't output stubs", NULL},
  {"noskels", '\0', POPT_ARG_NONE, &cl_disable_skels, 0, "Don't output skels", NULL},
  {"nocommon", '\0', POPT_ARG_NONE, &cl_disable_common, 0, "Don't output common", NULL},
  {"noheaders", '\0', POPT_ARG_NONE, &cl_disable_headers, 0, "Don't output headers", NULL},
  {"noidata", '\0', POPT_ARG_NONE, &cl_disable_idata, 0, "Don't generate Interface type data", NULL},
  {"imodule", 'i', POPT_ARG_NONE, &cl_enable_imodule, 0, "Output only an imodule file", NULL},
  {"add-imodule", '\0', POPT_ARG_NONE, &cl_add_imodule, 0, "Output an imodule file", NULL},
  {"skeleton-impl", '\0', POPT_ARG_NONE, &cl_enable_skeleton_impl, 0, "Output skeleton implementation", NULL},
  {"backenddir", '\0', POPT_ARG_STRING, &cl_backend_dir, 0, "Override IDL backend library directory", "DIR"},
  {"c-output-formatter", '\0', POPT_ARG_STRING, &c_output_formatter, 0, "DEPRECATED and IGNORED", "PROGRAM"},
  {"onlytop", '\0', POPT_ARG_NONE, &cl_onlytop, 0, "Inhibit includes", NULL},
  {"pidl", '\0', POPT_ARG_NONE, &cl_is_pidl, 0, "Treat as Pseudo IDL", NULL},
  {"nodefskels", '\0', POPT_ARG_NONE, &cl_disable_defs_skels, 0, "Don't output defs for skels in header", NULL},
  {"deps", '\0', POPT_ARG_STRING, &cl_deps_file, 0, "Generate dependency info suitable for inclusion in Makefile", "FILENAME"},
  {"headerguardprefix", '\0', POPT_ARG_STRING, &cl_header_guard_prefix, 0, "Prefix for #ifdef header guards. Sometimes useful to avoid conflicts.", NULL},
  {"output-dir", '\0', POPT_ARG_STRING, &cl_output_directory, 0, "Where to put generated files. This directory must exist.", NULL},
  POPT_AUTOHELP
  {NULL, '\0', 0, NULL, 0, NULL, NULL}
};
#else
static
struct poptOption options[] = {
  {NULL, '\0', POPT_ARG_INCLUDE_TABLE, &cl_cpp_callback_options, 0, NULL, NULL},
  {NULL, '\0', POPT_ARG_INCLUDE_TABLE, &cl_libIDL_callback_options, 0, NULL, NULL},
  {"version", 'v', POPT_ARG_NONE, &cl_output_version, 0, "Output compiler version and serial", NULL},
  {"lang", 'l', POPT_ARG_STRING, &cl_output_lang, 0, "Output language (default is C)", NULL},
  {"debug", 'd', POPT_ARG_INT, &cl_debuglevel, 0, "Debug level 0 to 4", NULL},
  {"idlwarnlevel", '\0', POPT_ARG_INT, &cl_idlwarnlevel, 0, "IDL warning level 0 to 4, default is 2", NULL},
  {"showcpperrors", '\0', POPT_ARG_NONE, &cl_showcpperrors, 0, "Show CPP errors", NULL},
#if 0
  {"nostubs", '\0', POPT_ARG_NONE, &cl_disable_stubs, 0, "Don't output stubs", NULL},
  {"noskels", '\0', POPT_ARG_NONE, &cl_disable_skels, 0, "Don't output skels", NULL},
  {"nocommon", '\0', POPT_ARG_NONE, &cl_disable_common, 0, "Don't output common", NULL},
#endif
  {"ihfile", '\0', POPT_ARG_NONE, &cl_create_ih, 0, "Output implementation file", NULL},
  {"header", '\0', POPT_ARG_NONE, &cl_create_headers, 0, "Output headers", NULL},
  {"c-template", '\0', POPT_ARG_NONE, &cl_create_ctemplate, 0, "Output a C template file with method bodies", NULL},
  {"add-imodule", '\0', POPT_ARG_NONE, &cl_add_imodule, 0, "Output an imodule file", NULL},
  {"skeleton-impl", '\0', POPT_ARG_NONE, &cl_enable_skeleton_impl, 0, "Output skeleton implementation", NULL},
  {"backenddir", '\0', POPT_ARG_STRING, &cl_backend_dir, 0, "Override IDL backend library directory", "DIR"},
#if 0
  {"onlytop", '\0', POPT_ARG_NONE, &cl_onlytop, 0, "Inhibit includes", NULL},
#endif
  {"pidl", '\0', POPT_ARG_NONE, &cl_is_pidl, 0, "Treat as Pseudo IDL", NULL},
  {"nodefskels", '\0', POPT_ARG_NONE, &cl_disable_defs_skels, 0, "Don't output defs for skels in header", NULL},
  {"deps", '\0', POPT_ARG_STRING, &cl_deps_file, 0, "Generate dependency info suitable for inclusion in Makefile", "FILENAME"},
  {"headerguardprefix", '\0', POPT_ARG_STRING, &cl_header_guard_prefix, 0, "Prefix for #ifdef header guards. Sometimes useful to avoid conflicts.", NULL},
  {"output-dir", '\0', POPT_ARG_STRING, &cl_output_directory, 0, "Where to put generated files. This directory must exist.", NULL},
  POPT_AUTOHELP
  {NULL, '\0', 0, NULL, 0, NULL, NULL}
};
#endif

/********** main routines **********/
int main(int argc, const char *argv[])
{
  poptContext pcon;
  int rc, retval = 0;
  const char *arg;
  OIDL_Run_Info rinfo;

  /* Argument parsing, etc. */
  cl_cpp_args = g_string_new("-D__ORBIT_IDL__ ");

  pcon = poptGetContext ("orbit-idl-2", argc, argv, options, 0);
  poptSetOtherOptionHelp (pcon, "<IDL files>");

  if(argc < 2) {
  	poptPrintUsage(pcon, stdout, 0);
	return 0;
  }

  if((rc=poptGetNextOpt(pcon)) < -1) {
    g_print ("orbit-idl-2: bad argument %s: %s\n", 
    		poptBadOption(pcon, POPT_BADOPTION_NOALIAS),
		poptStrerror(rc));
    exit(0);
  }

  if (cl_output_version) {
	  printf ("orbit-idl-2 %s - serial %d\n\n",
		  VERSION, ORBIT_CONFIG_SERIAL);
	  exit (0);
  }

  if (c_output_formatter != NULL)
	  g_warning ("Please do not use the 'c-output-formatter' option. It is ignored and will soon go away.");

  /* --header given */
  if(cl_create_headers)
    cl_disable_headers=FALSE;
  /* --ihfile given */
  if(cl_create_ih)
    cl_enable_skeleton_impl=TRUE;
  /* --c-template given */
  if(cl_create_ctemplate)
    cl_disable_stubs=FALSE;


  /* Prep our run info for the backend */
  rinfo.cpp_args = cl_cpp_args->str;
  rinfo.debug_level = cl_debuglevel;
  rinfo.idl_warn_level = cl_idlwarnlevel;
  rinfo.show_cpp_errors = cl_showcpperrors;
  rinfo.is_pidl = cl_is_pidl;
  rinfo.do_skel_defs = !cl_disable_defs_skels;
  rinfo.enabled_passes =
     (cl_disable_stubs?0:OUTPUT_STUBS)
    |(cl_disable_skels?0:OUTPUT_SKELS)
    |(cl_disable_common?0:OUTPUT_COMMON)
    |(cl_disable_headers?0:OUTPUT_HEADERS)
    |(cl_enable_skeleton_impl?OUTPUT_SKELIMPL:0)
    |(cl_add_imodule?OUTPUT_IMODULE:0)
    |(cl_deps_file != NULL?OUTPUT_DEPS:0);

  rinfo.deps_file = cl_deps_file;

  if (cl_enable_imodule) /* clobber */
    rinfo.enabled_passes =
      OUTPUT_COMMON | OUTPUT_HEADERS | OUTPUT_IMODULE;

  rinfo.output_language = cl_output_lang;
  rinfo.header_guard_prefix = cl_header_guard_prefix;
  rinfo.output_directory = cl_output_directory;
  rinfo.backend_directory = cl_backend_dir;
  rinfo.onlytop = cl_onlytop;
  rinfo.idata = !cl_disable_idata;
  
  printf ("orbit-idl-2 " VERSION " compiling\n");
  printf (" %s mode, %s preprocessor errors, passes: %s%s%s%s%s%s\n\n",
	  rinfo.is_pidl ? "pidl" : "",
	  rinfo.show_cpp_errors ? "show" : "hide",
	  cl_disable_stubs ? "" : "stubs ",
	  cl_disable_skels ? "" : "skels ",
	  cl_disable_common ? "" : "common ",
	  cl_disable_headers ? "" : "headers ",
	  cl_enable_skeleton_impl ? "skel_impl " : "",
	  cl_enable_imodule ? "imodule" : "");
	   
  /* Do it */
  while((arg=poptGetArg(pcon))!=NULL) {
    rinfo.input_filename = g_strdup (arg); /* g_path_get_basename(arg); - what !? */
    if (!orbit_idl_to_backend(arg, &rinfo)) {
      g_warning("%s compilation failed", arg);
      retval = 1;
    }
    g_free(rinfo.input_filename);
  }

  return retval;
}
