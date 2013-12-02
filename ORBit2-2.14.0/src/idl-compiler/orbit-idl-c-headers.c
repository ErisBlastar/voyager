#include "config.h"
#include "orbit-idl-c-backend.h"

#include <string.h>
#include <ctype.h>

/*
  Voyager implementation notes:

  The name of the explicit metaclass is found in ch_output_const_dcl(). That
  function is responsible for putting constants into the header file.
*/

/* ch = C header */
static void ch_output_types(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
#ifdef USE_LIBIDL_CODE
static void ch_output_poa(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
#endif
static void ch_output_itypes (IDL_tree tree, OIDL_C_Info *ci);
static void ch_output_stub_protos(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_skel_protos(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_voyager(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_VoyagerOutputClassDeclaration(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);

static GString *gsMetaClassName=NULL; /* Holding an explicit metaclass if any */

void
orbit_idl_output_c_headers (IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  fprintf (ci->fh, OIDL_C_WARNING);
  fprintf (ci->fh, "/*\n * %s\n */\n\n", rinfo->input_filename);

  fprintf(ci->fh, "#ifndef %s%s_H\n", rinfo->header_guard_prefix, ci->c_base_name);
  fprintf(ci->fh, "#define %s%s_H 1\n\n", rinfo->header_guard_prefix, ci->c_base_name);

  fprintf(ci->fh, "#include <glib.h>\n\n");
#ifdef USE_LIBIDL_CODE
  fprintf(ci->fh, "#define ORBIT_IDL_SERIAL %d\n", ORBIT_CONFIG_SERIAL);
  fprintf(ci->fh, "#include <orbit/orbit-types.h>\n\n");
#else
  fprintf(ci->fh, "#include <nomcls.h> /* This is needed for _nomNew() */\n\n");
#endif

  fprintf(ci->fh, "#ifdef __cplusplus\n");
  fprintf(ci->fh, "extern \"C\" {\n");
  fprintf(ci->fh, "#endif /* __cplusplus */\n\n");

  /* Forward declare our class */
  ch_VoyagerOutputClassDeclaration(tree, rinfo, ci);
  /* Do all the typedefs, etc. */
  fprintf(ci->fh, "\n/** typedefs **/\n");
  ch_output_types(tree, rinfo, ci);

#ifdef USE_LIBIDL_CODE  
  if ( ci->do_skel_defs ) {
  	/* Do all the POA structures, etc. */
  	fprintf(ci->fh, "\n/** POA structures **/\n");
  	ch_output_poa(tree, rinfo, ci);
  	fprintf(ci->fh, "\n/** skel prototypes **/\n");
  	ch_output_skel_protos(tree, rinfo, ci);
  }
#endif

  fprintf(ci->fh, "\n/** stub prototypes **/\n");
  fprintf(ci->fh, "  /* (%s, %s line %d) */\n", __FILE__, __FUNCTION__, __LINE__);
  ch_output_stub_protos(tree, rinfo, ci);

  /* Voyager special stuff */
  fprintf(ci->fh, "\n/** Voyager **/\n");
  ch_output_voyager(tree, rinfo, ci);

  if ( ci->ext_dcls && ci->ext_dcls->str )
    fputs( ci->ext_dcls->str, ci->fh);	/* this may be huge! */

#ifdef USE_LIBIDL_CODE
  if (rinfo->idata) {
    /* FIXME: hackish ? */
    fprintf(ci->fh, "\n#include <orbit/orb-core/orbit-interface.h>\n\n");

    ch_output_itypes(tree, ci);
  }
#endif

  fprintf(ci->fh, "#ifdef __cplusplus\n");
  fprintf(ci->fh, "}\n");
  fprintf(ci->fh, "#endif /* __cplusplus */\n\n");

#ifdef USE_LIBIDL_CODE
  fprintf(ci->fh, "#ifndef EXCLUDE_ORBIT_H\n");
  fprintf(ci->fh, "#include <orbit/orbit.h>\n\n");
  fprintf(ci->fh, "#endif /* EXCLUDE_ORBIT_H */\n");
#endif

  fprintf(ci->fh, "#endif /* %s%s_H */\n", rinfo->header_guard_prefix, ci->c_base_name);

#ifdef USE_LIBIDL_CODE
  fprintf(ci->fh, "#undef ORBIT_IDL_SERIAL\n");
#endif
}

static void ch_output_voyager(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{

  if (!tree)
    return;

  switch (IDL_NODE_TYPE (tree)) {
  case IDLN_INTERFACE:
    {
      char *id;

      //ch_output_interface (tree, rinfo, ci);
      id = orbit_cbe_get_typespec_str(tree);

      fprintf(ci->fh, "\n/** Voyager  (%s: %s line %d) **/\n", __FILE__, __FUNCTION__, __LINE__);

      /* C specific class structure */
      fprintf(ci->fh, "/*\n * (%s, %s line %d)\n */\n", __FILE__, __FUNCTION__, __LINE__);
      fprintf(ci->fh, "/*\n * C specific class structure\n */\n");
      fprintf(ci->fh, "NOMEXTERN struct %sCClassDataStructure {\n", id);
      fprintf(ci->fh, "   nomMethodTabs parentMtab;\n   nomDToken instanceDataToken;\n} NOMDLINK %sCClassData;\n\n", id);
      /* vomNewClass() */
      fprintf(ci->fh, "/*\n * Class creation function\n */\n");
      fprintf(ci->fh, "NOMEXTERN NOMClass * NOMLINK %sNewClass(gulong somtmajorVersion, gulong somtminorVersion);\n",
              id);
      fprintf(ci->fh, "\n");

      fprintf(ci->fh, "/* %s: Line %d %s */\n", __FUNCTION__, __LINE__, __FUNCTION__);
      if(NULL==gsMetaClassName)
        fprintf(ci->fh, "#define _%s %sClassData.classObject\n", id, id);
      else
        fprintf(ci->fh, "#define _%s (%s*)%sClassData.classObject\n", id, gsMetaClassName->str, id);
      /* New() macro */
      fprintf(ci->fh, "\n/*\n * New macro for %s\n */\n", id);
      fprintf(ci->fh, "#define %sNew() \\\n", id);
      /* Changed for typesafetyness */
      fprintf(ci->fh, "        ((%s*)_nomNew((_%s ? _%s : (%s*)%sNewClass(%s_MajorVersion, %s_MinorVersion)), (void*) 0))\n",
              id, id, id, id, id, id ,id);
      //  fprintf(ci->fh, "        (_nomNew((_%s ? _%s : %sNewClass(%s_MajorVersion, %s_MinorVersion)), (void*) 0))\n",
      //      id, id, id, id ,id);

      fprintf(ci->fh, "\n");
      g_free (id);
    }
    break;
  default:
    break;
  }

  switch (IDL_NODE_TYPE (tree)) {
  case IDLN_MODULE:
    ch_output_voyager (IDL_MODULE (tree).definition_list, rinfo, ci);
    break;
  case IDLN_LIST: {
    IDL_tree sub;
    
    for (sub = tree; sub; sub = IDL_LIST (sub).next) {
      ch_output_voyager (IDL_LIST (sub).data, rinfo, ci);
    }
  }
  break;
  case IDLN_INTERFACE:
    ch_output_voyager (IDL_INTERFACE (tree).body, rinfo, ci);
    break;
  default:
    break;
  }
}

static void
ch_output_var(IDL_tree val, IDL_tree name, OIDL_C_Info *ci)
{
  orbit_cbe_write_typespec(ci->fh, val);

  fprintf(ci->fh, " ");
  switch(IDL_NODE_TYPE(name)) {
  case IDLN_IDENT:
    fprintf(ci->fh, "%s", IDL_IDENT(name).str);
    break;
  case IDLN_TYPE_ARRAY:
    {
      IDL_tree curitem;

      fprintf(ci->fh, "%s", IDL_IDENT(IDL_TYPE_ARRAY(name).ident).str);
      for(curitem = IDL_TYPE_ARRAY(name).size_list; curitem; curitem = IDL_LIST(curitem).next) {
	fprintf(ci->fh, "[%" IDL_LL "d]", IDL_INTEGER(IDL_LIST(curitem).data).value);
      }
    }
    break;
  default:
    g_error("Weird varname - %s", IDL_tree_type_names[IDL_NODE_TYPE(name)]);
    break;
  }
  fprintf(ci->fh, ";\n");
}

static void ch_output_interface(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_type_struct(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_type_enum(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_type_dcl(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_native(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_type_union(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_codefrag(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_output_const_dcl(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_prep(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
static void ch_type_alloc_and_tc(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci, gboolean do_alloc);

static void
ch_output_types (IDL_tree       tree,
		 OIDL_Run_Info *rinfo,
		 OIDL_C_Info   *ci)
{
	if (!tree)
		return;

	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_EXCEPT_DCL: {
		char *id;

		id = IDL_ns_ident_to_qstring (
			IDL_IDENT_TO_NS (IDL_EXCEPT_DCL (tree).ident), "_", 0);

		fprintf (ci->fh, "#undef ex_%s\n", id);
		fprintf (ci->fh, "#define ex_%s \"%s\"\n",
				id, IDL_IDENT (IDL_EXCEPT_DCL (tree).ident).repo_id);

		g_free (id);

		ch_output_type_struct (tree, rinfo, ci);
		}
		break;
	case IDLN_FORWARD_DCL:
	case IDLN_INTERFACE:
      /* ch_output_interface (tree, rinfo, ci); */
		break;
	case IDLN_TYPE_STRUCT:
		ch_output_type_struct (tree, rinfo, ci);
		break;
	case IDLN_TYPE_ENUM:
		ch_output_type_enum (tree, rinfo, ci);
		break;
	case IDLN_TYPE_DCL:
		ch_output_type_dcl (tree, rinfo, ci);
		break;
	case IDLN_TYPE_UNION:
		ch_output_type_union (tree, rinfo, ci);
		break;
	case IDLN_CODEFRAG:
		ch_output_codefrag (tree, rinfo, ci);
		break;
	case IDLN_SRCFILE: {
		if (rinfo->onlytop) {
			char *idlfn = IDL_SRCFILE (tree).filename;

			if (!IDL_SRCFILE (tree).seenCnt &&
			    !IDL_SRCFILE(tree).isTop    &&
			    !IDL_SRCFILE(tree).wasInhibit) {
				gchar *hfn, *htail;

				hfn   = g_path_get_basename (idlfn);
				htail = strrchr (hfn,'.');

				g_assert (htail && strlen (htail) >= 2);

				htail [1] = 'h';
				htail [2] = 0;

				fprintf (ci->fh, "#include \"%s\"\n", hfn);

				g_free (hfn);
			}

		fprintf (ci->fh, "/* from IDL source file \"%s\" "
				 "(seen %d, isTop %d, wasInhibit %d) */ \n", 
					idlfn,
					IDL_SRCFILE (tree).seenCnt,
					IDL_SRCFILE (tree).isTop,
					IDL_SRCFILE (tree).wasInhibit);
		}
		}
		break;
	case IDLN_CONST_DCL:
		ch_output_const_dcl (tree, rinfo, ci);
		break;
	case IDLN_NATIVE:
		ch_output_native (tree, rinfo, ci);
		break;
	default:
		break;
	}

	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_MODULE:
		ch_output_types (IDL_MODULE (tree).definition_list, rinfo, ci);
		break;
	case IDLN_LIST: {
		IDL_tree sub;

		for (sub = tree; sub; sub = IDL_LIST (sub).next) {
			ch_output_types (IDL_LIST (sub).data, rinfo, ci);
		}
		}
		break;
	case IDLN_INTERFACE:
		ch_output_types (IDL_INTERFACE (tree).body, rinfo, ci);
		break;
	default:
		break;
	}
}

/*
  This function declares the class struct and the pointer on the class.
 */
static void
ch_VoyagerOutputClassDeclaration(IDL_tree tree,
                                 OIDL_Run_Info *rinfo,
                                 OIDL_C_Info   *ci)
{
	if (!tree)
		return;

	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_EXCEPT_DCL:
		break;
	case IDLN_FORWARD_DCL:
	case IDLN_INTERFACE:
		ch_output_interface (tree, rinfo, ci);
		break;
	case IDLN_TYPE_STRUCT:
		break;
	case IDLN_TYPE_ENUM:
		break;
	case IDLN_TYPE_DCL:
		break;
	case IDLN_TYPE_UNION:
		break;
	case IDLN_CODEFRAG:
		break;
	case IDLN_SRCFILE:
		break;
	case IDLN_CONST_DCL:
		break;
	case IDLN_NATIVE:
		break;
	default:
		break;
	}

	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_MODULE:
		break;
	case IDLN_LIST: {
		IDL_tree sub;

		for (sub = tree; sub; sub = IDL_LIST (sub).next) {
			ch_VoyagerOutputClassDeclaration (IDL_LIST (sub).data, rinfo, ci);
		}
		}
		break;
	case IDLN_INTERFACE:
		break;
	default:
		break;
	}
}


static void
ch_output_interface(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
    char *fullname;
    fullname = orbit_cbe_get_typespec_str(tree);
    fprintf(ci->fh,"/* %s, %s line %d */\n", __FILE__, __FUNCTION__, __LINE__);
    fprintf(ci->fh, "#if !defined(ORBIT_DECL_%s) && !defined(_%s_defined)\n#define ORBIT_DECL_%s 1\n#define _%s_defined 1\n",
            fullname, fullname, fullname, fullname);

#ifdef USE_LIBIDL_CODE
    if ( tree->declspec & IDLF_DECLSPEC_PIDL ) {
      /* PIDL interfaces are not normal CORBA Objects */
      fprintf(ci->fh, "typedef struct %s_type *%s;\n", fullname, fullname);
      fprintf(ci->fh, "#ifndef TC_%s\n", fullname);
      fprintf(ci->fh, "#  define TC_%s TC_CORBA_Object\n", fullname);
      fprintf(ci->fh, "#endif\n");
    } else {
      fprintf(ci->fh, "#define %s__freekids CORBA_Object__freekids\n", fullname);
      
      fprintf(ci->fh, "typedef CORBA_Object %s;\n\n", fullname);
      fprintf(ci->fh, "extern CORBA_unsigned_long %s__classid;\n", fullname);
      ch_type_alloc_and_tc(tree, rinfo, ci, FALSE);
    }
#else
    fprintf(ci->fh, "#ifndef %s\n", fullname);
#if 0
    /* For being more typesave when calling methods */
    fprintf(ci->fh, "typedef struct %s_struct {\n", fullname);
    fprintf(ci->fh, "  struct nomMethodTabStruct  *mtab;\n");
    fprintf(ci->fh, "  gulong body[1];\n");
    fprintf(ci->fh, "} %sObj;\n", fullname);

    fprintf(ci->fh, "#define %s %sObj\n", fullname, fullname);
    fprintf(ci->fh, "typedef %s *P%s;\n", fullname, fullname);
#endif
    fprintf(ci->fh, "#define %s NOMObject\n", fullname);
    fprintf(ci->fh, "typedef %s *P%s;\n", fullname, fullname);
    fprintf(ci->fh, "#endif\n");
#endif

    fprintf(ci->fh, "#endif\n\n");
    g_free(fullname);
}

static void
ch_output_type_enum (IDL_tree       tree,
		     OIDL_Run_Info *rinfo,
		     OIDL_C_Info   *ci)
{
	IDL_tree  l;
	char     *enumid;

	/* CORBA spec says to do
	 * typedef unsigned int enum_name;
	 * and then #defines for each enumerator.
	 * This works just as well and seems cleaner.
	 */

	enumid = IDL_ns_ident_to_qstring (
			IDL_IDENT_TO_NS (IDL_TYPE_ENUM (tree).ident), "_", 0);
	fprintf (ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n", enumid, enumid);
	fprintf (ci->fh, "typedef enum {\n");

	for (l = IDL_TYPE_ENUM (tree).enumerator_list; l; l = IDL_LIST (l).next) {
		char *id;

		id = IDL_ns_ident_to_qstring (
			IDL_IDENT_TO_NS (IDL_LIST (l).data), "_", 0);

		fprintf (ci->fh, "  %s%s\n", id, IDL_LIST (l).next ? "," : "");

		g_free (id);
	}

	fprintf (ci->fh, "} %s;\n", enumid);

	ch_type_alloc_and_tc (tree, rinfo, ci, FALSE);

	fprintf (ci->fh, "#endif\n");

	g_free (enumid);
}

static void
ch_output_type_dcl(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
	IDL_tree  l;

	ch_prep (IDL_TYPE_DCL (tree).type_spec, rinfo, ci);

	for (l = IDL_TYPE_DCL (tree).dcls; l; l = IDL_LIST (l).next) {
		char *ctmp = NULL;

		IDL_tree ent = IDL_LIST (l).data;

		switch (IDL_NODE_TYPE(ent)) {
		case IDLN_IDENT:
			ctmp = IDL_ns_ident_to_qstring (
					IDL_IDENT_TO_NS (ent), "_", 0);
			break;
		case IDLN_TYPE_ARRAY:
			ctmp = IDL_ns_ident_to_qstring (
					IDL_IDENT_TO_NS (IDL_TYPE_ARRAY (ent).ident), "_", 0);
			break;
		default:
			g_assert_not_reached ();
			break;
		}

		fprintf (ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n", ctmp, ctmp);
		fprintf (ci->fh, "typedef ");
		orbit_cbe_write_typespec (ci->fh, IDL_TYPE_DCL (tree).type_spec);

		switch (IDL_NODE_TYPE (ent)) {
		case IDLN_IDENT:
			fprintf (ci->fh, " %s;\n", ctmp);
			fprintf (ci->fh, "#define %s_marshal(x,y,z) ", ctmp);
			orbit_cbe_write_typespec (ci->fh, IDL_TYPE_DCL (tree).type_spec);
			fprintf (ci->fh, "_marshal((x),(y),(z))\n");

			fprintf (ci->fh, "#define %s_demarshal(x,y,z,i) ", ctmp);
			orbit_cbe_write_typespec (ci->fh, IDL_TYPE_DCL (tree).type_spec);
			fprintf (ci->fh, "_demarshal((x),(y),(z),(i))\n");
			break;
		case IDLN_TYPE_ARRAY: {
			IDL_tree sub;

			fprintf (ci->fh, " %s", ctmp);
			for (sub = IDL_TYPE_ARRAY (ent).size_list; sub; sub = IDL_LIST (sub).next)
				fprintf (ci->fh, "[%" IDL_LL "d]",
					 IDL_INTEGER (IDL_LIST (sub).data).value);

			fprintf (ci->fh, ";\n");
			fprintf (ci->fh, "typedef ");
			orbit_cbe_write_typespec (ci->fh, IDL_TYPE_DCL (tree).type_spec);
			fprintf (ci->fh, " %s_slice", ctmp);
			for (sub = IDL_LIST (IDL_TYPE_ARRAY (ent).size_list).next;
			     sub; sub = IDL_LIST (sub).next)
				fprintf (ci->fh, "[%" IDL_LL "d]", IDL_INTEGER (IDL_LIST (sub).data).value);
			fprintf(ci->fh, ";\n");
			}
			break;
		default:
			break;
		}

		ch_type_alloc_and_tc (ent, rinfo, ci, TRUE);
		fprintf (ci->fh, "#endif\n");
		g_free (ctmp);
	}
}

static void
ch_output_native(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
#ifdef USE_LIBIDL_CODE
    char *ctmp;
    IDL_tree id = IDL_NATIVE(tree).ident;
    ctmp = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(id), "_", 0);
    fprintf(ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n", ctmp, ctmp);
    fprintf(ci->fh, "typedef struct %s_type *%s;\n", ctmp, ctmp);
    /* Dont even think about emitting a typecode. */
    fprintf(ci->fh, "#endif\n");
    g_free(ctmp);
#endif
}

static void
ch_output_type_struct(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  char *id;
  IDL_tree cur, curmem;

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_TYPE_STRUCT(tree).ident), 
    "_", 0);
  fprintf(ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n", id, id);
  /* put typedef out first for recursive seq */
  fprintf(ci->fh, "typedef struct %s_type %s;\n", id, id);

  /* Scan for any nested decls */
  for(cur = IDL_TYPE_STRUCT(tree).member_list; cur; cur = IDL_LIST(cur).next) {
    IDL_tree ts;
    ts = IDL_MEMBER(IDL_LIST(cur).data).type_spec;
    ch_prep(ts, rinfo, ci);
  }

  fprintf(ci->fh, "struct %s_type {\n", id);

  for(cur = IDL_TYPE_STRUCT(tree).member_list; cur; cur = IDL_LIST(cur).next) {
    for(curmem = IDL_MEMBER(IDL_LIST(cur).data).dcls; curmem; curmem = IDL_LIST(curmem).next) {
      ch_output_var(IDL_MEMBER(IDL_LIST(cur).data).type_spec, IDL_LIST(curmem).data, ci);
    }
  }
  if(!IDL_TYPE_STRUCT(tree).member_list)
    fprintf(ci->fh, "int dummy;\n");
  fprintf(ci->fh, "};\n\n");

  ch_type_alloc_and_tc(tree, rinfo, ci, TRUE);

  fprintf(ci->fh, "#endif\n");

  g_free(id);
}


static void
ch_output_type_union(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  char *id;
  IDL_tree curitem;

  if (IDL_NODE_TYPE (IDL_TYPE_UNION (tree).switch_type_spec) == IDLN_TYPE_ENUM)
    ch_output_type_enum (IDL_TYPE_UNION (tree).switch_type_spec, rinfo, ci);

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_TYPE_UNION(tree).ident), "_", 0);
  fprintf(ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n", id, id);
  fprintf(ci->fh, "typedef struct %s_type %s;\n", id, id);

  /* Scan for any nested decls */
  for(curitem = IDL_TYPE_UNION(tree).switch_body; curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree member = IDL_CASE_STMT(IDL_LIST(curitem).data).element_spec;
    ch_prep(IDL_MEMBER(member).type_spec, rinfo, ci);
  }

  fprintf(ci->fh, "struct %s_type {\n", id);
  orbit_cbe_write_typespec(ci->fh, IDL_TYPE_UNION(tree).switch_type_spec);
  fprintf(ci->fh, " _d;\nunion {\n");

  for(curitem = IDL_TYPE_UNION(tree).switch_body; curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree member;

    member = IDL_CASE_STMT(IDL_LIST(curitem).data).element_spec;
    ch_output_var(IDL_MEMBER(member).type_spec,
		  IDL_LIST(IDL_MEMBER(member).dcls).data,
		  ci);
  }

  fprintf(ci->fh, "} _u;\n};\n");

  ch_type_alloc_and_tc(tree, rinfo, ci, TRUE);

  fprintf(ci->fh, "#endif\n");

  g_free(id);
}

static void
ch_output_codefrag(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  GSList *list;

  for(list = IDL_CODEFRAG(tree).lines; list;
      list = g_slist_next(list)) {
    if(!strncmp(list->data,
		"#pragma include_defs",
		sizeof("#pragma include_defs")-1)) {
	char *ctmp, *cte;
	ctmp = ((char *)list->data) + sizeof("#pragma include_defs");
	while(*ctmp && (isspace((int)*ctmp) || *ctmp == '"')) ctmp++;
	cte = ctmp;
	while(*cte && !isspace((int)*cte) && *cte != '"') cte++;
	*cte = '\0';
      fprintf(ci->fh, "#include <%s>\n", ctmp);
    } else
      fprintf(ci->fh, "%s\n", (char *)list->data);
  }
}

#if 0
static gchar* mySkipSpaces(char*  theString)
{
  if(!theString) /* Huh???*/
    return "";

  while(*theString)
    theString++;

  return theString
}
#endif

#include <string.h>
/*
  This function extracts the return vlaue from the string which is created by
  the parameter check macro. The string is of the form:

  "retval, parm1, parm2,..."

 */
static gchar* VoyagerExtractRetValFromParmCheck(gchar* theString)
{
  gchar* ptr, *ptr2;

  if(NULL==(ptr=strchr(theString, ',')))
    {
      /* No ',' so no parameters provided */
      return g_strdup(theString);
    }
  *ptr='\0';
  ptr2=g_strdup(theString);
  *ptr=',';
  return ptr2;
};

/*
  This functions puts constants into the header file. For Voyager we
  use constants to encode some special information e.g. the metaclass name
  or info about which parameters to check for methods. This information is
  marked by special strings in the constants name.
 */
static void
ch_output_const_dcl(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
	char    *id;
	IDL_tree ident;
	IDL_tree typespec;
    gchar *ptr;

	ident = IDL_CONST_DCL (tree).ident;
	id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS (ident), "_", 0);

    /* Enable parameter check for a method. This is done by putting a #define xxx
       at the top of the file. Checking code is sourrounded by #ifdef/#endif. */
    ptr=strstr( id , NOM_PARMCHECK_STRING);
    if(ptr)
      {
        gchar *retVal;
        *ptr='\0';
        //printf(" %d    --- > %s %s Params: %s\n",
        //__LINE__, id, IDL_IDENT(ident).str, IDL_STRING(IDL_CONST_DCL(ski->tree).const_exp).value);
        /* Print out the line for easy finding in this source file */
        fprintf(ci->fh, "/* %s: %s line %d */\n", __FILE__, __FUNCTION__, __LINE__);
        fprintf(ci->fh, "/* Value: %s */\n", IDL_STRING(IDL_CONST_DCL(tree).const_exp).value);
        /* Protect by #ifdef...*/
        fprintf(ci->fh, "#ifndef %s_ParmCheck_h\n", id);
        fprintf(ci->fh, "#define %s_ParmCheck_h\n", id);
        /* Default return code (provided by the macro) */
        retVal=VoyagerExtractRetValFromParmCheck(IDL_STRING(IDL_CONST_DCL(tree).const_exp).value);
        fprintf(ci->fh, "#define %s_retval %s\n", id, retVal);
        g_free(retVal);
        fprintf(ci->fh, "#endif /* !%s */\n\n", id);
        *ptr='_';
      }
    else
      {
        fprintf(ci->fh, "#ifndef %s\n", id);
        fprintf(ci->fh, "#define %s ", id);
        
        orbit_cbe_write_const(ci->fh,
                              IDL_CONST_DCL(tree).const_exp);
        
        typespec = orbit_cbe_get_typespec (IDL_CONST_DCL(tree).const_type);
        if (IDL_NODE_TYPE (typespec) == IDLN_TYPE_INTEGER &&
            !IDL_TYPE_INTEGER (typespec).f_signed)
          fprintf(ci->fh, "U");
        
        fprintf(ci->fh, "\n");
        fprintf(ci->fh, "#endif /* !%s */\n\n", id);
      }

    /* Get the name of our explicit metaclass if any. */
    if(IDLN_STRING==IDL_NODE_TYPE(IDL_CONST_DCL(tree).const_exp))
      {
        /* Our metaclass info is a string */
        if(strstr( IDL_IDENT(ident).str, NOM_METACLASS_STRING))
          {
            gsMetaClassName=g_string_new(NULL);
            g_string_printf(gsMetaClassName, "%s", IDL_STRING(IDL_CONST_DCL(tree).const_exp).value);
            //printf(" %d    --- > %s %s (%x)\n",
            //__LINE__, id, IDL_STRING(IDL_CONST_DCL(ski->tree).const_exp).value, gsMetaClassName[ulCurInterface]);
          }
       }
	g_free(id);
}

static void
ch_prep_fixed(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  char *ctmp;

  ctmp = orbit_cbe_get_typespec_str(tree);
  fprintf(ci->fh,
	  "typedef struct { CORBA_unsigned_short _digits; CORBA_short _scale; CORBA_char _value[%d]; } %s;\n",
	  (int) (IDL_INTEGER(IDL_TYPE_FIXED(tree).positive_int_const).value + 2)/2,
	  ctmp);
  g_free(ctmp);

  ch_type_alloc_and_tc(tree, rinfo, ci, TRUE);
}

static void
ch_prep_sequence(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  char *ctmp, *fullname, *fullname_def, *ctmp2;
  IDL_tree tts;
  gboolean separate_defs, fake_if;
  IDL_tree fake_seq = NULL;

  tts = orbit_cbe_get_typespec(IDL_TYPE_SEQUENCE(tree).simple_type_spec);
  ctmp = orbit_cbe_get_typespec_str(IDL_TYPE_SEQUENCE(tree).simple_type_spec);
  ctmp2 = orbit_cbe_get_typespec_str(tts);
  fake_if = (IDL_NODE_TYPE(tts) == IDLN_INTERFACE);
  if(fake_if)
    {
      g_free(ctmp2);
      ctmp2 = g_strdup("CORBA_Object");
    }
  separate_defs = strcmp(ctmp, ctmp2);
  fullname = orbit_cbe_get_typespec_str(tree);

  if(separate_defs)
    {
      if(fake_if)
	tts = IDL_type_object_new();
      fake_seq = IDL_type_sequence_new(tts, NULL);
      IDL_NODE_UP(fake_seq) = IDL_NODE_UP(tree);
      ch_prep_sequence(fake_seq, rinfo, ci);
      fullname_def = g_strdup_printf("CORBA_sequence_%s", ctmp2);
      if(!fake_if)
	IDL_TYPE_SEQUENCE(fake_seq).simple_type_spec = NULL;
    }
  else
    fullname_def = g_strdup(fullname);

  if(IDL_NODE_TYPE(IDL_TYPE_SEQUENCE(tree).simple_type_spec)
     == IDLN_TYPE_SEQUENCE)
    ch_prep_sequence(IDL_TYPE_SEQUENCE(tree).simple_type_spec, rinfo, ci);

  /* NOTE: ORBIT_DECL_%s protects redef of everything (struct,TC,externs)
   * while _%s_defined protects only the struct */

  fprintf(ci->fh, "#if !defined(ORBIT_DECL_%s)\n#define ORBIT_DECL_%s 1\n",
	  fullname, fullname);
  if ( ci->do_impl_hack )
      orbit_cbe_id_define_hack(ci->fh, "ORBIT_IMPL", fullname, ci->c_base_name);

  if(separate_defs)
    {
      fprintf(ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n",
	      fullname, fullname);
      if(!strcmp(ctmp, "CORBA_RepositoryId"))
	fprintf(ci->fh, "/* CRACKHEADS */\n");
      fprintf(ci->fh, "typedef %s %s;\n", fullname_def, fullname);
      fprintf(ci->fh, "#endif\n");
      ch_type_alloc_and_tc(tree, rinfo, ci, FALSE);
      fprintf(ci->fh, "#define %s__alloc %s__alloc\n",
	      fullname, fullname_def);
      fprintf(ci->fh, "#define %s__freekids %s__freekids\n",
	      fullname, fullname_def);
      fprintf(ci->fh, "#define CORBA_sequence_%s_allocbuf CORBA_sequence_%s_allocbuf\n",
	      ctmp, ctmp2);
      fprintf(ci->fh, "#define %s_marshal(x,y,z) %s_marshal((x),(y),(z))\n", fullname, fullname_def);

      fprintf(ci->fh, "#define %s_demarshal(x,y,z,i) %s_demarshal((x),(y),(z),(i))\n", fullname, fullname_def);
      IDL_tree_free(fake_seq);
    }
  else
    {
      char *tc, *member_type;

      fprintf(ci->fh, "#if !defined(_%s_defined)\n#define _%s_defined 1\n",
	      fullname, fullname);
      fprintf(ci->fh, "typedef struct { CORBA_unsigned_long _maximum, _length; ");
      orbit_cbe_write_typespec(ci->fh, IDL_TYPE_SEQUENCE(tree).simple_type_spec);
      fprintf(ci->fh, "* _buffer; CORBA_boolean _release; } ");
      orbit_cbe_write_typespec(ci->fh, tree);
      fprintf(ci->fh, ";\n#endif\n");
      ch_type_alloc_and_tc(tree, rinfo, ci, TRUE);

      tc = orbit_cbe_get_typecode_name (orbit_cbe_get_typespec (tree));
      member_type = orbit_cbe_type_is_builtin (IDL_TYPE_SEQUENCE (tree).simple_type_spec) ?
				ctmp + strlen ("CORBA_") : ctmp;

      fprintf (ci->fh, "#define CORBA_sequence_%s_allocbuf(l) "
		       "((%s*)ORBit_small_allocbuf (%s, (l)))\n",
		       member_type, member_type, tc);

      g_free (tc);
    }

  fprintf(ci->fh, "#endif\n");

  g_free(ctmp2);
  g_free(ctmp);
  g_free(fullname);
  g_free(fullname_def);
}

static
void ch_prep (IDL_tree       tree,
	      OIDL_Run_Info *rinfo,
	      OIDL_C_Info   *ci)
{
	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_TYPE_SEQUENCE:
		ch_prep_sequence (tree, rinfo, ci);
		break;
	case IDLN_TYPE_FIXED:
		ch_prep_fixed (tree, rinfo, ci);
		break;
	case IDLN_TYPE_STRUCT:
		ch_output_type_struct (tree, rinfo, ci);
		break;
	case IDLN_TYPE_ENUM:
		ch_output_type_enum (tree, rinfo, ci);
		break;
	default:
		break;
	}
}

static void
ch_type_alloc_and_tc(IDL_tree tree, OIDL_Run_Info *rinfo,
		     OIDL_C_Info *ci, gboolean do_alloc)
{
  char *ctmp;
  IDL_tree tts;

  ctmp = orbit_cbe_get_typespec_str(tree);

  if ( ci->do_impl_hack ) {
      fprintf(ci->fh, "#if !defined(TC_IMPL_TC_%s_0)\n", ctmp);
      orbit_cbe_id_define_hack(ci->fh, "TC_IMPL_TC", ctmp, ci->c_base_name);
  }

  fprintf (ci->fh, "#ifdef ORBIT_IDL_C_IMODULE_%s\n", ci->c_base_name);
  fprintf (ci->fh, "static\n");
  fprintf (ci->fh, "#else\n");
  fprintf (ci->fh, "extern\n");
  fprintf (ci->fh, "#endif\n");
  fprintf (ci->fh, "ORBIT2_MAYBE_CONST struct CORBA_TypeCode_struct TC_%s_struct;\n", ctmp);

  fprintf (ci->fh, "#define TC_%s ((CORBA_TypeCode)&TC_%s_struct)\n", ctmp, ctmp);
  if (ci->do_impl_hack)
      fprintf (ci->fh, "#endif\n");

  if(do_alloc) {
      char *tc;

      tts = orbit_cbe_get_typespec(tree);

      tc = orbit_cbe_get_typecode_name (tts);

      fprintf (ci->fh, "#define %s__alloc() ((%s%s *)ORBit_small_alloc (%s))\n",
		   ctmp, ctmp, (IDL_NODE_TYPE(tree) == IDLN_TYPE_ARRAY)?"_slice":"", tc);

      fprintf (ci->fh, "#define %s__freekids(m,d) ORBit_small_freekids (%s,(m),(d))\n", ctmp, tc);

      if ( IDL_NODE_TYPE(tts) == IDLN_TYPE_SEQUENCE )
      {
	char *member_type = orbit_cbe_get_typespec_str(IDL_TYPE_SEQUENCE(tts).simple_type_spec);
	char *member_name = orbit_cbe_type_is_builtin (IDL_TYPE_SEQUENCE (tts).simple_type_spec) ?
	  member_type + strlen ("CORBA_") : member_type;
	
	fprintf (ci->fh, "#define %s_allocbuf(l) "
		 "((%s*)ORBit_small_allocbuf (%s, (l)))\n",
		 ctmp, member_name, tc);
	
	g_free (member_type);
      }
	
      g_free (tc);
  }

  g_free(ctmp);
}

/************************/
#ifdef USE_LIBIDL_CODE
static void
cbe_header_interface_print_vepv(IDL_tree node, FILE *of)
{
  char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(node).ident),
				     "_", 0);
  fprintf(of, "  POA_%s__epv *%s_epv;\n", id, id);
  g_free(id);

}

static void
ch_output_poa(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  if(!tree) return;

  if ( tree->declspec & IDLF_DECLSPEC_PIDL )
	return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_MODULE:
    ch_output_poa(IDL_MODULE(tree).definition_list, rinfo, ci);
    break;
  case IDLN_LIST:
    {
      IDL_tree sub;

      for(sub = tree; sub; sub = IDL_LIST(sub).next) {
	ch_output_poa(IDL_LIST(sub).data, rinfo, ci);
      }
    }
    break;
  case IDLN_INTERFACE:
    {
      IDL_tree sub;
      char *id;


      id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(tree).ident), "_", 0);

      fprintf(ci->fh, "#ifndef _defined_POA_%s\n#define _defined_POA_%s 1\n", 
        id, id);

      /* First, do epv for this interface, then vepv, then servant */
      fprintf(ci->fh, "typedef struct {\n");
      fprintf(ci->fh, "  void *_private;\n");
      for(sub = IDL_INTERFACE(tree).body; sub; sub = IDL_LIST(sub).next) {
	IDL_tree cur;

	cur = IDL_LIST(sub).data;

	switch(IDL_NODE_TYPE(cur)) {
	case IDLN_OP_DCL:
	  orbit_cbe_op_write_proto(ci->fh, cur, "", TRUE);
	  fprintf(ci->fh, ";\n");
	  break;
	case IDLN_ATTR_DCL:
	  {
	    OIDL_Attr_Info *ai;
	    IDL_tree curitem;

	    for(curitem = IDL_ATTR_DCL(cur).simple_declarations; curitem; curitem = IDL_LIST(curitem).next) {
	      ai = IDL_LIST(curitem).data->data;
	      
	      orbit_cbe_op_write_proto(ci->fh, ai->op1, "", TRUE);
	      fprintf(ci->fh, ";\n");
	      
	      if(ai->op2) {
		orbit_cbe_op_write_proto(ci->fh, ai->op2, "", TRUE);
		fprintf(ci->fh, ";\n");
	      }
	    }
	  }
	  break;
	default:
	  break;
	}
      }

      fprintf(ci->fh, "} POA_%s__epv;\n", id);

      fprintf(ci->fh, "typedef struct {\n");
      fprintf(ci->fh, "  PortableServer_ServantBase__epv *_base_epv;\n");
      IDL_tree_traverse_parents(tree, (GFunc)cbe_header_interface_print_vepv, ci->fh);
      fprintf(ci->fh, "} POA_%s__vepv;\n", id);

      fprintf(ci->fh, "typedef struct {\n");
      fprintf(ci->fh, "  void *_private;\n");
      fprintf(ci->fh, "  POA_%s__vepv *vepv;\n", id);
      fprintf(ci->fh, "} POA_%s;\n", id);

      fprintf(ci->fh,
	      "extern void POA_%s__init(PortableServer_Servant servant, CORBA_Environment *ev);\n", id);
      fprintf(ci->fh,
	      "extern void POA_%s__fini(PortableServer_Servant servant, CORBA_Environment *ev);\n", id);

      fprintf(ci->fh, "#endif /* _defined_POA_%s */\n", id);

      g_free(id);
    }
    break;
  default:
    break;
  }
}
#endif

/************************/
typedef struct {
  FILE *of;
  IDL_tree realif;
  char* chrOverridenMethodName;
} InheritedOutputInfo;
static void ch_output_inherited_protos(IDL_tree curif, InheritedOutputInfo *ioi);

static void
VoyagerOutputClassDataStructMember(FILE  *of, IDL_tree op, const char *nom_prefix)
{
  g_assert (IDL_NODE_TYPE(op) == IDLN_OP_DCL);

  fprintf (of, "    nomMToken %s", IDL_IDENT (IDL_OP_DCL (op).ident).str);
}

static void
VoyagerOutputClassDataStructAttributeMember (FILE       *of,
                                             IDL_tree    op,
                                             const char *nom_prefix,
                                             gboolean    for_epv)
{
#ifdef USE_LIBIDL_CODE
  IDL_tree  sub;
#endif
  char     *id;
  
  g_assert (IDL_NODE_TYPE(op) == IDLN_OP_DCL);

  
#ifdef USE_LIBIDL_CODE
  /* This is used for writing the return type */
  orbit_cbe_write_param_typespec (of, op);
#endif

  id = IDL_ns_ident_to_qstring (
                                IDL_IDENT_TO_NS (IDL_INTERFACE (
                                IDL_get_parent_node (op, IDLN_INTERFACE, NULL)).ident), "_", 0);
  
#ifdef USE_LIBIDL_CODE
  if (for_epv)
    fprintf (of, " (*%s%s)", nom_prefix ? nom_prefix : "",
			 IDL_IDENT(IDL_OP_DCL(op).ident).str);
	else 
		fprintf (of, " %s%s_%s", nom_prefix ? nom_prefix : "",
			 id, IDL_IDENT (IDL_OP_DCL (op).ident).str);
#else
  /* We support special instance vars which are not known to the outside
     (in contrast to attributes which have _set() and _get() methods). 
     These instance vars are specially marked attributes in the IDL file
     which are processed by a macro. This macro adds the string 
     __INSTANCEVAR__ to the attribute name. For these attributes
     we do not add methods to the class here. */
  if(!strstr(IDL_IDENT (IDL_OP_DCL (op).ident).str, "__INSTANCEVAR__"))
    if(!for_epv)
      fprintf (of, "    _%s",  IDL_IDENT (IDL_OP_DCL (op).ident).str);
#endif

#ifdef USE_LIBIDL_CODE
	fprintf (of, "(");

	if (for_epv)
		fprintf (of, "PortableServer_Servant _servant, ");
	else
		fprintf (of, "%s _obj, ", id);
#endif

	g_free (id);

#ifdef USE_LIBIDL_CODE
    /* Do params... */
	for (sub = IDL_OP_DCL (op).parameter_dcls; sub; sub = IDL_LIST (sub).next) {
		IDL_tree parm = IDL_LIST (sub).data;

		orbit_cbe_write_param_typespec (of, parm);

		fprintf (of, " %s, ", IDL_IDENT (IDL_PARAM_DCL (parm).simple_declarator).str);
	}

	if (IDL_OP_DCL (op).context_expr)
		fprintf (of, "CORBA_Context _ctx, ");
	fprintf (of, "CORBA_Environment *ev)");
#endif
}


/*
  This function puts out something like the following in the *.h file:

  #define WPFolder_wpEchoString(nomSelf, a, b, ...) \
          WPObject_wpEchoString((WPObject*) nomSelf, a, b, ...)
*/
static void
VoyagerOutputOverridenMethod(IDL_tree curif, InheritedOutputInfo *ioi)
{
  char *id, *realid;
  IDL_tree curitem;
  char* overridenMethodName;

  if(curif == ioi->realif)
    return;

  overridenMethodName=ioi->chrOverridenMethodName;

  realid = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ioi->realif).ident), "_", 0);
  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(curif).ident), "_", 0);

  for(curitem = IDL_INTERFACE(curif).body; curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree curop = IDL_LIST(curitem).data;

    switch(IDL_NODE_TYPE(curop)) {
    case IDLN_OP_DCL:
      {
        /* Check if the current method (introduced by some parent) is the one to be
           overriden. */
        if(!strcmp(overridenMethodName, IDL_IDENT(IDL_OP_DCL(curop).ident).str)){
          IDL_tree  sub;

          fprintf(ioi->of, "#define %s_%s(nomSelf, ",
                  realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str);
          for (sub = IDL_OP_DCL (curop).parameter_dcls; sub; sub = IDL_LIST (sub).next) {
            IDL_tree parm = IDL_LIST (sub).data;
            fprintf (ioi->of, "%s, ", IDL_IDENT (IDL_PARAM_DCL (parm).simple_declarator).str);
          }
          fprintf(ioi->of, "ev) \\\n");
          fprintf(ioi->of, "        %s_%s((%s*) nomSelf, ",
                  id, IDL_IDENT(IDL_OP_DCL(curop).ident).str, id);
          for (sub = IDL_OP_DCL (curop).parameter_dcls; sub; sub = IDL_LIST (sub).next) {
            IDL_tree parm = IDL_LIST (sub).data;
            fprintf (ioi->of, "%s, ", IDL_IDENT (IDL_PARAM_DCL (parm).simple_declarator).str);
          }
          fprintf(ioi->of, "ev)\n");
        }
        break;
      }
	default:
	  break;
    }
  }

  g_free(id);
  g_free(realid);
}

