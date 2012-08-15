#**********************************************************************
#* Copyright (C) 1999-2008, International Business Machines Corporation
#* and others.  All Rights Reserved.
#**********************************************************************
# nmake file for creating data files on win32
# invoke with
# nmake /f makedata.mak icup=<path_to_icu_instalation> distout=<path_to_binaries> [Debug|Release]
#
#	12/10/1999	weiv	Created

#If no config, we default to debug
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to common - Win32 Debug.
!ENDIF

#Here we test if a valid configuration is given
!IF "$(CFG)" != "Release" && "$(CFG)" != "release" && "$(CFG)" != "Debug" && "$(CFG)" != "debug" && "$(CFG)" != "x86\Release" && "$(CFG)" != "x86\Debug" && "$(CFG)" != "x64\Release" && "$(CFG)" != "x64\Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "makedata.mak" CFG="Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Release"
!MESSAGE "Debug"
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

#Let's see if user has given us a path to ICU
#This could be found according to the path to makefile, but for now it is this way
!IF "$(ICUP)"==""
!ERROR Can't find path!
!ENDIF
!MESSAGE ICU path is $(ICUP)
RESNAME=uconvmsg
RESDIR=resources
RESFILES=resfiles.mk
ICUDATA=$(ICUP)\data

#  DISTOUT
#    Our final output location.
!if "$(DISTOUT)"==""
!ERROR Can't find DISTOUT (should point to final output location)!
!ENDIF
!MESSAGE DISTOUT path is $(DISTOUT)

# set the following to 'static' or 'dll' depending
PKGMODE=static

ICD=$(ICUDATA)^\
DATA_PATH=$(ICUP)\data^\

ICUTOOLS=$(DISTOUT)Bin
!if "$(PLATFORM)" == "x64"
ICUTOOLS=$(DISTOUT)Bin\x64
PATH = $(DISTOUT)Bin\x64;$(PATH)
!ELSE
PATH = $(DISTOUT)Bin;$(PATH)
!ENDIF

!MESSAGE ICUTOOLS path is $(ICUTOOLS)

# Suffixes for data files
.SUFFIXES : .ucm .cnv .dll .dat .res .txt .c

# We're including a list of resource files.
FILESEPCHAR=

!IF EXISTS("$(RESFILES)")
!INCLUDE "$(RESFILES)"
!ELSE
!ERROR ERROR: cannot find "$(RESFILES)"
!ENDIF
RES_FILES = $(RESSRC:.txt=.res)
RB_FILES = resources\$(RES_FILES:.res =.res resources\)
RESOURCESDIR=

# This target should build all the data files
!IF "$(PKGMODE)" == "dll"
DLL_OUTPUT=$(DISTOUT)Bin
!if "$(PLATFORM)" == "x64"
DLL_OUTPUT=$(DISTOUT)Bin\x64
!ENDIF
OUTPUT = "$(DLL_OUTPUT)\$(RESNAME).dll"
!ELSE
DLL_OUTPUT=$(DISTOUT)Lib
!if "$(PLATFORM)" == "x64"
DLL_OUTPUT=$(DISTOUT)Lib\x64
!ENDIF
OUTPUT = "$(DLL_OUTPUT)\$(RESNAME).lib"
!ENDIF

!MESSAGE DLL_OUTPUT path is $(DLL_OUTPUT)
!MESSAGE OUTPUT path is $(OUTPUT)

ALL : $(OUTPUT)
	@echo All targets are up to date (mode $(PKGMODE))


# invoke pkgdata - static
"$(DLL_OUTPUT)\$(RESNAME).lib" : $(RB_FILES) $(RESFILES)
	@echo Building $(RESNAME).lib
	@"$(ICUTOOLS)\pkgdata" -f -v -m static -c -p $(RESNAME) -d "$(DLL_OUTPUT)" -s "$(RESDIR)" <<pkgdatain.txt
$(RES_FILES:.res =.res
)
<<KEEP

# This is to remove all the data files
CLEAN :
    -@erase "$(RB_FILES)"
	-@erase "$(CFG)\*uconvmsg*.*"
    -@"$(ICUTOOLS)\pkgdata" -f --clean -v -m static -c -p $(RESNAME) -d "$(DLL_OUTPUT)" -s "$(RESDIR)" pkgdatain.txt

# Inference rule for creating resource bundles
{$(RESDIR)}.txt{$(RESDIR)}.res:
	@echo Making Resource Bundle files
	"$(ICUTOOLS)\genrb" -s $(@D) -d $(@D) $(?F)


$(RESSRC) : {"$(ICUTOOLS)"}genrb.exe

