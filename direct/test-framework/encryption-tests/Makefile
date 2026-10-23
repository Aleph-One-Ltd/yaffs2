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

TARGETS = encryptor_test

all: $(TARGETS)

ENCRYPTOR_TEST_OBJS = encryptor_test_main.o\
                      encryptor_test.o\
                      yaffs_encryption.o\
		      yaffs_encrypt_mbedtls.o

ALL_UNSORTED_OBJS += $(ENCRYPTOR_TEST_OBJS)

include ../FrameworkRules.mk

C_FLAGS += -DCONFIG_YAFFS_USE_PTHREADS

encryptor_test: $(FRAMEWORK_SOURCES) $(ENCRYPTOR_TEST_OBJS)
	gcc $(CFLLAG) -o $@ $(ENCRYPTOR_TEST_OBJS) -lpthread -lmbedtls -lmbedcrypto
