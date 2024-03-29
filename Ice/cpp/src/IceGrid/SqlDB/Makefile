# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

LIBFILENAME     = $(call mklibfilename,IceGridSqlDB,$(VERSION))
SONAME          = $(call mksoname,IceGridSqlDB,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceGridSqlDB)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS  		= SqlStringApplicationInfoDict.o \
		  SqlIdentityObjectInfoDict.o \
		  SqlStringAdapterInfoDict.o \
		  SqlDB.o

DB_OBJS		= SqlTypes.o

%.o: $(top_srcdir)/src/IceDB/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I../.. $(CPPFLAGS) $(QT_FLAGS)

LINKWITH        := $(QT_RPATH_LINK) $(QT_LIBS) -lIceGrid -lGlacier2 -lIceDB -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(OBJS) $(DB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(DB_OBJS),$(LINKWITH))
ifneq ($(ANDROID),1)
$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
endif


install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))


include .depend
