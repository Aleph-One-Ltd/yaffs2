# Makefile for YAFFS direct stress tests
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




          


$(YAFFSLIBOBJS): %.o: %.c
	gcc -c $(CFLAGS)   -o $@ $<


$(YAFFSDIRECTSYMLINKS):
	ln -s ../$@ $@

$(DIRECTEXTRASYMLINKS):
	ln -s ../basic-test/$@ $@


# Makefile for stress tests and fuzzer
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

YDI_DIR = ../../
YDI_FRAMEWORK_DIR = ../

TARGETS = libyaffsfs.so


all: $(TARGETS)


YAFFSLIBOBJS  = $(COMMONTESTOBJS) yaffs_python_helper.o

ALL_UNSORTED_OBJS += $(YAFFSLIBOBJS)

include ../FrameworkRules.mk

CFLAGS +=    -O0 -fPIC


libyaffsfs.so: $(FRAMEWORK_SOURCES) $(YAFFSLIBOBJS)
	gcc -shared $(YAFFSLIBOBJS) -o $@

