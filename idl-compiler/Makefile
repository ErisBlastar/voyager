#
# Makefile for the NOM IDL compiler.
#
include ./../make.inc


OBJDIR		=	./../../../o
BINDIR		=	./../../../bin
DOCDIR		=	./../../../docs
CDIR		=	./c


PARSERDIR	=	./parser_c
HEMITTERDIR	=	./h-emitter_c
IHEMITTERDIR	=	./ih-emitter_c
CEMITTERDIR	=	./c-emitter_c

CFLAGS		=	-Zmt -Zcrtdll -D__OS2__ -D__ST_MT_ERRNO__ -c -Wall

CLASSINC	=	./class_h
INCDIR		=	./include

INC		= 	./include -I $(NOMPATH)/include
			

# Define objects to build


PGOBJECTS	=	$(OBJDIR)/nom-idl-compiler.o \
			$(OBJDIR)/token.o \
			$(OBJDIR)/util.o \
			$(OBJDIR)/emitter.o \
			$(OBJDIR)/open_outfile.o \
			$(OBJDIR)/printdata.o \
			$(OBJDIR)/typespec_parser.o \
			$(OBJDIR)/method_parser.o \
			$(OBJDIR)/instancevar_parser.o \
			$(OBJDIR)/interface_parser.o \
			$(OBJDIR)/classversion_parser.o \
			$(OBJDIR)/override_parser.o \
			$(OBJDIR)/hash_parser.o \
			$(OBJDIR)/lineinfo_parser.o \
			$(OBJDIR)/metaclass_parser.o \
			$(OBJDIR)/filestem_parser.o \
			$(OBJDIR)/h_file_emitter.o \
			$(OBJDIR)/ih_file_emitter.o \
			$(OBJDIR)/c_file_emitter.o


#$(CLASSINC)/wpnativetypes.ih \


.PRECIOUS:	$(CDIR)/%.c

.PHONY:		dox

all:	$(BINDIR)/nom-idl-compiler.exe

$(BINDIR)/nom-idl-compiler.exe:	$(OBJDIR)/o.dep $(PGOBJECTS)
	@echo "[33;1;mLinking "$@"...[0;m"
	$(CC)  -o $@ $(PGOBJECTS) $(LIBS)
	@echo "[32;1;mDone linking "$@"...[0;m"


###################################
#
#    Rules for compiling
#
###################################


$(OBJDIR)/%.o:	$(CDIR)/%.c $(INCDIR)/parser.h
	$(CC) -I $(INC) $(CFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(PARSERDIR)/%.c $(INCDIR)/parser.h
	$(CC) -I $(INC) $(CFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(HEMITTERDIR)/%.c $(INCDIR)/parser.h
	$(CC) -I $(INC) $(CFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(IHEMITTERDIR)/%.c $(INCDIR)/parser.h
	$(CC) -I $(INC) $(CFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(CEMITTERDIR)/%.c $(INCDIR)/parser.h
	$(CC) -I $(INC) $(CFLAGS)  -o$@ $<

# Create the directories for temp files
$(OBJDIR)/o.dep:
	mkdir -p $(OBJDIR)
	echo > $(OBJDIR)/o.dep
	mkdir -p $(BINDIR)
	mkdir -p $(DOCDIR)

dox:
	cd ./dox && doxygen desktop.dox

clean:
	-cd $(OBJDIR) && rm -f *.o
	-cd $(BINDIR) && rm -f *.dll
