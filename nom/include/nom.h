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

#ifndef NOM_H_INCLUDED
#define NOM_H_INCLUDED


#include <glib.h>

#ifndef NOMEXTERN
 #ifdef __cplusplus
  #define NOMEXTERN extern "C"
 #else
  #define NOMEXTERN extern
 #endif
#endif

#define  NOM_Scope  NOMEXTERN

#ifndef NOMLINK
#if defined(__OS2__)
  #if defined(__IBMCPP__)
    #define NOMLINK _System
  #elif defined(__EMX__)
    #define NOMLINK _System 
  #else
    #define NOMLINK
  #endif /* __IBMCPP__ */
#else
  #define NOMLINK 
#endif
#endif

#define NOMDLINK

/** @def NOMDLLEXPORT 
 * Used for exporting a symbol. 
 * See __declspec(dllexport) in the Visual C++ reference for details. */
/** @def NOMDLLIMPORT 
 * Used for importing a symbol from a DLL/so. 
 * See __declspec(dllimport) in the Visual C++ reference for details. */
#if defined(_WIN32) || (defined(__OS2__) && defined(__GNUC__) && defined(__declspec))
# define NOMDLLEXPORT __declspec(dllexport)
# define NOMDLLIMPORT __declspec(dllimport)
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define NOMDLLEXPORT __attribute__((visibility("default")))
# else
#  define NOMDLLEXPORT
# endif
# define NOMDLLIMPORT
#else
# define NOMDLLEXPORT
# define NOMDLLIMPORT
#endif

typedef void* NOMLINK nomMethodProc(void*);

#ifndef NULL
# ifdef __cplusplus
#  define NULL 0
# else
#  define NULL ((void *) 0)
# endif
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

typedef gchar *string;

typedef gint16    CORBA_short;
typedef gint32    CORBA_long;
typedef guint16   CORBA_unsigned_short;
typedef guint32   CORBA_unsigned_long;
typedef gfloat    CORBA_float;
typedef gdouble   CORBA_double;
typedef char      CORBA_char;
typedef gunichar2 CORBA_wchar;
typedef guchar    CORBA_boolean;
typedef guchar    CORBA_octet;
typedef gdouble   CORBA_long_double;
typedef char*     CORBA_string;

//typedef gboolean    boolean;

#if 0
#if !defined(ORBIT_DECL_CORBA_Object) && !defined(_CORBA_Object_defined)
#define ORBIT_DECL_CORBA_Object 1
#define _CORBA_Object_defined 1
typedef struct CORBA_Object_type *CORBA_Object;
#endif
#endif

typedef unsigned long NOM_ulong;
typedef GQuark nomId;
typedef nomId* pnomId;

typedef GData*  PGData;
typedef gulong* pgulong;
typedef GTree*  PGTree;
typedef GArray* PGArray;


typedef gchar **nomID;                 /* This will be reomoved later. Don't use anymore */ 
typedef void *nomToken;                /* Use a gpointer instead of void* here?          */

#ifndef NOM_BOOLEAN
  #define NOM_BOOLEAN
  typedef gboolean boolean;  
#endif /* NOM_BOOLEAN */


/*  Object Instance Structure */
struct nomMethodTabStruct;
typedef struct NOMAnyObj_struct {
  struct nomMethodTabStruct  *mtab;
  gulong body[1];
} NOMAnyObj;

#if 0
#ifndef NOMClass
typedef struct NOMClass_struct {
  struct nomMethodTabStruct  *mtab;
  gulong body[1];
} NOMClassObj;
#define NOMClass NOMClassObj
typedef NOMClass *PNOMClass;
#endif
#endif

#ifndef NOMObject
typedef struct NOMObject_struct {
  struct nomMethodTabStruct  *mtab;
  gulong body[1];
} NOMObjectObj;
#define NOMObject NOMObjectObj
typedef NOMObject *PNOMObject;
#endif

#define NOMClass NOMObject
typedef NOMClass *PNOMClass;

//#define NOMObject NOMAnyObj
#define NOMClassMgr NOMAnyObj

typedef NOMObject *CORBA_Object;

/* The following is probably not correct... */
//typedef NOMAnyObj CORBA_Environment;

#define nomresolve_(obj,mToken) ((nomMethodProc*)((void)obj, mToken))
#define NOM_Resolve(obj, objClassName, methodName) \
    (( nomTD_ ## objClassName ## _ ## methodName ) \
     nomresolve_(obj, objClassName ## ClassData.methodName ))

#include <nomapi.h>

#endif /* NOM_H_INCLUDED */







