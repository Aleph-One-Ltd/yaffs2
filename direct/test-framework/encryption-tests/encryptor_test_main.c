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
 *
 * This is a test harness for a Yaffs encryptor.
 * This allows the encryptor to be tested without the whole of Yaffs.
 *
 * This works by setting up a very stripped down yaffs_dev that has a
 * fake driver layer and can prod and observe the encryptor.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "encryptor_test.h"

int random_seed;
int verbose;

int sleep_exit = 0;
int sleep_time = 0;


void *yaffsfs_malloc(size_t size)
{
	return malloc(size);
}

void yaffsfs_free(void *ptr)
{
	free(ptr);
}

void test_fatal(void)
{
	printf("fatal yaffs test pid %d sleeping\n",getpid());
	while(!sleep_exit){
		sleep(1);
		sleep_time++;
	}

	signal(SIGSEGV,SIG_IGN);
	signal(SIGBUS,SIG_IGN);
	signal(SIGABRT,SIG_IGN);
	sleep_exit = 0;
}

void bad_ptr_handler(int sig)
{
	printf("signal %d received\n",sig);
	test_fatal();
}

static void bad_usage(const char * str)
{
	fprintf(stderr, "%s\n", str);
	fprintf(stderr, "Usage: encryptor_test [options]\n");
	fprintf(stderr, "options\n");
	fprintf(stderr, " d  data_byes: Size of data page\n");
	fprintf(stderr, " t  type: \"inband\" or \"oob\"\n");
	fprintf(stderr, " e encryptor: encryptor id\n");
	fprintf(stderr, " v: verbose\n");
	exit(2);
}

int main(int argc, char **argv)
{
	int ch;
	int total_data_bytes;
	int inband_tags;
	char *tag_type = "";
	char *selected_encryptor = "";
	int ret;

	signal(SIGSEGV,bad_ptr_handler);
	signal(SIGBUS,bad_ptr_handler);
	signal(SIGABRT,bad_ptr_handler);
	while ((ch = getopt(argc,argv, "d:t:y:e:v")) != EOF)
		switch (ch) {
		case 'd':
			total_data_bytes = atoi(optarg);
			break;
		case 't':
			tag_type = strdup(optarg);
			break;
		case 'e':
			selected_encryptor = strdup(optarg);
			break;
		case 'v':
			verbose=1;
			break;
		default:
			bad_usage("Unknown argument");
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (total_data_bytes < 512)
		bad_usage("Need a data bytes argument >= 512");

	if(strcmp(tag_type, "inband") == 0)
		inband_tags = 1;
	else if (strcmp(tag_type, "oob") == 0)
		inband_tags = 0;
	else
		bad_usage("Need a valid tag type (oob or inband)");

	if(strcmp(selected_encryptor, "mbedtls") &&
	   strcmp(selected_encryptor, "unencrypted"))
		bad_usage("Need a valid encryptor (mbedtls, unencrypted)");

	ret = encryptor_test(total_data_bytes,
			     inband_tags,
			     selected_encryptor);

	if (ret < 0) {
		bad_usage("Failed to create fake dev, try different parameters.");
	}
	return 0;
}


