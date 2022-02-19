# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak
SUBDIRS		= Glacier2 \
		  Ice \
		  IceSSL \
		  IceGrid \
		  IceStorm \
		  IceUtil

!if "$(WINRT)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  Freeze \
		  IceBox \
		  IcePatch2 \
		  IceSSL \
		  IceXML \
		  Slice
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
