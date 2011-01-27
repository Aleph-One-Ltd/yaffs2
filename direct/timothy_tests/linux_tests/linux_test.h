/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
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

#ifndef __linux_test_h__
#define __linux_test_h__
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mount.h>
#include "lib.h"
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

typedef struct dir_struct_temp{ 
	node *path_list;
	int number_of_items;
}dir_struct;

int check_dir(dir_struct *old_dir);
dir_struct * scan_dir(void);
int link_test(void);
int symlink_test(void);
int mknod_test(void);
int mkdir_test(void);
int rename_test(void);
int rmdir_test(void);
int remount_test(void);
#endif
