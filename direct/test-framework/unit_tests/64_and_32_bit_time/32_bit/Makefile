# Makefile for 64 bit time test.
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

YDI_DIR = ../../../../
YDI_FRAMEWORK_DIR = ../../../

MAINFILES = time_32_tests create_32_bit validate_32_bit
MAIN_OBJS = $(addsuffix .o,$(MAINFILES))

EXTRA_OBJS = shared.o

CLEAN_OBJS = $(MAINFILES) 
CFLAGS = -DCONFIG_YAFFS_USE_32_BIT_TIME_T

YAFFS_TEST_OBJS = $(COMMONTESTOBJS) $(EXTRA_OBJS)
ALL_UNSORTED_OBJS += $(YAFFS_TEST_OBJS) $(MAIN_OBJS)

all: $(MAINFILES) 

include $(YDI_FRAMEWORK_DIR)/FrameworkRules.mk

phony. test: time_32_tests
	./time_32_tests

$(MAINFILES): $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS) $(MAIN_OBJS) 
	gcc $(CFLAGS) -o $@ $(YAFFS_TEST_OBJS) $@.o -lpthread -DCONFIG_YAFFS_USE_32_BIT_TIME_T

