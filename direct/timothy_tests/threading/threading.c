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

#include "threading.h"

int random_seed;
int simulate_power_failure = 0;

const struct option long_options[]={
	{"help",	0,NULL,'h'},
	{"threads",	1,NULL,'t'}
};

const char short_options[]="ht:";




void main_init(int argc, char *argv[])
{
	int new_option;
	int x=0;
	int new_num_of_threads=5;	
	x=(unsigned)time(NULL);
	printf("seeding srand with: %d\n",x);
	srand(x);
	do{
		new_option=getopt_long(argc,argv,short_options,long_options,NULL);		
		if (new_option=='h'){
			printf("help\n");
			printf("-h will print the commands available\n");
			printf("-t [number] sets the number of threads\n");
			exit(0);
		} else if (new_option=='t') {
			new_num_of_threads=atoi(optarg);
		}
	}while (new_option!=-1);
	number_of_threads(new_num_of_threads);
	init_counter(new_num_of_threads);

}

int main(int argc, char *argv[])
{
	main_init(argc,argv);
	pthread_t threads[get_num_of_threads()];
	unsigned int x=0;
	int output=0;
	int y=0;

	for (x=0;x<get_num_of_threads();x++)
	{
		
		output=pthread_create(&threads[x], NULL,thread_function, (void *)x );
		if (output>0){
			printf("failed to create thread %d. Error is %d\n",x,output);
		}
		
	}
	while (1){
		y=0;
		printf("thread counter: %d ",get_counter(y));
		for (y=1;y<get_num_of_threads();y++){
			printf("| %d ",get_counter(y));
		}
		printf("\n");
		sleep(1);
	}

}

