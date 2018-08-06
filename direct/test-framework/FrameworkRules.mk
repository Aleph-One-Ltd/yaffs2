# Makefile rules for building in test framwork
#
#
# YAFFS: Yet another Flash File System. A NAND-flash specific file system.
#
# Copyright (C) 2002-2018 Aleph One Ltd.
#
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# NB Warning this Makefile does not include header dependencies.
#

#EXTRA_COMPILE_FLAGS = -DYAFFS_IGNORE_TAGS_ECC

CFLAGS =      -DCONFIG_YAFFS_DIRECT -DCONFIG_YAFFS_YAFFS2  -DCONFIG_YAFFS_DEFINES_TYPES
CFLAGS +=     -DCONFIG_YAFFS_PROVIDE_DEFS -DCONFIG_YAFFSFS_PROVIDE_VALUES
CFLAGS +=    -Wall -g $(EXTRA_COMPILE_FLAGS) -Wstrict-aliasing -Werror
#CFLAGS +=    -fno-strict-aliasing
CFLAGS +=    -O0
CFLAGS +=    -Wextra -Wpointer-arith
#CFLAGS +=    -DCONFIG_YAFFS_VALGRIND_TEST

#CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
#CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline


COMMONTESTOBJS = yaffscfg2k.o yaffs_osglue.o yaffs_hweight.o yaffs_error.o\
		 yaffs_ecc.o yaffs_fileem.o yaffs_fileem2k.o yaffsfs.o yaffs_guts.o \
		 yaffs_packedtags1.o yaffs_ramdisk.o yaffs_ramem2k.o \
		 yaffs_tagscompat.o yaffs_tagsmarshall.o \
		 yaffs_packedtags2.o yaffs_nand.o \
		 yaffs_checkptrw.o  \
		 yaffs_endian.o \
		 yaffs_nameval.o yaffs_attribs.o \
		 yaffs_m18_drv.o  yaffs_nor_drv.o ynorsim.o \
		 yaffs_nand_drv.o \
		 nanddrv.o \
		 yaffs_nandsim_file.o nandsim.o nandsim_file.o nandstore_file.o \
		 yaffs_allocator.o \
		 yaffs_bitmap.o \
		 yaffs_yaffs1.o \
		 yaffs_yaffs2.o \
		 yaffs_verify.o \
		 yaffs_summary.o



ALLOBJS = $(sort $(ALL_UNSORTED_OBJS))

YAFFSDIRECTSYMLINKS =  \
          yaffsfs.c yaffs_flashif.h yaffs_flashif2.h\
          yaffsfs.h yaffs_osglue.h ydirectenv.h \
          yaffscfg.h \
          yaffs_nandemul2k.h yaffs_list.h \
          yaffs_attribs.c \
          yportenv.h \
          yaffs_hweight.c yaffs_hweight.h \
          yaffs_endian.c yaffs_endian.h \
	  yaffs_error.c \
          yaffs_ecc.c yaffs_ecc.h yaffs_guts.c yaffs_guts.h \
          yaffs_tagscompat.c yaffs_tagscompat.h \
          yaffs_tagsmarshall.c yaffs_tagsmarshall.h \
          yaffs_packedtags1.c yaffs_packedtags1.h \
          yaffs_packedtags2.c yaffs_packedtags2.h  \
          yaffs_nand.c yaffs_nand.h yaffs_getblockinfo.h \
          yaffs_checkptrw.h yaffs_checkptrw.c \
          yaffs_nameval.c yaffs_nameval.h \
          yaffs_trace.h yaffs_attribs.h \
          yaffs_allocator.c yaffs_allocator.h \
          yaffs_yaffs1.c yaffs_yaffs1.h \
          yaffs_yaffs2.c yaffs_yaffs2.h \
          yaffs_bitmap.c yaffs_bitmap.h \
          yaffs_verify.c yaffs_verify.h \
          yaffs_summary.c yaffs_summary.h


FRAMEWORKEXTRASYMLINKS = \
		yaffscfg2k.c yaffs_fileem2k.c yaffs_fileem2k.h\
		yaffs_fileem.c yaffs_m18_drv.c yaffs_m18_drv.h \
		yaffs_nor_drv.c yaffs_nor_drv.h \
		yaffs_nand_drv.c yaffs_nand_drv.h \
		yaffs_ramdisk.c yaffs_ramdisk.h yaffs_ramem2k.c \
		nand_chip.h nanddrv.c nanddrv.h nandsim.c nandsim.h nand_store.h \
		nandsim_file.c nandsim_file.h nandstore_file.c nandstore_file.h \
		yaffs_nandsim_file.c yaffs_nandsim_file.h \
		ynorsim.h ynorsim.c yaffs_osglue.c

FRAMEWORK_SOURCES = $(YAFFSDIRECTSYMLINKS) $(FRAMEWORKEXTRASYMLINKS)

clean:
	rm -f $(TARGETS) $(ALLOBJS) core $(FRAMEWORK_SOURCES) $(CLEAN_OBJS)

$(YAFFSDIRECTSYMLINKS):
	ln -s $(YDI_DIR)/$@ $@


$(FRAMEWORKEXTRASYMLINKS):
	ln -s $(YDI_FRAMEWORK_DIR)/$@ $@

$(ALLOBJS): %.o: %.c
	gcc -c $(CFLAGS)   -o $@ $<
