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

TARGETS =  yaffs2_image_maker
TARGETS += yaffs2_dumper

all: $(TARGETS)

IMAGEMAKEROBJS = $(COMMONTESTOBJS) yaffs2_image_maker.o
IMAGEDUMPEROBJS = $(COMMONTESTOBJS) yaffs2_dumper.o

ALL_UNSORTED_OBJS += $(IMAGEMAKEROBJS) $(IMAGEDUMPEROBJS)

include ../FrameworkRules.mk

yaffs2_dumper: $(FRAMEWORK_SOURCES) $(IMAGEDUMPEROBJS)
	gcc -o $@ $(IMAGEDUMPEROBJS) -lpthread

yaffs2_image_maker: $(FRAMEWORK_SOURCES) $(IMAGEMAKEROBJS)
	gcc -o $@ $(IMAGEMAKEROBJS) -lpthread


