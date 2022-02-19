#编译器相关
ARCH		= x86_64
CROSS_COMPILE	= 
RELEASE_DIR = release_linux
ifeq ($(PUC_ARM), 1)
   ARCH		= armv6l
   CROSS_COMPILE	= arm-none-linux-gnueabi-
   RELEASE_DIR = release
endif
ifeq ($(ANDROID), 1)
   ARCH		= armeabi-v7a
   CROSS_COMPILE	= arm-linux-androideabi-
   RELEASE_DIR = release_android
endif


AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
CXX		= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
RANLIB		= $(CROSS_COMPILE)ranlib	

export CROSS_COMPILE ARCH AS LD CC CPP CXX AR NM STRIP OBJCOPY OBJDUMP RANLIB


MAKEFLAT:=Makefile.flat

export ICE_TOP_DIR = $(shell 'pwd')


#all target
all:mcpp db bzip2 expat openssl

clean:mcpp_clean db_clean bzip2_clean expat_clean openssl_clean
	rm -rf $(RELEASE_DIR)

	
#mcpp-2.7.2
export MCPP_SRCDIR=$(shell 'pwd')/ThirdParty-Sources-3.5.1/mcpp-2.7.2
mcpp:
	make -C $(MCPP_SRCDIR) -f $(MAKEFLAT) all
mcpp_clean:
	make -C $(MCPP_SRCDIR) -f $(MAKEFLAT) clean

#db-5.3.21.NC
export DB_SRCDIR=$(shell 'pwd')/ThirdParty-Sources-3.5.1/db-5.3.21.NC/build_unix
db:
	make -C $(DB_SRCDIR) -f $(MAKEFLAT) all
db_clean:
	make -C $(DB_SRCDIR) -f $(MAKEFLAT) clean

#bzip2-1.0.5
export BZIP_SRCDIR=$(shell 'pwd')/bzip2-1.0.5
bzip2:
	make -C $(BZIP_SRCDIR) all
	make -C $(BZIP_SRCDIR) install
bzip2_clean:
	make -C $(BZIP_SRCDIR) clean
	
	
#expat-2.0.1
export EXPAT_SRCDIR=$(shell 'pwd')/expat-2.0.1
expat:
	make -C $(EXPAT_SRCDIR) -f $(MAKEFLAT) all
expat_clean:
	make -C $(EXPAT_SRCDIR) -f $(MAKEFLAT) clean

#openssl-0.9.8g gcc 可以通过 os/compiler 配置，但是/ar/ranlib 无法在 config 中配置，直接使用config生成的Makefile
export OPENSSL_SRCDIR=$(shell 'pwd')/openssl-0.9.8g
openssl:
	make -C $(OPENSSL_SRCDIR) all
	make -C $(OPENSSL_SRCDIR) install
openssl_clean:                    
	make -C $(OPENSSL_SRCDIR) clean
	
install:
	if ( test ! -d $(RELEASE_DIR) ) ; then mkdir -p $(RELEASE_DIR) ; fi
	
	if ( test ! -d $(RELEASE_DIR)/mcpp-2.7.2 ) ; then mkdir -p $(RELEASE_DIR)/mcpp-2.7.2 ; fi
	cp -rf $(MCPP_SRCDIR)/install/* $(RELEASE_DIR)/mcpp-2.7.2 -R
	cp -rf $(MCPP_SRCDIR)/install/lib $(RELEASE_DIR)/mcpp-2.7.2/lib64 -R

	if ( test ! -d $(RELEASE_DIR)/db-5.3.21.NC ) ; then mkdir -p $(RELEASE_DIR)/db-5.3.21.NC ; fi
	cp -rf $(DB_SRCDIR)/../install/* $(RELEASE_DIR)/db-5.3.21.NC -R
	cp -rf $(RELEASE_DIR)/db-5.3.21.NC/lib/* $(ICE_TOP_DIR)/Ice/cpp/lib -R
	cp -rf $(RELEASE_DIR)/db-5.3.21.NC/lib $(RELEASE_DIR)/db-5.3.21.NC/lib64 -R

	if ( test ! -d $(RELEASE_DIR)/bzip2-1.0.5 ) ; then mkdir -p $(RELEASE_DIR)/bzip2-1.0.5 ; fi
	cp -rf $(BZIP_SRCDIR)/install/* $(RELEASE_DIR)/bzip2-1.0.5 -R	
	cp -rf $(RELEASE_DIR)/bzip2-1.0.5/lib $(RELEASE_DIR)/bzip2-1.0.5/lib64 -R	
	
	if ( test ! -d $(RELEASE_DIR)/expat-2.0.1 ) ; then mkdir -p $(RELEASE_DIR)/expat-2.0.1 ; fi
	cp -rf $(EXPAT_SRCDIR)/install/* $(RELEASE_DIR)/expat-2.0.1 -R
	cp $(RELEASE_DIR)/expat-2.0.1/lib $(RELEASE_DIR)/expat-2.0.1/lib64 -R

	if ( test ! -d $(RELEASE_DIR)/openssl-0.9.8g ) ; then mkdir -p $(RELEASE_DIR)/openssl-0.9.8g ; fi
	cp -rf $(OPENSSL_SRCDIR)/install/* $(RELEASE_DIR)/openssl-0.9.8g -R
	cp $(RELEASE_DIR)/openssl-0.9.8g/lib $(RELEASE_DIR)/openssl-0.9.8g/lib64 -R
