#ifndef ORBIT_CONFIG_H
#define ORBIT_CONFIG_H 1

#define ORBIT_CONFIG_SERIAL 20
#ifndef ORBIT_MAJOR_VERSION
#define ORBIT_MAJOR_VERSION (2)
#define ORBIT_MINOR_VERSION (14)
#define ORBIT_MICRO_VERSION (0)
#endif

#define ORBIT_IMPLEMENTS_IS_A 1

/*
 * Alignment of CORBA types mapped to C.
 * These have *nothing* to do with CDR alignment.
 */
#define ORBIT_ALIGNOF_CORBA_OCTET        1
#define ORBIT_ALIGNOF_CORBA_BOOLEAN      1
#define ORBIT_ALIGNOF_CORBA_CHAR         1
#define ORBIT_ALIGNOF_CORBA_WCHAR        2
#define ORBIT_ALIGNOF_CORBA_SHORT        2
#define ORBIT_ALIGNOF_CORBA_LONG         4
#define ORBIT_ALIGNOF_CORBA_LONG_LONG    4
#define ORBIT_ALIGNOF_CORBA_FLOAT        4
#define ORBIT_ALIGNOF_CORBA_DOUBLE       4
#define ORBIT_ALIGNOF_CORBA_LONG_DOUBLE  4
#define ORBIT_ALIGNOF_CORBA_STRUCT       1
#define ORBIT_ALIGNOF_CORBA_POINTER      4

#endif