/*
  Output the typespecs and parameters of a new method.
  Note that tree must be correctly chosen.
 */
static void
doWriteTypespecAndParameters(FILE  *of, IDL_tree tree)
{
  IDL_tree curitem;

  for(curitem = IDL_OP_DCL(tree).parameter_dcls;
      curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree tr;
    tr = IDL_LIST(curitem).data;      
    /*  Write list of params */
    if(IDL_NODE_TYPE(tr) == IDLN_PARAM_DCL)
      {
        fprintf(of, "        ");
        orbit_cbe_write_param_typespec(of, tr);
        fprintf(of, " %s,\n", IDL_IDENT(IDL_PARAM_DCL(tr).simple_declarator).str);
      }
  }
}

/*
  Output the parameters only of a new method.
  Note that tree must be correctly chosen.
 */
static void
doWriteParametersOnly(FILE  *of, IDL_tree tree)
{
  IDL_tree curitem;

  for(curitem = IDL_OP_DCL(tree).parameter_dcls;
      curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree tr;
    tr = IDL_LIST(curitem).data;      
    /*  Write list of params */
    if(IDL_NODE_TYPE(tr) == IDLN_PARAM_DCL)
      {
        fprintf(of, " %s,", IDL_IDENT(IDL_PARAM_DCL(tr).simple_declarator).str);
      }
  }
}

