/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __yaffs_test_truncate_h__
#define __yaffs_test_truncate_h__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "yaffsfs.h"
#include "lib.h"

int yaffs_test_truncate(arg_temp *args_struct);

#endif
