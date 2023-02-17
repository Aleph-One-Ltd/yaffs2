/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>


#include "yaffsfs.h"

#include "yaffs_guts.h" /* Only for dumping device innards */
#include "yaffs_endian.h" /*For testing the swap_u64 macro */

extern int yaffs_trace_mask;
int random_seed;
int simulate_power_failure;

void unlink_fill_test(const char *mountpt, int close_type)
{

	char fn[100];
	int i;
	int h;
	int j;
	int close_result;
	int unlink_result;

	yaffs_start_up();
	yaffs_mount(mountpt);

	printf("Starting test\n");
	if (close_type == 0)
		printf("Let handle table get full before closing some handles\n");
	if (close_type == 1)
		printf("Closing before unlink\n");
	else if (close_type == 2)
		printf("Closing after unlink\n");
	else
		printf("Not closing any handles\n");

	for(i = 0; i < 200; i++) {
		sprintf(fn,"%s/a",mountpt);
		h = yaffs_open(fn, O_CREAT | O_TRUNC | O_RDWR, 0666);

		if (close_type == 1) /* Close before the unlink */
			close_result = yaffs_close(h);

		unlink_result = yaffs_unlink(fn);

		if (close_type == 2) /* Close after the unlink. */
			close_result = yaffs_close(h);

		printf("%6d: Opened OK, h = %d, close result = %d, unlink_result = %d\n",
			i, h, close_result, unlink_result);

		if (h < 0) {
			printf("Error opening file. h = %d error %d\n", h, yaffs_get_error());
			if (close_type != 0)
				exit(1);
			printf("Closing some handles so that table is not full any more\n");
			for(j = 0; j < 20; j++)
				yaffs_close(j);
			printf("Continuing with handles 0..19 closed. These will be reused.\n");
		}
	}

	printf("Test completed.\n");

}



int main(int argc, char *argv[])
{
	int close_type = 99;

	if(argc < 2 ||
	   sscanf(argv[1], "%d", &close_type) != 1) {
	   	printf("Specify a close type argument 0, 1, 2 or 3\n");
	   	exit(1);
	}


	yaffs_trace_mask = 0;

	random_seed = time(NULL);

	unlink_fill_test("/nand", close_type);

	return 0;

}
