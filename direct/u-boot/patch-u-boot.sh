#!/bin/sh
#
# YAFFS: Yet another FFS. A NAND-flash specific file system.
#
# Copyright (C) 2002-2010 Aleph One Ltd.
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Patch yaffs2 into u-boot
#
#  args: 
#	  u-boot-path  : Full path to u-boot sources to be patched
#
#  Inspired yaffs kernel patching script

#set -e -x

UBOOTDIR=$1

# Display usage of this script
usage () {
	echo "usage:  $0 u-boot-path"
	exit 1
}

not_u_boot () {
	echo "$1 does not seem to be a u-boot directory"
	exit 1
}


if [ -z $UBOOTDIR ]
then
    usage;
fi

# Basic sanity check:
# Check it has a Makefile, common and fs sub-directories

if [ ! -f "$UBOOTDIR/Makefile" ]
then
	not_u_boot
fi

if [ ! -d "$UBOOTDIR/common" ]
then
	not_u_boot
fi

if [ ! -d "$UBOOTDIR/fs" ]
then
	not_u_boot
fi


YAFFS_IN_MAKEFILE=`grep -s libyaffs2 <$UBOOTDIR/Makefile | head -n 1`
YAFFS_IN_COMMON=`grep -s yaffs2 <$UBOOTDIR/common/Makefile | head -n 1`
LIB_A=`grep -s libnet\.a <$UBOOTDIR/Makefile | head -n 1`

if [ ! -z "$LIB_A" ]
then
    A_OR_O="a"
else
    A_OR_O="o"
fi

# Patch Makefile if it does not mention yaffs2
if [ ! -z "$YAFFS_IN_MAKEFILE" ]
then
    echo "$UBOOTDIR/Makefile already makes yaffs library"
else
   # Update the Makefile
   mv -f $UBOOTDIR/Makefile $UBOOTDIR/Makefile.old
   sed -n -e "/libnet/,99999 ! p" < $UBOOTDIR/Makefile.old > $UBOOTDIR/Makefile
   echo "LIBS += fs/yaffs2/libyaffs2.$A_OR_O" >> $UBOOTDIR/Makefile
   sed -n -e "/libnet/,99999 p" < $UBOOTDIR/Makefile.old >> $UBOOTDIR/Makefile
fi

# Patch common/Makefile if it does not mention yaffs2
if [ ! -z "$YAFFS_IN_COMMON" ]
then
    echo "$UBOOTDIR/common/Makefile already makes yaffs commands"
else
   # Update the Makefile
   mv -f $UBOOTDIR/common/Makefile $UBOOTDIR/common/Makefile.old
   sed -n -e "/cmd_jffs/,99999 ! p" <$UBOOTDIR/common/Makefile.old > $UBOOTDIR/common/Makefile
   echo "COBJS-\$(CONFIG_YAFFS2) += cmd_yaffs2.o" >> $UBOOTDIR/common/Makefile
   sed -n -e "/cmd_jffs/,99999 p" <$UBOOTDIR/common/Makefile.old >> $UBOOTDIR/common/Makefile
fi


#
# Ensure the direct code is up to date
#

RUN_DIR=`pwd`
cd ..
./handle_common.sh copy
cd $RUN_DIR

#
# Copy in the files we need
#

cp common/* $UBOOTDIR/common/

mkdir -p $UBOOTDIR/fs/yaffs2
cp ../*.[ch] $UBOOTDIR/fs/yaffs2/
cp fs/yaffs2/*.[ch] $UBOOTDIR/fs/yaffs2/

sed -e "s/A_OR_O/$A_OR_O/" < fs/yaffs2/Makefile > $UBOOTDIR/fs/yaffs2/Makefile



