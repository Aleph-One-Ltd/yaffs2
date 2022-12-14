# Main Makefile for out-of-tree yaffs2 .ko building
#
# You can make two flavours of the .ko
#  make YAFFS_CURRENT=1  : makes yaffs2.ko using the current version code
#  make                  : makes yaffs2multi.ko using the multi-version code
#
# YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
#
# Copyright (C) 2002-2018 Aleph One Ltd.
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ifdef YAFFS_CURRENT
	YAFFS_O := yaffs2.o
	EXTRA_CFLAGS += -DYAFFS_CURRENT
else
	YAFFS_O := yaffs2multi.o
endif

ifneq ($(KERNELRELEASE),)
	EXTRA_CFLAGS += -DYAFFS_OUT_OF_TREE

	ccflags-y += -I$(src)
	ccflags-y += -I$(src)/../core/

	obj-m := $(YAFFS_O)

	yaffs2-objs := yaffs_vfs_single.o
	yaffs2-objs += yaffs_mtdif_single.o
	yaffs2-objs += ../core/yaffs_cache.o
	yaffs2-objs += ../core/yaffs_packedtags1.o
	yaffs2-objs += ../core/yaffs_ecc.o ../core/yaffs_guts.o
	yaffs2-objs += ../core/yaffs_packedtags2.o
	yaffs2-objs += ../core/yaffs_tagscompat.o
	yaffs2-objs += ../core/yaffs_tagsmarshall.o
	yaffs2-objs += ../core/yaffs_checkptrw.o ../core/yaffs_nand.o
	yaffs2-objs += ../core/yaffs_checkptrw.o ../core/yaffs_nand.o ../core/yaffs_nameval.o
	yaffs2-objs += ../core/yaffs_allocator.o ../core/yaffs_bitmap.o ../core/yaffs_attribs.o
	yaffs2-objs += ../core/yaffs_yaffs1.o
	yaffs2-objs += ../core/yaffs_yaffs2.o
	yaffs2-objs += ../core/yaffs_verify.o
	yaffs2-objs += ../core/yaffs_endian.o
	yaffs2-objs += ../core/yaffs_summary.o

	yaffs2multi-objs := yaffs_vfs_multi.o
	yaffs2multi-objs += yaffs_mtdif_multi.o
	yaffs2multi-objs += ../core/yaffs_cache.o
	yaffs2multi-objs += ../core/yaffs_packedtags1.o
	yaffs2multi-objs += ../core/yaffs_ecc.o ../core/yaffs_guts.o
	yaffs2multi-objs += ../core/yaffs_packedtags2.o
	yaffs2multi-objs += ../core/yaffs_tagscompat.o
	yaffs2multi-objs += ../core/yaffs_tagsmarshall.o
	yaffs2multi-objs += ../core/yaffs_checkptrw.o ../core/yaffs_nand.o
	yaffs2multi-objs += ../core/yaffs_checkptrw.o ../core/yaffs_nand.o ../core/yaffs_nameval.o
	yaffs2multi-objs += ../core/yaffs_allocator.o ../core/yaffs_bitmap.o ../core/yaffs_attribs.o
	yaffs2multi-objs += ../core/yaffs_yaffs1.o
	yaffs2multi-objs += ../core/yaffs_yaffs2.o
	yaffs2multi-objs += ../core/yaffs_verify.o
	yaffs2multi-objs += ../core/yaffs_endian.o
	yaffs2multi-objs += ../core/yaffs_summary.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

modules default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

mi modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f $(PWD)/../core/.*.o.cmd $(PWD)/../core/*.o
endif