/*
  This function does quite some real work. It puts all the info about an introduced method
  into the *.h file. This includes convenience macros, strings specifying the method name
  and macros for automatic checking of parameters.
 */
static void
VoyagerOutputNewMethod(FILE  *of, IDL_tree tree, const char *nom_prefix)
{
	/* If you fix anything here, please also fix it in
	   cbe_ski_do_inherited_op_dcl(), which is almost a
	   cut-and-paste of this routine */

	char *id, *id2, *id3;
	IDL_tree curitem, op;
	int level;

    g_assert (IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

    curitem = IDL_get_parent_node(tree, IDLN_INTERFACE, &level);
    g_assert(curitem);
    
    id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_OP_DCL(tree).ident), "_", 0);    
    id2 = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(curitem).ident), "_", 0);
    id3 = IDL_IDENT (IDL_OP_DCL (tree).ident).str;

    fprintf(of, "/* %s, %s line %d */\n", __FILE__, __FUNCTION__, __LINE__);
    /* protect with #ifdef block  */
    fprintf(of, "#if !defined(_decl_");
    fprintf(of, "%s_)\n", id);
    
    fprintf(of, "#define _decl_");
    fprintf(of, "%s_ 1\n\n", id);
    
    fprintf(of, "typedef ");
    orbit_cbe_write_param_typespec(of, tree);
    
    fprintf(of, " NOMLINK nomTP_%s(%s *nomSelf,\n", id, id2);
        
    op = tree;
    for(curitem = IDL_OP_DCL(tree).parameter_dcls;
        curitem; curitem = IDL_LIST(curitem).next) {
      IDL_tree tr;
      tr = IDL_LIST(curitem).data;      

      /*  Write list of params */
      if(IDL_NODE_TYPE(tr) == IDLN_PARAM_DCL)
        {
          fprintf(of, "        ");
          orbit_cbe_write_param_typespec(of, tr);
          fprintf(of, " %s,\n", IDL_IDENT(IDL_PARAM_DCL(tr).simple_declarator).str);
        }
#if 0 
     subski.tree = IDL_LIST(curitem).data;      
      orbit_cbe_ski_process_piece(&subski);
#endif
    }
    tree=op;

