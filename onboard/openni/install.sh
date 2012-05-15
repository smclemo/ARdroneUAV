#!/bin/sh -e

INSTALL_LIB=/usr/lib
INSTALL_BIN=/usr/bin
INSTALL_INC=/usr/include/ni
INSTALL_VAR=/var/lib/ni

OS_NAME=`uname -s`

case $OS_NAME in
Darwin)
	MODULES="libnimMockNodes.dylib libnimCodecs.dylib libnimRecorder.dylib"
	;;
*)
	MODULES="libnimMockNodes.so libnimCodecs.so libnimRecorder.so"
	;;
esac

SCRIPT_DIR=`pwd`/`dirname $0`

# create file list
LIB_FILES=`ls $SCRIPT_DIR/Lib/*`
BIN_FILES=`ls $SCRIPT_DIR/Bin/ni*`

case $1 in
-i | "")
	echo "Installing OpenNI\n"
	echo "****************************\n\n"
	
	# copy libraries
	echo "copying shared libraries..."
	cp $LIB_FILES $INSTALL_LIB
    echo "OK\n"

	# utilities
	echo "copying executables..."
	cp $BIN_FILES $INSTALL_BIN
    echo "OK\n"

	# include files
	echo "copying include files..."
	mkdir -p $INSTALL_INC
	cp -r Include/* $INSTALL_INC
    echo "OK\n"

	# create database dir
	echo "creating database directory..."
	mkdir -p $INSTALL_VAR
    echo "OK\n"

	# register modules
	for module in $MODULES; do
		echo "registering module '$module'..."
		niReg -r $INSTALL_LIB/$module
		echo "OK\n"
	done

        # mono
	if [ -f /usr/bin/gmcs ]
	then
		gacutil -i Bin/OpenNI.net.dll -package 2.0
	fi
	;;
	
-u)
	echo "Uninstalling OpenNI\n"
	echo "****************************\n\n"

	# unregister modules
	for module in $MODULES; do
    	echo "unregistering module '$module'..."
        if niReg -u $INSTALL_LIB/$module; then
            echo "OK\n"
        fi
	done

	# include files
	echo "removing include files..."
	rm -rf $INSTALL_INC
	echo "OK\n"

	# binaries
	echo "removing executables..."
    for filename in $BIN_FILES; do
        rm -f $INSTALL_BIN/`basename $filename`
    done
	echo "OK\n"

	# libraries
    echo "removing shared libraries..."
    for filename in $LIB_FILES; do
        rm -f $INSTALL_LIB/`basename $filename`
    done
    echo "OK\n"

	# mono
	if [ -f /usr/bin/gmcs ]
	then
		echo "Removing OpenNI.net: "
		gacutil -u OpenNI.net
	fi
	;;
*) 
	echo "Usage: $0 [options]"
	echo "Available options:"
	echo "\t-i\tInstall (default)\n"
	echo "\t-u\tUninstall\n"
	exit 1
	;;
esac

echo "\n*** DONE ***\n\n"

