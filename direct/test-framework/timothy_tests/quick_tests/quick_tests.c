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

#include "quick_tests.h"


int random_seed;
int simulate_power_failure = 0;




static int number_of_random_tests=0;
static unsigned int num_of_tests_pass=0;
static unsigned int num_of_tests_failed=0;
static unsigned int total_number_of_tests=(sizeof(test_list)/sizeof(test_template));


const struct option long_options[]={
	{"help",	0,NULL,'h'},
	{"quiet",	0,NULL,'q'},
	{"number",	1,NULL,'n'},
	{"trace",	1,NULL,'t'},
	{"continue",	0,NULL,'c'},
	{"verbose",	0,NULL,'v'}
};

const char short_options[]="hqn:t:cv";



int main(int argc, char *argv[])
{
	int x=0;

	init_quick_tests(argc, argv);
	logical_run_of_tests();
	for (x=0;x<number_of_random_tests;x ++){
		run_random_test_loop();
	}	
	/*this is where the loop should break to*/
	quit_quick_tests(0);
	
}


void run_random_test_loop(void)
{
	int id=0;
	unsigned int x=0;
	//int run_list[total_number_of_tests];
	for (x=0;x<total_number_of_tests;x++){ 
		id = (rand() % (total_number_of_tests-1));
		run_test(id); 	
	}
}

void logical_run_of_tests(void)
{
	unsigned int x=0;
	print_message("\n\nrunning quick tests for yaffs\n\n", 0);

	for (x=0;x<total_number_of_tests;x++){
		run_test(x);
	}
}

void run_test(int x)
{
	int output=0;
	int y= 0;
	char message[200];
	message[0]='\0';

	yaffs_mkdir(TEST_DIR,S_IWRITE | S_IREAD);
	yaffs_set_error(0);	/*reset the last error to 0 */
	//printf("foo exists %d\n",test_yaffs_open());
	sprintf(message,"\nrunning test: %s \n",test_list[x].name_of_test);
	print_message(message,3);
	output=test_list[x].p_function();	/*run test*/
	if (output>=0){
		/*test has passed*/
		sprintf(message,"\ttest %s passed\n",test_list[x].name_of_test);
		print_message(message,3); 
		num_of_tests_pass++;
	} else {
		/*test is assumed to have failed*/
		printf("test failed\n");
		sprintf(message,"test: %s failed\n",test_list[x].name_of_test);
		print_message(message,1);		
		num_of_tests_failed ++;	
	
		get_error();
		print_message("\n\n",1);
		if (get_exit_on_error()){	
			quit_quick_tests(1);
		}
	}
	output=0;
	output=test_list[x].p_function_clean();	/*clean the test*/
	if (output <0){
		/* if the test failed to clean it's self then */
		sprintf(message,"test: %s failed to clean\n",test_list[x].name_of_test);
		print_message(message,1);		
		num_of_tests_failed ++;	
		num_of_tests_pass--;
		get_error();
		printf("\n\n");
		if (get_exit_on_error()){
			quit_quick_tests(1);
		}
		
	} else {
		sprintf(message,"\ttest clean: %s passed\n",test_list[x].name_of_test);
		print_message(message,3);
	}
	/* close all open handles */
	for (y=0; y<100;y++){
		yaffs_close(y);
	}
	delete_dir(TEST_DIR);
}

void quit_quick_tests(int exit_code)
{
	/*char message[30];
	message[0]='\0';
	*/	
	if (num_of_tests_pass==total_number_of_tests &&  num_of_tests_failed==0){
		printf("\t OK --all tests passed\n");
	}
	printf("out of %d tests, %d ran: %d passed and %d failed\n\n\n", total_number_of_tests,(num_of_tests_pass+num_of_tests_failed) ,num_of_tests_pass,num_of_tests_failed);
	yaffs_unmount(YAFFS_MOUNT_POINT);
	exit(exit_code);
}


void init_quick_tests(int argc, char *argv[])
{
	int trace=0;
	int new_option;
	//int x=0;	
	do{
		new_option=getopt_long(argc,argv,short_options,long_options,NULL);		
		if (new_option=='h'){
			printf("help\n");
			printf("-h will print the commands available\n");
			printf("-c will continue after a test fails else the program will exit\n");
			printf("-v will print all messages\n");
			printf("-q quiet mode only the number of tests passed and failed will be printed\n");
			printf("-t [number] set yaffs_trace to number\n");
			printf("-n [number] sets the number of random loops to run after the the test has run\n");
			exit(0);
		} else if (new_option=='c') {
			set_exit_on_error(0);
		} else if (new_option=='q') {
			set_print_level(-3);
		} else if (new_option=='t') {
			trace = atoi(optarg);
		}  else if (new_option=='v') {
			set_print_level(5);
		} else if (new_option=='n') {
			number_of_random_tests=atoi(optarg);
		}

	}while (new_option!=-1);
	yaffs_start_up();
	yaffs_set_trace(trace);

}