#ifdef NOT_YET
    if(IDL_OP_DCL(op).context_expr)
      fprintf(of, "CORBA_Context ctx,\n");
#endif
    
    fprintf(of, "CORBA_Environment *ev)");
    fprintf(of, ";\n");
    
    fprintf(of, "typedef ");
    fprintf(of, "nomTP_%s *nomTD_%s;\n", id, id);

    /* define the ID for this method */
    fprintf(of, "/* define the name for this method */\n");
    fprintf(of, "#define nomMNDef_%s \"%s\"\n", id,  id3);
    fprintf(of, "#define nomMNFullDef_%s \"%s:%s\"\n", id, id2, id3);

    /* define method call as a macro */
    fprintf(of, "/* define method call as a macro */\n");
    fprintf(of, "#ifndef NOM_NO_PARAM_CHECK /* Parameter chack at all? */\n");
    fprintf(of, "#ifdef %s_ParmCheck_h /* Extended parameter check enabled */\n", id);

    /* Forward declaration of parameter test function */
    fprintf(of, "NOMEXTERN ");                
    fprintf(of, "gboolean NOMLINK parmCheckFunc_%s(%s *nomSelf, \n",id, id2);
    /* Write parameter list including the type spec */
    doWriteTypespecAndParameters(of, tree);
    fprintf(of, "CORBA_Environment *ev);\n");

    /* Macro to be used when several parameters are checked */
    fprintf(of, "#define %s(nomSelf, ", id);
    /* add the parms */
    doWriteParametersOnly(of, tree);
    fprintf(of, "ev) \\\n");
    if(!strstr(id, "_nomIsObject"))
      {
        /* No check for _nomIsObject or otherwise we have a recursion */
        fprintf(of, "        (parmCheckFunc_%s(nomSelf, ", id);
        /* Parameters for call */
        doWriteParametersOnly(of, tree);        
        fprintf(of, "ev) ? \\\n");
        fprintf(of, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", id2, id3);
        fprintf(of, "        (nomSelf,");
        /* add the parms */
        doWriteParametersOnly(of, tree);
        fprintf(of, "ev)) : %s_retval)\n", id);
      }
    else
      {
        fprintf(of, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", id2, id3);
        fprintf(of, "        (nomSelf,");
        /* add the parms */
        doWriteParametersOnly(of, tree);
        fprintf(of, "ev))\n");
      }
    /* else NOM_NO_PARAM_CHECK */
    fprintf(of, "#else /* Extended parameter check */\n");

    /* Check object only  */
    fprintf(of, "#define %s(nomSelf, ", id);
    /* add the parms */
    doWriteParametersOnly(of, tree);
    fprintf(of, "ev) \\\n");
    if(!strstr(id, "_nomIsObject"))
      {
        /* No check for _nomIsObject or otherwise we have a recursion */
        if(!strcmp(id2, "NOMObject"))
          fprintf(of, "        (nomCheckNOMObjectPtr((NOMObject*)nomSelf, %sClassData.classObject,", id2);
        else
          fprintf(of, "        (nomCheckObjectPtr((NOMObject*)nomSelf, %sClassData.classObject,", id2);
        fprintf(of, " \"%s\", ev) ? \\\n", id); /* method name */
        fprintf(of, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", id2, id3);
        fprintf(of, "        (nomSelf,");
        /* add the parms */
        doWriteParametersOnly(of, tree);
        fprintf(of, "ev)) : (");
        orbit_cbe_write_param_typespec(of, tree);
        fprintf(of, ") NULL)\n");
      }
    else
      {
        fprintf(of, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", id2, id3);
        fprintf(of, "        (nomSelf,");
        /* add the parms */
        doWriteParametersOnly(of, tree);
        fprintf(of, "ev))\n");
      }
    fprintf(of, "#endif\n");

    /* No parameter check at all */
    fprintf(of, "#else /* NOM_NO_PARAM_CHECK */\n");

    /* Normal macro */
    fprintf(of, "#define %s(nomSelf, ", id);
    doWriteParametersOnly(of, tree);
    fprintf(of, "ev) \\\n");
    fprintf(of, "        (NOM_Resolve(nomSelf, %s, %s) \\\n", id2, id3);
    fprintf(of, "        (nomSelf,");
    /* add the parms */
    doWriteParametersOnly(of, tree);
    fprintf(of, "ev))\n");
    fprintf(of, "#endif\n"); /* Parameter check*/

    /* Method macro */
    fprintf(of, "#define _%s %s", id3 , id);

    fprintf(of, "\n#endif\n\n"); /* end of protective #ifdef block */
    /* Don't dare to free id3 here ;) */
    g_free(id); g_free(id2);
    return ;
}

/* CW: probably some cleanup isn't too bad here... */
static void
ch_output_stub_protos(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_MODULE:
    ch_output_stub_protos(IDL_MODULE(tree).definition_list, rinfo, ci);
    break;
  case IDLN_LIST:
    {
      IDL_tree sub;

      for(sub = tree; sub; sub = IDL_LIST(sub).next) {
        ch_output_stub_protos(IDL_LIST(sub).data, rinfo, ci);
      }
    }
    break;
  case IDLN_INTERFACE:
    {
      /* write stubs into c header file */
      IDL_tree sub;
      char     *id;

      /* Get interface name */
      sub = IDL_INTERFACE(tree).body;
      sub = IDL_LIST(sub).data;
      id = IDL_ns_ident_to_qstring (IDL_IDENT_TO_NS (IDL_INTERFACE (IDL_get_parent_node (sub, IDLN_INTERFACE, NULL)).ident),
                                    "_", 0);

      /* Inherited stuff */
      if(IDL_INTERFACE(tree).inheritance_spec) {
        InheritedOutputInfo ioi;
        ioi.of = ci->fh;
        ioi.realif = tree;
        IDL_tree_traverse_parents(IDL_INTERFACE(tree).inheritance_spec, (GFunc)ch_output_inherited_protos, &ioi);
      }

      /* Voyager special struct */
      fprintf(ci->fh, "\n/** (%s, %s line %d) **/\n", __FILE__, __FUNCTION__, __LINE__);
      fprintf(ci->fh, "\n/** Voyager class data structure **/\n");
      fprintf(ci->fh, "NOMEXTERN struct %sClassDataStructure {\n    NOMClass *classObject;\n", 
              id);

      /* For all */
      for(sub = IDL_INTERFACE(tree).body; sub; sub = IDL_LIST(sub).next) {
        IDL_tree cur;
        
        cur = IDL_LIST(sub).data;
        
        switch(IDL_NODE_TYPE(cur)) {
        case IDLN_OP_DCL:
          orbit_idl_check_oneway_op (cur);

          /* NOM allows method overriding. This is done using a macro which builds a method
             name containing the string __OVERRIDE__. Make sure these methods are not put
             into the class structure holding introduced methods. */
          if(!strstr(IDL_IDENT (IDL_OP_DCL (cur).ident).str, "__OVERRIDE__"))
            {
              VoyagerOutputClassDataStructMember(ci->fh, cur, "");
              fprintf(ci->fh, ";\n");
            }
          break;
        case IDLN_ATTR_DCL:
          {
            OIDL_Attr_Info *ai;
            IDL_tree curitem;
            
            for(curitem = IDL_ATTR_DCL(cur).simple_declarations; curitem; curitem = IDL_LIST(curitem).next) {
              ai = IDL_LIST(curitem).data->data;
#ifdef USE_LIBIDL_CODE
              /* Get methods for attributes */
              orbit_cbe_op_write_proto(ci->fh, ai->op1, "", FALSE);
              fprintf(ci->fh, ";\n");
              /* Set method for attributes */
              if(ai->op2) {
                orbit_cbe_op_write_proto(ci->fh, ai->op2, "", FALSE);
                fprintf(ci->fh, ";\n");
              }
#else
#if 0
              /* 11.03.2007: We don't use attributes for now! */
              /* We put the attribute methods automatically into the *ClassStruct.
                 That's different to SOM where you have to put them in the releaseorder list
                 if desired. Maybe we should just ignore Attributes... */
              VoyagerOutputClassDataStructAttributeMember(ci->fh, ai->op1, "", FALSE);
              fprintf(ci->fh, ";\n");
              if(ai->op2) {
                VoyagerOutputClassDataStructAttributeMember(ci->fh, ai->op2, "", FALSE);
                fprintf(ci->fh, ";\n");
              }
#endif
#endif
            }/* for() */
          }
          break;
        default:
          break;
        }/* switch */
      }/* for */
      /* Voyager special struct */
      fprintf(ci->fh, "} %sClassData;\n\n", id);
      
      /******* Print generic object check function  *******/

      if(!strcmp(id, "NOMObject"))
        fprintf(ci->fh, "NOMEXTERN gboolean NOMLINK nomCheckNOMObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev);\n\n");
      else
        fprintf(ci->fh, "NOMEXTERN gboolean NOMLINK nomCheckObjectPtr(NOMObject *nomSelf, NOMClass* nomClass, gchar* chrMethodName, CORBA_Environment *ev);\n\n");
      
      /******* Print introduced methods *******/
      for(sub = IDL_INTERFACE(tree).body; sub; sub = IDL_LIST(sub).next) {
        IDL_tree cur;
        
        cur = IDL_LIST(sub).data;

        switch(IDL_NODE_TYPE(cur)) {
        case IDLN_OP_DCL:
          orbit_idl_check_oneway_op (cur);

          if(!strstr(IDL_IDENT (IDL_OP_DCL (cur).ident).str, "__OVERRIDE__"))
            {
              fprintf(ci->fh, "/* %s, %s line %d */\n ", __FILE__, __FUNCTION__, __LINE__);
              fprintf(ci->fh, "/*\n * NEW_METHOD: ");
              fprintf(ci->fh, "%s %s\n */\n", IDL_IDENT (IDL_OP_DCL (cur).ident).str, id );
              /* Do print... */
              VoyagerOutputNewMethod(ci->fh, cur, "");
            }
          break;
        default:
          break;
        }/* switch */
      }/* for */
      fprintf(ci->fh, "\n");
      /******* Print introduced method for possible postprocessing *******/

      /******* Output overriden methods *******/
      for(sub = IDL_INTERFACE(tree).body; sub; sub = IDL_LIST(sub).next) {
        IDL_tree cur;
        
        cur = IDL_LIST(sub).data;

        switch(IDL_NODE_TYPE(cur)) {
        case IDLN_OP_DCL:
          {
            char* ptr;
            orbit_idl_check_oneway_op (cur);
            
            if((ptr=strstr(IDL_IDENT (IDL_OP_DCL (cur).ident).str, "__OVERRIDE__"))!=NULL)
              {
                /* There's an overriden method here */
                *ptr='\0';
                fprintf(ci->fh, "/* %s, %s line %d */\n", __FILE__, __FUNCTION__, __LINE__);
                fprintf(ci->fh, "/* OVERRIDE_METHOD: ");
                fprintf(ci->fh, "%s %s */\n", IDL_IDENT (IDL_OP_DCL (cur).ident).str, id );
                fprintf(ci->fh, "#ifdef %s_%s\n", id, IDL_IDENT (IDL_OP_DCL (cur).ident).str);
                fprintf(ci->fh, "#undef %s_%s\n", id, IDL_IDENT (IDL_OP_DCL (cur).ident).str);
                fprintf(ci->fh, "#endif\n");

                /* Try to find the interface introducing this method */

                /* Inherited */
                if(IDL_INTERFACE(tree).inheritance_spec) {
                  InheritedOutputInfo ioi;
                  ioi.of = ci->fh;
                  ioi.realif = tree;
                  ioi.chrOverridenMethodName=IDL_IDENT (IDL_OP_DCL (cur).ident).str;
                  IDL_tree_traverse_parents(IDL_INTERFACE(tree).inheritance_spec,
                                            (GFunc)VoyagerOutputOverridenMethod, &ioi);
                }
                *ptr='_';
              }
            break;
          }
        default:
          break;
        }/* switch */
      }/* for */
      fprintf(ci->fh, "\n");
      /******* Output overriden methods *******/

      /* */
      g_free (id);
    }/* case */
    break;
  default:
    break;
  }
}

/*
  This function writes the params of methods. The typespec is not written.
  This function is useful for putting parameters into macros.
 */
static void
VoyagerOutputMethodParamsNoTypeSpec(FILE  *of, IDL_tree tree, const char *nom_prefix)
{
	IDL_tree curitem;

    g_assert (IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

    /* add the parms */
    for(curitem = IDL_OP_DCL(tree).parameter_dcls;
        curitem; curitem = IDL_LIST(curitem).next) {
      IDL_tree tr;

      tr = IDL_LIST(curitem).data;      
      /*  Write list of params */
      if(IDL_NODE_TYPE(tr) == IDLN_PARAM_DCL)
        {
          fprintf(of, " %s,", IDL_IDENT(IDL_PARAM_DCL(tr).simple_declarator).str);
        }
    }
    return ;
}

static void
ch_output_inherited_protos(IDL_tree curif, InheritedOutputInfo *ioi)
{
  char *id, *realid;
  IDL_tree curitem;

  if(curif == ioi->realif)
    return;

  realid = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ioi->realif).ident), "_", 0);
  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(curif).ident), "_", 0);

  for(curitem = IDL_INTERFACE(curif).body; curitem; curitem = IDL_LIST(curitem).next) {
    IDL_tree curop = IDL_LIST(curitem).data;

    switch(IDL_NODE_TYPE(curop)) {
    case IDLN_OP_DCL:
      /* Only output methods which are not overriden. Overriden methods are also
         put into the data but they're marked with __OVERRIDE__. */
      if(!strstr(IDL_IDENT (IDL_OP_DCL (curop).ident).str, "__OVERRIDE__"))
        {
          fprintf(ioi->of, "/* %s, %s line %d */\n", __FILE__, __FUNCTION__, __LINE__);
          //          fprintf(ioi->of, "#define %s_%s %s_%s\n",
          //      realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str,
          //      id, IDL_IDENT(IDL_OP_DCL(curop).ident).str);
          fprintf(ioi->of, "#define %s_%s(nomSelf,",realid, IDL_IDENT(IDL_OP_DCL(curop).ident).str);
          VoyagerOutputMethodParamsNoTypeSpec(ioi->of, curop, "");
          fprintf(ioi->of, " ev) \\\n        %s_%s((%s*) nomSelf,",
                  id, IDL_IDENT(IDL_OP_DCL(curop).ident).str, id);
          VoyagerOutputMethodParamsNoTypeSpec(ioi->of, curop, "");
          fprintf(ioi->of, " ev)\n");
        }
      break;
#if 0
      /* We don't use _Set*() and _Get*() methods with attributes in Voyager */
    case IDLN_ATTR_DCL:
      {
        IDL_tree curitem;
        
        /* We don't use OIDL_Attr_Info here because inherited ops may go back into trees that are output-inhibited
           and therefore don't have the OIDL_Attr_Info generated on them */
        
        for(curitem = IDL_ATTR_DCL(curop).simple_declarations; curitem; curitem = IDL_LIST(curitem).next) {
          IDL_tree ident;
          
          ident = IDL_LIST(curitem).data;
          
          fprintf(ioi->of, "#define %s__get_%s %s__get_%s\n",
                  realid, IDL_IDENT(ident).str,
                  id, IDL_IDENT(ident).str);
          
          if(!IDL_ATTR_DCL(curop).f_readonly)
            fprintf(ioi->of, "#define %s__set_%s %s__set_%s\n",
                    realid, IDL_IDENT(ident).str,
                    id, IDL_IDENT(ident).str);
        }/* for() */
      }
      break;
#endif
	default:
	  break;
    }
  }

  g_free(id);
  g_free(realid);
}

