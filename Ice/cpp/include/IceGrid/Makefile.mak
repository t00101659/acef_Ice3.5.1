# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= IceGrid

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"

install::
	@if not exist "$(install_includedir)\IceGrid" \
	    @echo "Creating $(install_includedir)\IceGrid..." && \
	    mkdir "$(install_includedir)\IceGrid"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\IceGrid"

!else

all::
        @echo SDK_HEADERS       =  \> .headers
        @for /f %i in ('dir /b *.h') do \
                @echo ^ $$(SDK_INCLUDE_PATH)\$$(INCLUDE_DIR)\%i \>> .headers


!include .headers

all:: $(SDK_HEADERS)

install:: all

!endif
