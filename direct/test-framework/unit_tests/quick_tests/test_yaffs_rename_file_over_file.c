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


#include "test_yaffs_rename_file_over_file.h"

int  test_yaffs_rename_file_over_file(void)
{

	int output=0;

	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	if (yaffs_close(yaffs_open(FILE_PATH2,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	output = yaffs_rename(FILE_PATH ,FILE_PATH2 );
	if (output<0){ 
		print_message("could not rename a file over a file (which is a bad thing)\n",2);
		return -1;
	} 
		
	return 1;

}


int  test_yaffs_rename_file_over_file_clean(void)
{
	return 1;
}