static void
doskel(IDL_tree cur, OIDL_Run_Info *rinfo, char *ifid, OIDL_C_Info *ci)
{
  char *id;

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_OP_DCL(cur).ident), "_", 0);

  fprintf(ci->fh, "void _ORBIT_skel_small_%s("
	    "POA_%s *_ORBIT_servant, "
	    "gpointer _ORBIT_retval, "
	    "gpointer *_ORBIT_args, "
	    "CORBA_Context ctx,"
	    "CORBA_Environment *ev, ", id, ifid);
  orbit_cbe_op_write_proto(ci->fh, cur, "_impl_", TRUE);
  fprintf(ci->fh, ");\n");

  g_free(id);
}

static void
ch_output_skel_protos(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  if(!tree) return;

  if ( tree->declspec & IDLF_DECLSPEC_PIDL )
	return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_MODULE:
    ch_output_skel_protos(IDL_MODULE(tree).definition_list, rinfo, ci);
    break;
  case IDLN_LIST:
    {
      IDL_tree sub;

      for(sub = tree; sub; sub = IDL_LIST(sub).next) {
	ch_output_skel_protos(IDL_LIST(sub).data, rinfo, ci);
      }
    }
    break;
  case IDLN_INTERFACE:
    {
      IDL_tree sub;
      char *ifid;

      ifid = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(tree).ident), "_", 0);

      for(sub = IDL_INTERFACE(tree).body; sub; sub = IDL_LIST(sub).next) {
	IDL_tree cur;

	cur = IDL_LIST(sub).data;

	switch(IDL_NODE_TYPE(cur)) {
	case IDLN_OP_DCL:
	  doskel(cur, rinfo, ifid, ci);
	  break;
	case IDLN_ATTR_DCL:
	  {
	    OIDL_Attr_Info *ai = cur->data;
	    IDL_tree curitem;

	    for(curitem = IDL_ATTR_DCL(cur).simple_declarations; curitem; curitem = IDL_LIST(curitem).next) {
	      ai = IDL_LIST(curitem).data->data;
	      
	      doskel(ai->op1, rinfo, ifid, ci);
	      if(ai->op2)
		doskel(ai->op2, rinfo, ifid, ci);
	    }
	  }
	  break;
	default:
	  break;
	}
      }
      g_free(ifid);
    }
    break;
  default:
    break;
  }
}

