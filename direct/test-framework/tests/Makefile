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

TARGETS = yaffs_test fuzzer

all: $(TARGETS)

YAFFS_TEST_OBJS = $(COMMONTESTOBJS) yaffs_test.o nor_stress.o yaffs_fsx.o
FUZZER_OBJS = fuzzer.o

ALL_UNSORTED_OBJS += $(YAFFS_TEST_OBJS) $(FUZZER_OBJS)

include ../FrameworkRules.mk

C_FLAGS += -DCONFIG_YAFFS_USE_PTHREADS

yaffs_test: $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS)
	gcc $(CFLLAG) -o $@ $(YAFFS_TEST_OBJS) -lpthread

fuzzer: $(FUZZER_OBJS)
	gcc $(CFLAGS) -o $@ $<
