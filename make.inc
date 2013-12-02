#
# Includes for desktop makefiles
#

CC		=	gcc

GCCFLAGSDLL	=	-Zmtd -Zdll -D__OS2__ -D__ST_MT_ERRNO__ -c -Wall 
GCCLDFLAGSDLL	=	-Zmt  -Zdll

GCCFLAGS	=	-Zmt -Zcrtdll -D__OS2__ -D__ST_MT_ERRNO__ -c -Wall
#GCCFLAGS	=		-E -P

GCCLDFLAGS	=	-Zmt -Zcrtdll -Zdll

LIBS		=	-lgtk-x11-2.0 -lgdk-x11-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0


VERSION		=	0_0_1
VERSION_STRING  =	0.0.1
AUTHOR_STRING	=	"(C) Chris Wohlgemuth 2003-2007"

# Put version in environment for REXX skripts
export	DESKTOPVERSION	=	$(VERSION_STRING)
export	AUTHORSTRING	=	$(AUTHOR_STRING)


IDLCOMP		=	L:\svn-sources\nom\trunk\ORBit2-2.14.0\src\idl-compiler\.libs\orbit-idl-2.exe

NOMPATH		=	L:/svn-sources/nom/trunk/nom
GUITKPATH	=	L:/svn-sources/desktop/trunk/gui
FOUNDATIONPATH	=	L:/svn-sources/desktop/trunk/foundation
DESKTOPPATH	=	L:/svn-sources/desktop/trunk/desktop
