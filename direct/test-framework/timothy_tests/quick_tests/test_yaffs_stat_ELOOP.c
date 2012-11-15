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

#include "test_yaffs_stat_ELOOP.h"

int test_yaffs_stat_ELOOP(void)
{
	int error_code=0;
	struct yaffs_stat stat;
	int output=0;
	char text[100];
	text[0] ='\0';

	if (set_up_ELOOP()<0){
		print_message("failed to setup symlinks\n",2);
		return -1;
	}
	output=yaffs_stat(ELOOP_PATH, &stat);;
	if (output<0){ 
		error_code=yaffs_get_error();
		if (abs(error_code)==ELOOP){
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

int test_yaffs_stat_ELOOP_clean(void)
{
	return 1;
}
