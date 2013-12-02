#
# Makefile for the Voyager foundation classes.
#

include ./../make.inc

# No extension here
DLLNAME		=	nomtest
OBJDIR		=	./../../../o
BINDIR		=	./../../../bin
DOCDIR		=	./../../../docs
CDIR		=	./c

CLASSCDIR	=	./class_c
CLASSTEMPLATEDIR	=	./../../../class_templates
CLASSINC	=	./class_h
INCDIR		=	./include
IDLDIR		=	./idl

# The object TK lives here
TKIDLDIR	=	$(NOMPATH)/idl

INC		= 	./include -I ./class_h \
			-I $(NOMPATH)/class_h -I $(NOMPATH)/include \
			-I $(GUITKPATH)/class_h -I $(GUITKPATH)/include

# NOM IDL compiler
NOMIDLINC	=	-I$(TKIDLDIR)
NOMIDLCOMPILER	=	$(BINDIR)/nom-idl-compiler.exe 


NOMTKLIB	=	-l$(OBJDIR)/nobjtk
ADDLIBS		=	-l$(OBJDIR)/nomtest $(NOMTKLIB) 

# Define objects to build


CLASSOBJECTS	=	$(OBJDIR)/aclass.o\
			$(OBJDIR)/bclass.o

OBJECTS		=	$(OBJDIR)/test-nom.o


.PRECIOUS:	$(CLASSINC)/%.ih $(CLASSCDIR)/%.c $(CDIR)/%.c

.PHONY:		dox

all:	$(BINDIR)/$(DLLNAME).dll $(BINDIR)/test-nom.exe



$(BINDIR)/$(DLLNAME).dll:	 $(OBJDIR)/o.dep $(CLASSOBJECTS) exports.def
	@echo "[33;1;mLinking "$@"...[0;m"
	@cmd.exe /C create_$(DLLNAME)_def $(OBJDIR)/$(DLLNAME).def
	$(CC) $(GCCLDFLAGS) -o $@ $(CLASSOBJECTS) $(LIBS) $(NOMTKLIB) $(OBJDIR)/$(DLLNAME).def
	emximp -o $(OBJDIR)/$(basename $(notdir $@)).a $(OBJDIR)/$(DLLNAME).def
	@echo "[32;1;mDone linking "$@"...[0;m"

$(BINDIR)/test-nom.exe:	$(OBJDIR)/o.dep $(OBJECTS)
	@echo "[33;1;mLinking "$@"...[0;m"
	$(CC)  -o $@ $(OBJECTS) $(LIBS) $(ADDLIBS)
	@echo "[32;1;mDone linking "$@"...[0;m"


###################################
#
#    Rules for compiling
#
###################################

#BASE_NOMCOMPILEH	=	$(IDLCOMP) -include=$(TKIDLDIR) --showcpperrors --header --output-dir=$(CLASSINC) $<
#BASE_NOMCOMPILEIH	=	$(IDLCOMP) -include=$(TKIDLDIR) --showcpperrors --ihfile --output-dir=$(CLASSINC) $<
#BASE_NOMCOMPILEC	=	$(IDLCOMP) -include=$(TKIDLDIR) --showcpperrors --c-template --output-dir=$(CLASSTEMPLATEDIR) $< 

BASE_NOMCOMPILEH	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-h -d $(CLASSINC) -
BASE_NOMCOMPILEIH	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-ih -d $(CLASSINC) -
BASE_NOMCOMPILEC	=	gcc -E -x c -D__NOM_IDL_COMPILER__ $(NOMIDLINC) $< | $(NOMIDLCOMPILER) --emit-c -d $(CLASSTEMPLATEDIR) -


# Rules for desktop classes
$(OBJDIR)/%.o:	$(CLASSCDIR)/%.c $(CLASSINC)/%.ih
	$(CC) -I $(INC) $(GCCFLAGS)  -o$@ $<


$(CLASSINC)/%.ih:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)

$(OBJDIR)/%.o:	$(CDIR)/%.c
	$(CC) -I $(INC) $(GCCFLAGSDLL)  -o$@ $<

$(CLASSCDIR)/%.c:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)


# Create the directories for temp files
$(OBJDIR)/o.dep:
	mkdir -p $(OBJDIR)
	echo > $(OBJDIR)/o.dep
	mkdir -p $(BINDIR)
	mkdir -p $(CLASSTEMPLATEDIR)
	mkdir -p $(DOCDIR)

dox:
	cd ./dox && doxygen foundation.dox

clean:
	-cd $(OBJDIR) && rm -f *.o
	-cd $(BINDIR) && rm -f *.dll
