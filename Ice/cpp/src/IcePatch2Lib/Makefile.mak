# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icepatch2$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icepatch2$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS         = $(LIBNAME) $(DLLNAME)

OBJS		= Util.obj \
		  ClientUtil.obj \
		  FileInfo.obj \
		  FileServer.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(headerdir)\IcePatch2
SDIR		= $(slicedir)\IcePatch2

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

SLICE2CPPFLAGS	= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IcePatch2.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IcePatch2.res
	$(LINK) $(BASE):0x29000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(BZIP2_LIBS) \
		$(OPENSSL_LIBS) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

clean::
	-del /q FileInfo.cpp $(HDIR)\FileInfo.h
	-del /q FileServer.cpp $(HDIR)\FileServer.h
	-del /q IcePatch2.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
