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


#include "test_yaffs_rename_dir_over_dir.h"

int  test_yaffs_rename_dir_over_dir(void)
{

	int output=0;

	if (yaffs_mkdir(DIR_PATH,O_CREAT | O_RDWR)==-1){
		print_message("failed to create dir\n",1);
		return -1;
	}
	if (yaffs_mkdir(DIR_PATH2,O_CREAT | O_RDWR)==-1){
		print_message("failed to create dir2\n",1);
		return -1;
	}
	output = yaffs_rename(DIR_PATH ,DIR_PATH2 );
	if (output<0){ 
		print_message("could not rename a directory over a empty directory (which is a bad thing)\n",2);
		return -1;
	} 
		
	return 1;

}


int  test_yaffs_rename_dir_over_dir_clean(void)
{
	return 1;
}

