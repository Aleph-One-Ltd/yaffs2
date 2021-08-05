/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef _lib_h__
#define _lib_h__

#include <stdio.h>
#include <string.h>
#include "yaffsfs.h"


#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_NAME "test_dir/foo"
#define FILE_SIZE 10

#define YAFFS_TEST_LONG_VALUE (100000000000000000LL)

#define FILE_MODE (S_IREAD | S_IWRITE)
#define FILE_SIZE_TRUNCATED 100
#define FILE_TEXT "file foo "	/* keep space at end of string */
#define FILE_TEXT_NBYTES 10
#define TEST_DIR "/yflash2/test_dir"
#define DIR_PATH "/yflash2/test_dir/new_directory"
#define DIR_PATH2 "/yflash2/test_dir/new_directory2"
#define DIR_PATH2_FILE "/yflash2/test_dir/new_directory2/foo"
#define SYMLINK_PATH "/yflash2/test_dir/sym_foo"

#define HARD_LINK_PATH "/yflash2/test_dir/hard_link"

#define NODE_PATH "/yflash2/test_dir/node"

#define RENAME_PATH "/yflash2/test_dir/foo2"

#define RENAME_DIR_PATH "/yflash2/test_dir/dir2"

#define ELOOP_PATH "/yflash2/test_dir/ELOOP"
#define ELOOP2_PATH "/yflash2/test_dir/ELOOP2"

#define RMDIR_PATH "/yflash2/test_dir/RM_DIR"

/* warning do not define anything as FILE because there seems to be a conflict with stdio.h */ 
#define FILE_PATH "/yflash2/test_dir/foo"
#define FILE_PATH2 "/yflash2/test_dir/foo2"

void join_paths(char *path1,char *path2,char *new_path );
void print_message(char *message,char print_level);
void set_print_level(int new_level);
void set_exit_on_error(int num);
int get_exit_on_error(void);
int set_up_ELOOP(void);
int EROFS_setup(void);
int EROFS_clean(void);
int delete_dir(char *dir_name);
void get_error(void);
#endif
