#!/bin/bash

type="1"
if [ $# -eq 1 ];then
	type=$1
fi

BUILD_TOP_DIR=$(pwd)
STRIP=""
if [ -"$type" = -"PUC_ARM" ];then
	export THIRD_RELEASE=$BUILD_TOP_DIR/release
	STRIP="arm-none-linux-gnueabi-strip"
	export LD_LIBRARY_PATH=$BUILD_TOP_DIR/release_linux/mcpp-2.7.2/lib:$LD_LIBRARY_PATH
elif [ -"$type" = -"ANDROID" ];then
	export THIRD_RELEASE=$BUILD_TOP_DIR/release_android
	STRIP="arm-linux-androideabi-strip"
	export LD_LIBRARY_PATH=$BUILD_TOP_DIR/release_linux/mcpp-2.7.2/lib:$LD_LIBRARY_PATH
else
	export THIRD_RELEASE=$BUILD_TOP_DIR/release_linux
	export LD_LIBRARY_PATH=$THIRD_RELEASE/mcpp-2.7.2/lib:$LD_LIBRARY_PATH
	STRIP="strip -s"
fi

export ICE_CPP=$BUILD_TOP_DIR

echo -e "\n........................begin to build Ice-3.5.1........................\n"

BIN_STRIP_PATH=$ICE_CPP/bin
LIB_STRIP_PATH=$ICE_CPP/lib

cd $ICE_CPP/src
if [ -"$type" = -"PUC_ARM" ];then
	export LD_LIBRARY_PATH=$LIB_STRIP_PATH:$LD_LIBRARY_PATH
	export PATH=$BIN_STRIP_PATH:$PATH
	BIN_STRIP_PATH=$ICE_CPP/bin_arm
	LIB_STRIP_PATH=$ICE_CPP/lib_arm
	if ! -d [ $BIN_STRIP_PATH ];then
		mkdir -p $BIN_STRIP_PATH
	fi
	if ! -d [ $LIB_STRIP_PATH ];then
		mkdir -p $LIB_STRIP_PATH
	fi
	make PUC_ARM=1 clean
	make PUC_ARM=1 
elif [ -"$type" = -"ANDROID" ];then
	export LD_LIBRARY_PATH=$LIB_STRIP_PATH:$LD_LIBRARY_PATH
	export PATH=$BIN_STRIP_PATH:$PATH
	BIN_STRIP_PATH=$ICE_CPP/bin_android
	LIB_STRIP_PATH=$ICE_CPP/lib_android
	if ! -d [ $BIN_STRIP_PATH ];then
		mkdir -p $BIN_STRIP_PATH
	fi
	if ! -d [ $LIB_STRIP_PATH ];then
		mkdir -p $LIB_STRIP_PATH
	fi
	export ANDROID_THIRDPART=$BUILD_TOP_DIR/../../../../Inter_release/platform/ice/NdkExtend
	make ANDROID=1 clean
	make ANDROID=1 
else
	make clean
	make
fi
echo -e "\n........................build Ice-3.5.1 over........................\n"

echo -e "\n........................begin to backup non-strip Ice-3.5.1 lib........................\n"

if ( test ! -d $BIN_STRIP_PATH/non-strip )
then
	mkdir -p $BIN_STRIP_PATH/non-strip
fi
cp -d $BIN_STRIP_PATH/* $BIN_STRIP_PATH/non-strip

if ( test ! -d $LIB_STRIP_PATH/non-strip )
then
	mkdir -p $LIB_STRIP_PATH/non-strip
fi
cp -d $LIB_STRIP_PATH/* $LIB_STRIP_PATH/non-strip

echo -e "\n........................backup non-strip Ice-3.5.1 lib over........................\n"

echo -e "\n........................begin to strip Ice-3.5.1 lib........................\n"

binfilelist=`ls $BIN_STRIP_PATH`
for binfile in $binfilelist
do
        if [ ! -d "$BIN_STRIP_PATH/$binfile" -a -x "$BIN_STRIP_PATH/$binfile" ]
        then
                if [ -L "$BIN_STRIP_PATH/$binfile" ]
                then
                        echo "$binfile is a link file"
                else
                        echo "strip $binfile"
                        $STRIP "$BIN_STRIP_PATH/$binfile"
                fi
        else
                echo "$binfile is wrong format, not strip"
        fi
done

libfilelist=`ls $LIB_STRIP_PATH`
for libfile in $libfilelist
do
        if [ ! -d "$LIB_STRIP_PATH/$libfile" -a -x "$LIB_STRIP_PATH/$libfile" ]
        then
                if [ -L "$LIB_STRIP_PATH/$libfile" ]
                then
                        echo "$libfile is a link file, not strip"
                else
			echo "strip $libfile"	
                        $STRIP "$LIB_STRIP_PATH/$libfile"
                fi
        else
                echo "$libfile is wrong format, not strip"
        fi
done

echo -e "\n........................strip Ice-3.5.1 lib over........................\n"