static void
ch_output_itypes (IDL_tree tree, OIDL_C_Info *ci)
{
	static int num_methods = 0;

	if (!tree)
		return;

	switch(IDL_NODE_TYPE(tree)) {
	case IDLN_MODULE:
		ch_output_itypes (IDL_MODULE(tree).definition_list, ci);
		break;
	case IDLN_LIST: {
		IDL_tree sub;
		for (sub = tree; sub; sub = IDL_LIST(sub).next)
			ch_output_itypes (IDL_LIST(sub).data, ci);
	}
	break;
	case IDLN_ATTR_DCL: {
		OIDL_Attr_Info *ai = tree->data;

		IDL_tree curitem;
      
		for(curitem = IDL_ATTR_DCL(tree).simple_declarations; curitem;
		    curitem = IDL_LIST(curitem).next) {
			ai = IDL_LIST(curitem).data->data;
	
			ch_output_itypes (ai->op1, ci);
			if(ai->op2)
				ch_output_itypes (ai->op2, ci);
		}
	}
	break;

	case IDLN_INTERFACE: {
		char  *id;

		id = IDL_ns_ident_to_qstring (IDL_IDENT_TO_NS (
			IDL_INTERFACE (tree).ident), "_", 0);

		ch_output_itypes (IDL_INTERFACE(tree).body, ci);
      
		fprintf (ci->fh, "#ifdef ORBIT_IDL_C_IMODULE_%s\n",
			 ci->c_base_name);
		fprintf (ci->fh, "static \n");
		fprintf (ci->fh, "#else\n");
		fprintf (ci->fh, "extern \n");
		fprintf (ci->fh, "#endif\n");
		fprintf (ci->fh, "ORBit_IInterface %s__iinterface;\n", id);

		fprintf (ci->fh, "#define %s_IMETHODS_LEN %d\n", id, num_methods);

		if (num_methods == 0)
			fprintf (ci->fh, "#define %s__imethods (ORBit_IMethod*) NULL\n", id);
		else {
			fprintf (ci->fh, "#ifdef ORBIT_IDL_C_IMODULE_%s\n",
				 ci->c_base_name);
			fprintf (ci->fh, "static \n");
			fprintf (ci->fh, "#else\n");
			fprintf (ci->fh, "extern \n");
			fprintf (ci->fh, "#endif\n");
			fprintf (ci->fh, "ORBit_IMethod %s__imethods[%s_IMETHODS_LEN];\n", id, id);
		}


		num_methods = 0;

		break;
	}

	case IDLN_OP_DCL:
		num_methods++;
		break;
	default:
		break;
	}
}
