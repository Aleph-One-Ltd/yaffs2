/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "test_yaffs_stat_ENAMETOOLONG.h"

int test_yaffs_stat_ENAMETOOLONG(void)
{
	int error_code=0;
	struct yaffs_stat stat;
	int output=0;
	char text[100];
	text[0] ='\0';

	int file_name_length=1000000;
	char file_name[file_name_length];
	int x=0;
	for (x=0; x<file_name_length -1; x++){
		file_name[x]='a';
	}
	file_name[file_name_length-2]='\0';
	

	output=yaffs_stat(file_name, &stat);;
	if (output<0){ 
		error_code=yaffs_get_error();
		if (abs(error_code)==ENAMETOOLONG){
			return 1;
		} else {
			print_message("returned error does not match the the expected error\n",2);
			return -1;
		}
	} else {
		print_message("stated a ELOOP (which is a bad thing)\n",2);
		return -1;
	}	
}

int test_yaffs_stat_ENAMETOOLONG_clean(void)
{
	return 1;
}
