# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.5.1
INTVERSION		= 3.5.1
SHORT_VERSION           = 3.5
PATCH_VERSION           = 1
SOVERSION		= 35

OBJEXT			= .obj

#
# Ensure ice_language has been set by the file that includes this one.
#
!if "$(ice_language)" == ""
!error ice_language must be defined
!endif

!if "$(USE_BIN_DIST)" == "yes"
ice_bin_dist = 1
!endif

!if "$(PLATFORM)" == "x64" || "$(PLATFORM)" == "X64" || "$(AS)" == "ml64"
x64suffix		= \x64
ARCH			= x64
!elseif "$(PLATFORM)" == "ARM"
ARCH			= arm
!else
ARCH			= x86
!endif

!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
ice_bin_dist_dir = $(PROGRAMFILES) (x86)\ZeroC\Ice-$(VERSION)
!else
ice_bin_dist_dir = $(PROGRAMFILES)\ZeroC\Ice-$(VERSION)
!endif

#
# The following variables might also be defined:
#
# - slice_translator: the name of the slice translator required for the build.
#   Setting this variable is required when building source trees other than the
#   the source distribution (e.g.: the demo sources).
#
# - ice_require_cpp: define this variable to check for the presence of the C++
#   dev kit and check for the existence of the include/Ice/Config.h header.
#

#
# First, check if we're building a source distribution. 
#
# If building from a source distribution, ice_dir is defined to the
# top-level directory of the source distribution and ice_cpp_dir is
# defined to the directory containing the C++ binaries and headers to
# use to build the sources.
#
!if "$(ice_bin_dist)" == "" && exist ($(top_srcdir)\..\$(ice_language))

ice_dir = $(top_srcdir)\..
ice_src_dist = 1

#
# When building a source distribution, if ICE_HOME is specified, it takes precedence over 
# the source tree for building the language mappings. For example, this can be used to 
# build the Python language mapping using the translators from the distribution specified
# by ICE_HOME.
#
!if "$(ICE_HOME)" != ""

!if "$(slice_translator)" != ""
!if !exist ("$(ICE_HOME)\bin$(x64suffix)\$(slice_translator)")
!error Unable to find $(slice_translator) in $(ICE_HOME)\bin$(x64suffix), please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
!if exist ($(ice_dir)\cpp\bin\$(slice_translator))
!message Found $(slice_translator) in both ICE_HOME\bin and $(ice_dir)\cpp\bin, ICE_HOME\bin\$(slice_translator) will be used!
!endif
ice_cpp_dir = $(ICE_HOME)
!else
!message Ignoring ICE_HOME environment variable to build current source tree.
ice_cpp_dir = $(ice_dir)\cpp
!endif

!else

ice_cpp_dir = $(ice_dir)\cpp

!endif

!endif

#
# Then, check if we're building against a binary distribution.
#
!if "$(ice_src_dist)" == ""

!if "$(slice_translator)" == ""
!error slice_translator must be defined
!endif

!if "$(ICE_HOME)" != ""
!if !exist ("$(ICE_HOME)\bin$(x64suffix)\$(slice_translator)")
!error Unable to find $(slice_translator) in $(ICE_HOME)\bin$(x64suffix), please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
ice_dir = $(ICE_HOME)
!elseif exist ($(top_srcdir)/bin/$(slice_translator))
ice_dir = $(top_srcdir)
!elseif exist ("$(ice_bin_dist_dir)\bin$(x64suffix)\$(slice_translator)")
ice_dir = $(ice_bin_dist_dir)
!endif

!if "$(ice_dir)" == ""
!error Unable to find a valid Ice distribution, please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
ice_bin_dist = 1
ice_cpp_dir = $(ice_dir)
!endif

#
# If ice_require_cpp is defined, ensure the C++ headers exist
#
!if "$(ice_require_cpp)" == "yes"
!if "$(ice_src_dist)" != ""
ice_cpp_header = $(ice_cpp_dir)\include\Ice\Ice.h
!else
ice_cpp_header = $(ice_dir)\include\Ice\Ice.h
!endif
!if !exist ("$(ice_cpp_header)")
!error Unable to find the C++ header file $(ice_cpp_header), please verify ICE_HOME is properly configured and Ice is correctly installed.
!endif
!endif

#
# Set slicedir to the path of the directory containing the Slice files.
#
slicedir		= $(ice_dir)\slice

!if exist ($(top_srcdir)\..\slice)
install_slicedir    	= $(prefix)\slice
!endif

all::

!if exist ($(top_srcdir)\..\ICE_LICENSE.txt)
TEXT_EXT	= .txt
!endif

install-common::
	@if not exist "$(prefix)" \
	    @echo "Creating $(prefix)..." && \
	    mkdir "$(prefix)"

!if "$(WINRT)" != "yes" && "$(install_slicedir)" != ""
	@if not exist "$(install_slicedir)" \
	    @echo "Creating $(install_slicedir)..." && \
	    mkdir "$(install_slicedir)" && \
	    @echo "Copying slice files..." && \
	    cmd /c "xcopy /s /y $(top_srcdir)\..\slice "$(install_slicedir)"" || exit 1

	@if not exist "$(prefix)\ICE_LICENSE$(TEXT_EXT)" \
	    @copy $(top_srcdir)\..\ICE_LICENSE$(TEXT_EXT) "$(prefix)"
	@if not exist "$(prefix)\LICENSE$(TEXT_EXT)" \
	    @copy $(top_srcdir)\..\LICENSE$(TEXT_EXT) "$(prefix)"
!endif
