#! /bin/sh

YAFFS_COMMON_SOURCES="\
          yaffs_ecc.c yaffs_ecc.h \
          yaffs_cache.c yaffs_cache.h \
          yaffs_guts.c yaffs_guts.h \
          yaffs_encryption.c yaffs_encryption.h \
          yaffs_tagscompat.c yaffs_tagscompat.h \
          yaffs_tagsmarshall.c yaffs_tagsmarshall.h \
          yaffs_packedtags1.c yaffs_packedtags1.h \
          yaffs_packedtags2.c yaffs_packedtags2.h \
          yaffs_nand.c yaffs_nand.h yaffs_getblockinfo.h \
          yaffs_checkptrw.h yaffs_checkptrw.c \
          yaffs_nameval.c yaffs_nameval.h \
          yaffs_trace.h yaffs_attribs.h \
          yaffs_allocator.c yaffs_allocator.h \
          yaffs_yaffs1.c yaffs_yaffs1.h \
          yaffs_yaffs2.c yaffs_yaffs2.h \
          yaffs_bitmap.c yaffs_bitmap.h \
          yaffs_endian.c yaffs_endian.h \
          yaffs_verify.c yaffs_verify.h \
          yaffs_summary.c yaffs_summary.h \
          "


do_copy=0
do_clean=0

if [ "$1" = "copy" ] ; then
	do_copy=1
elif [ "$1" = "clean" ] ; then
	do_clean=1
elif [ "$1" = "refresh" ] ; then
	do_clean=1
	do_copy=1
else
	echo "please specify: copy, clean or refresh"
	echo "    copy: Copies core code into direct, doing some massages"
	echo "    clean: Cleans (removes the copied code)"
	echo "    refresh: Does a clean then a copy"
	exit 1
fi


if [ "${do_clean}" = "1" ] ; then
	echo "Cleaning old core source"
	for i in $YAFFS_COMMON_SOURCES ; do
		rm -rf $i
	done
fi

if [ "${do_copy}" = "1" ] ; then
	echo "Copying new core source"
set -e -x
	for i in $YAFFS_COMMON_SOURCES ; do
		sed ../core/$i \
		-e "s/strcat/yaffs_strcat/g" \
		-e "s/strcpy/yaffs_strcpy/g" \
		-e "s/strncpy/yaffs_strncpy/g" \
		-e "s/strnlen/yaffs_strnlen/g" \
		-e "s/strcmp/yaffs_strcmp/g" \
		-e "s/strncmp/yaffs_strncmp/g"\
		-e "s/loff_t/Y_LOFF_T/g" \
		 >$i

		chmod 0444 $i
	done
fi

