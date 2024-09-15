#################################################################################
#####                 This makefile generates ELWCHR.DLL                  #####
#################################################################################
!include $(WGDEV)\src\gcrules.mak

PARSEOPTS = $(PARSEOPTS) -I. -DIS30

VIRDIR = .\GCVIRDAT

DEFILE =

VIRFILES =                 \
     GCVIRDAT\ELWCHRHI.VIR \
     GCVIRDAT\ELWCHRPL.VIR \
     GCVIRDAT\ELWCHRPZ.VIR

DLLLIB =            \
     $(DLLLIB)      \
     wgserver.lib   \
     galgsbl.lib

OBJFILES = \
     elwchr.obj 

start:              \
     $(GCBUILD)     \
     elwchr.dll

elwchr.dll:         \
     $(OBJFILES)    \
     $(DLLLIB)      \
     $(DEFILE)      \
     $(VIRFILES)
     $(LINK) $(LIBCMD) @&&|
$(PRODLL) $(OBJFILES)
$(RUNDIR)\$&
$(RUNDIR)\$&
$(DLLLIB) $(LINKDLL)
$(DEFILE)
|
