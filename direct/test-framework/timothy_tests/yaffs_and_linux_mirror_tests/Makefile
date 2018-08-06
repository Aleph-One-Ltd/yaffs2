# Makefile for mirror_tests
#
#
# YAFFS: Yet another Flash File System. A NAND-flash specific file system.
#
# Copyright (C) 2002-2018 Aleph One Ltd.
#
#
# Created by Timothy Manning <timothy@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# NB Warning this Makefile does not include header dependencies.
#

YDI_DIR = ../../../
YDI_FRAMEWORK_DIR = ../../
CLEAN_OBJS = mirror_tests emfile-2k-0

TESTFILES = 	mirror_tests.o lib.o\
		linux_test_open.o yaffs_test_open.o\
		linux_test_truncate.o yaffs_test_truncate.o \
		linux_test_unlink.o yaffs_test_unlink.o \
		linux_test_write.o yaffs_test_write.o

all: mirror_tests

YAFFS_TEST_OBJS = $(COMMONTESTOBJS) $(TESTFILES)


ALL_UNSORTED_OBJS += $(YAFFS_TEST_OBJS) $(FUZZER_OBJS)

include $(YDI_FRAMEWORK_DIR)/FrameworkRules.mk


yaffs_test: $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS)
	gcc $(CFLAGS) -o $@ $(YAFFS_TEST_OBJS)



mirror_tests: $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS)
	gcc $(CFLAGS) -o $@ $(YAFFS_TEST_OBJS)

