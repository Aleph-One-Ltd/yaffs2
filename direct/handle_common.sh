#! /bin/sh

YAFFS_COMMON_SOURCES="\
          yaffs_ecc.c yaffs_ecc.h yaffs_guts.c yaffs_guts.h \
          yaffs_tagscompat.c yaffs_tagscompat.h \
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
          yaffs_verify.c yaffs_verify.h \
          yaffs_summary.c yaffs_summary.h \
          "


if [ "$1" = "copy" ] ; then
	for i in $YAFFS_COMMON_SOURCES ; do
		sed ../$i \
		-e "s/strcat/yaffs_strcat/g" \
		-e "s/strcpy/yaffs_strcpy/g" \
		-e "s/strncpy/yaffs_strncpy/g" \
		-e "s/strnlen/yaffs_strnlen/g" \
		-e "s/strcmp/yaffs_strcmp/g" \
		-e "s/strncmp/yaffs_strncmp/g"\
		-e "s/loff_t/Y_LOFF_T/g" \
		 >$i
	done
elif [ "$1" = "clean" ] ; then 
	for i in $YAFFS_COMMON_SOURCES ; do
		rm -rf $i
	done
else
	echo "please specify copy or clean"
	exit 1
fi
