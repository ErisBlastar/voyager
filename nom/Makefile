# Makefile

include ./make.inc
	

GCCINC		=	$(INC) 
INC		=	./include -I ./class_h -I $(FOUNDATIONPATH)/class_h -I $(FOUNDATIONPATH)/include
 

# Define search directories
CDIR		=	./src
CLASSCDIR	=	./class_c
CLASSTEMPLATEDIR	=	./../../../class_templates

CLASSINC	=	./class_h
IDLDIR		=	./idl
INCDIR		=	./include
OBJDIR		=	./../../../o
BINDIR		=	./../../../bin
DOCDIR		=	./../../../docs
TOOLDIR		=	./../tools

GCLIB		=	./../../../o/gc.a

# NOM IDL compiler
NOMIDLINC	=
#	-I$(FOUNDATIONPATH)/idl -I$(TKIDLDIR) \
#		-I$(GUITKPATH)/idl

NOMIDLCOMPILER	=	$(BINDIR)/nom-idl-compiler.exe 

ADDLIBS		=
#	-l$(OBJDIR)/voyfcls

# Define objects to build

TKOBJECTS	= 	$(OBJDIR)/nomtkinit.o \
			$(OBJDIR)/nommemory.o \
			$(OBJDIR)/nombuildclass.o \
			$(OBJDIR)/nomdebug.o \
			$(OBJDIR)/nombuildnomobj.o \
			$(OBJDIR)/nombuildnomcls.o \
			$(OBJDIR)/nomid.o \
			$(OBJDIR)/nomgc.o


NOMOBJECTS	=	$(OBJDIR)/nomobj.o \
			$(OBJDIR)/nomclassmanager.o \
			$(OBJDIR)/nomcls.o


NOMINCLUDES	=	$(INCDIR)/nomtk.h \
			$(INCDIR)/nomapi.h \
			$(INCDIR)/nom.h

TSTHFILES	=	$(CLASSINC)/nomobj.h $(CLASSINC)/nomcls.h

IHFILES		=	$(CLASSINC)/nomobj.ih \
			$(CLASSINC)/nomcls.ih \
			$(CLASSINC)/nomclassmanager.ih



OBJECTS		=	 $(NOMOBJECTS) $(TKOBJECTS)


.PRECIOUS:	$(CLASSINC)/%.ih $(CLASSCDIR)/%.c $(CDIR)/%.c

.PHONY:		dox

all:	$(BINDIR)/nobjtk.dll


$(BINDIR)/nobjtk.dll:	 $(OBJDIR)/o.dep $(IHFILES) $(NOMOBJECTS) $(OBJECTS) $(GCLIB) ./exports.def
	@echo "[33;1;mLinking "$@"...[0;m"
	cmd.exe /C create_vobjtk_def $(OBJDIR)/nobjtk.def
	$(CC) $(GCCLDFLAGS) -Zmap -o $@ $(OBJECTS) $(LIBS) $(ADDLIBS) $(GCLIB) $(OBJDIR)/nobjtk.def
	emximp -o $(OBJDIR)/$(basename $(notdir $@)).a $(OBJDIR)/nobjtk.def
	@echo "[32;1;mDone linking "$@"...[0;m"


###################################
#
#    Rules for compiling
#
###################################

# FIXME: includes should be created in the object dir
BASE_SCCOMPILEC		=	sc -I $(IDLDIR) -p -r -sc -d$(CLASSCDIR) -maddstar -mnoint -S128000 -C128000  $< && $(TOUCHFILE)
BASE_SCCOMPILEIH	=	sc -I $(IDLDIR) -p -r -sih -d$(CLASSINC) -maddstar -mnoint -S128000 -C128000  $< && $(FIXLINKAGEIH)
BASE_SCCOMPILEH		=	sc -I $(IDLDIR) -p -r -sh -d$(CLASSINC) -maddstar -mnoint -S128000 -C128000  $< && $(FIXLINKAGEH)

#BASE_NOMCOMPILEH	=	$(IDLCOMP) --showcpperrors --header --output-dir=$(CLASSINC) $<
#BASE_NOMCOMPILEIH	=	$(IDLCOMP) --showcpperrors --ihfile --output-dir=$(CLASSINC) $<
#BASE_NOMCOMPILEC	=	$(IDLCOMP) --showcpperrors --c-template --output-dir=$(CLASSTEMPLATEDIR) $< 
 
BASE_NOMCOMPILEH	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-h -d $(CLASSINC) -
BASE_NOMCOMPILEIH	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-ih -d $(CLASSINC) -
BASE_NOMCOMPILEC	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-c -d $(CLASSTEMPLATEDIR) -


$(OBJDIR)/%.o:	$(CLASSCDIR)/%.c $(NOMINCLUDES)
	$(CC) -I $(INC) $(GCCFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(CDIR)/%.c $(NOMINCLUDES)
	$(CC) -I $(INC) $(GCCFLAGS)  -o$@ $<


$(CLASSINC)/%.h:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEH)

$(CLASSINC)/%.ih:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)

$(CLASSCDIR)/%.c:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)
	@echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	@echo '!!! C template file created. Copy template source now !!!'
	@echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

# Create object dir for compilation results
$(OBJDIR)/o.dep:
	mkdir -p $(OBJDIR)
	echo > $(OBJDIR)/o.dep
	mkdir -p $(BINDIR)
	mkdir -p $(CLASSTEMPLATEDIR)
	mkdir -p $(DOCDIR)


$(GCLIB):
	cp $(GCPATH)/.libs/gc.a $(GCLIB)

dox:
	cd ./dox && doxygen nom.dox

clean:
	@cd $(OBJDIR) && rm *
