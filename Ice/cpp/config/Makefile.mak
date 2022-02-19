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

install::
	copy templates.xml "$(install_configdir)"
	copy convertssl.py "$(install_configdir)"
	copy upgradeicegrid33.py "$(install_configdir)"
	copy upgradeicegrid35.py "$(install_configdir)"
	copy icegridregistry.cfg "$(install_configdir)"
	copy icegridnode.cfg "$(install_configdir)"
	copy glacier2router.cfg "$(install_configdir)"
	copy icegrid-slice.3.1.ice.gz "$(install_configdir)"
	copy icegrid-slice.3.2.ice.gz "$(install_configdir)"
	copy icegrid-slice.3.3.ice.gz "$(install_configdir)"
	copy icegrid-slice.3.5.ice.gz "$(install_configdir)"
