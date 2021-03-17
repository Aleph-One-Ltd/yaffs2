# Makefile for YAFFS direct test
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

TARGETS = yaffs2_image_maker

all: $(TARGETS)

DIRECTTESTOBJS = $(COMMONTESTOBJS) yaffs2_image_maker.o

ALL_UNSORTED_OBJS += $(DIRECTTESTOBJS)

include ../FrameworkRules.mk


yaffs2_image_maker: $(FRAMEWORK_SOURCES) $(DIRECTTESTOBJS)
	gcc -o $@ $(DIRECTTESTOBJS) -lpthread

