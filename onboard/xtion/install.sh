#!/bin/sh -e

SCRIPT_DIR=`pwd`/`dirname $0`

INSTALL_LIB=/usr/lib
INSTALL_BIN=/usr/bin
INSTALL_ETC=/usr/etc/primesense
INSTALL_RULES=/etc/udev/rules.d

OS_NAME=`uname -s`

case $OS_NAME in
Darwin)
    MODULES="libXnDeviceSensorV2.dylib libXnDeviceFile.dylib"
    ;;
*)
    MODULES="libXnDeviceSensorV2.so libXnDeviceFile.so"
    ;;
esac

RULES_FILE="55-primesense-usb.rules"

# create file list
LIB_FILES=`ls $SCRIPT_DIR/Lib/*`
BIN_FILES=`ls $SCRIPT_DIR/Bin/*`

case $1 in
-i | "")
	echo "Installing PrimeSense Sensor\n"
	echo "****************************\n\n"
	
    # create config dir
    echo "creating config dir $INSTALL_ETC..."
    mkdir -p $INSTALL_ETC
    echo "OK\n"

    # Copy shared libraries
    echo "copying shared libraries..."
    cp $LIB_FILES $INSTALL_LIB
    echo "OK\n"

    # Copy executables
    echo "copying executables..."
    cp $BIN_FILES $INSTALL_BIN
    echo "OK\n"

    # register modules
    for module in $MODULES; do
        echo "registering module '$module' with OpenNI..."
	niReg -r $INSTALL_LIB/$module $INSTALL_ETC
        echo "OK\n"
    done

    # copy config file
    echo "copying server config file..."
    cp Config/GlobalDefaults.ini $INSTALL_ETC
    echo "OK\n"

    # make server run as root
    echo "setting uid of server..."
    chown root $INSTALL_BIN/XnSensorServer
    chmod +s $INSTALL_BIN/XnSensorServer
    echo "OK\n"

    # create server log dir
    echo "creating server logs dir..."
    mkdir -p /var/log/primesense/XnSensorServer
    # make this dir readable and writable by all (we allow anyone to delete logs)
    chmod a+w /var/log/primesense/XnSensorServer
    echo "OK\n"

    if [ "`uname -s`" != "Darwin" ]; then
        # install USB rules (so that PrimeSense sensors will be mounted with write permissions)
        echo "installing usb rules..."
        cp Install/$RULES_FILE $INSTALL_RULES
        echo "OK\n"
    fi
    ;;
    
-u)
	echo "Uninstalling PrimeSense Sensor\n"
	echo "******************************\n\n"

    # unregister modules
    for module in $MODULES; do
    	echo "unregistering module '$module' from OpenNI..."
        if niReg -u $INSTALL_LIB/$module; then
            echo "OK\n"
        fi
    done

    # delete shared libraries
    echo "removing shared libraries..."
    for filename in $LIB_FILES; do
        rm -f $INSTALL_LIB/`basename $filename`
    done
    echo "OK\n"
	
    # delete executables
    echo "removing executables..."
    for filename in $BIN_FILES; do
        rm -f $INSTALL_BIN/`basename $filename`
    done
    echo "OK\n"
	
    # delete config dir
    echo "removing config dir..."
	rm -rf $INSTALL_ETC
    echo "OK\n"

    if [ "`uname -s`" != "Darwin" ]; then
        # remove USB rules
        echo "removing usb rules..."
	    rm -f $INSTALL_RULES/$RULES_FILE
        echo "OK\n"
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
