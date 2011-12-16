/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "linux_test.h"

int random_seed;
int simulate_power_failure = 1;

char message[400];	//this is used for storing print messages.



const struct option long_options[]={
	{"help",	0,NULL,'h'},


	{"print_level",	1,NULL,'p'},
	{"quiet",	0,NULL,'q'},

	{"seed",	1,NULL,'s'},

	{"verbose",	0,NULL,'v'}
};

const char short_options[]="hp:qs:v";


void init(int argc, char *argv[])
{
	char dir[200];
	dir[0]='\0';
	int x=-1;
	char message[100];
	int new_option;

	x=(unsigned)time(NULL);
	sprintf(message,"seeding srand with: %d\n",x);
	print_message(2,message);
	srand(x);



	do {
		new_option=getopt_long(argc,argv,short_options,long_options,NULL);		
		if (new_option=='h'){
			printf("mirror_tests help\n");
			printf("arguments:\n");
			printf("\t-p [NUMBER] //sets the print level for mirror_tests.\n");
			printf("\t-v //verbose mode everything is printed\n");
			printf("\t-q //quiet mode nothing is printed.\n");
			printf("\t-s [number] //seeds rand with the number\n");

			exit(0);
	
		} else if (new_option=='p'){
			set_print_level(atoi(optarg));
		} else if (new_option=='v'){
			set_print_level(5);
		} else if (new_option=='q'){
			set_print_level(-1);
		} else if (new_option=='s'){
			srand(atoi(argv[x+1]));

		} else if (new_option==-1){

		} else if (new_option=='?') {
			printf("bad argument\n");
			exit(0);
		}
	}while(new_option!=-1);
}

int main(int argc, char *argv[])
{
	dir_struct *scanned_dir=NULL;
	int output=0;
	int break_bool=0;
	int x=5;

	init(argc,argv); 
	while( 1){
		while (break_bool!=1){
			//printf("x %d\n",x);
			x--;
			if (x<0 &&(break_bool!=1)){
				output=mkdir_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				output=rmdir_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				output=mknod_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				output=symlink_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				output=link_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				output=rename_test();
				break_bool=1;
			} 
			x--;
			if (x<0 &&(break_bool!=1)){
				scanned_dir=scan_dir();
				
				output=remount_test();
				check_dir(scanned_dir);
				scanned_dir=NULL;	//the scanned dir has been freed in check_dir.
				break_bool=1;
			} 
		}	
		//printf("resetting x\n"); 
		check_function(output);
		break_bool=0;
		x=(rand()% 99);
	}
	return 0;
}

dir_struct * scan_dir(void)
{	
	struct dirent *dir_data;	
	dir_struct *dir=NULL;
	dir=malloc(sizeof(dir_struct));
	memset(dir, 0, sizeof(dir_struct));
	DIR *open_dir=NULL;


	open_dir=opendir(ROOT_PATH);
	if (open_dir < 0){
		sprintf(message,"failed to find the directory: %s",ROOT_PATH);
		print_message(1,message);
	}
	dir_data=readdir(open_dir);
	while(dir_data){
		dir->path_list=linked_list_add_node(HEAD,dir->path_list);
		dir->path_list->string=malloc(strlen(dir_data->d_name)+1);
		strcpy(dir->path_list->string,dir_data->d_name);
		sprintf(message,"opened file: %s\n",dir->path_list->string);
		print_message(5,message);
		dir_data=readdir(open_dir);
	}
	closedir(open_dir);
	//node_print_pointers(dir->path_list);
	return dir;
}

int check_dir(dir_struct *old_dir)
{
	print_message(3,"scanning new dir\n");
	dir_struct *new_dir=scan_dir();
	node *new_list=new_dir->path_list;
	node *old_list=old_dir->path_list;
	int exit_loop=0,error=0;
	print_message(3,"checking dir\n");
	for (;old_list!= NULL;old_list=old_list->next){
		//sprintf(message,"new_list=!NULL= %d, exit_loop !=1 = %d\n",(new_list!=NULL),(exit_loop !=1));
		//print_message(3,message);
		for (;(new_list!=NULL) && (exit_loop !=1);new_list=new_list->next){
			//sprintf(message,"comparing %s  and %s\n",old_list->string,new_list->string);
			//print_message(3,message);
			if (strcmp( new_list->string ,old_list->string)==0){
				//files match -now compare the modes and contents of the files.
				//and set the paths to NULL.
				exit_loop=1;
			}
			/*if (new_list->next==NULL){
				print_message(3,"next is null\n");
				
			}*/
		}
		if (exit_loop !=1){
			//failed to find a matching file
			sprintf(message,"a file has disappeared: %s\n",old_list->string); 
			print_message(3,message);
			error=1;
			
		}
		new_list=new_dir->path_list;
		exit_loop=0;
	}
	//now check if there are any old unmatched files 
	
	//free both data structs
	delete_linked_list(old_dir->path_list);
	delete_linked_list(new_dir->path_list);
	new_dir->path_list=NULL;
	old_dir->path_list=NULL;
	free(old_dir);
	free(new_dir);
	if (error ==1){
		print_message(3,"checking dir failed\n");
		if (get_exit_on_error()==1){
			print_message(3,"exiting_program\n");
			exit(0);
		}
	}

	else if (error !=1){
		print_message(3,"checking dir passed\n");
	}
	return error;
}

int remount_test(void)
{
	int output;
	print_message(3,"\nunmounting\n");
	output=umount2("/mnt/y",1);
	check_function(output);
	print_message(3,"mounting\n");
	mount("/dev/mtdblock0","/mnt/y","yaffs2",0,NULL);
	check_function(output);
	return output;
}

int mkdir_test(void)
{

	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int mode=0,output=0;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));
	mode = ((S_IREAD|S_IWRITE)&random_int());
	sprintf(message,"\nmaking directory: %s, with mode %d\n",string,mode);
	print_message(3,message);
	output= mkdir(string,mode);
	return output;
}

int rmdir_test(void)
{
	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int output=0;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));

	sprintf(message,"\nremoving directory: %s\n",string);
	print_message(3,message);
	output= rmdir(string);
	return output;
}
int symlink_test(void)
{
	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	char string2[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int output;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));
	strcpy(string2,ROOT_PATH);
	strcat(string2,generate_random_string(FILE_NAME_LENGTH));
	sprintf(message,"\nsymlink from: %s, to %s\n",string,string2);
	print_message(3,message);
	output= symlink(string,string2);
	return output;
}
int rename_test(void)
{
	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	char string2[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int output;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));
	strcpy(string2,ROOT_PATH);
	strcat(string2,generate_random_string(FILE_NAME_LENGTH));
	sprintf(message,"\nrenaming from: %s, to %s\n",string,string2);
	print_message(3,message);
	output= rename(string,string2);
	return output;
}
int link_test(void)
{
	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	char string2[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int output=0;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));
	strcpy(string2,ROOT_PATH);
	strcat(string2,generate_random_string(FILE_NAME_LENGTH));
	sprintf(message,"\nlink from: %s, to %s\n",string,string2);
	print_message(3,message);
	output= link(string,string2);
	return output;
}
int mknod_test(void)
{
	char string[FILE_NAME_LENGTH+strlen(ROOT_PATH)];
	int mode=0,dev=0,output=0;
	strcpy(string,ROOT_PATH);
	strcat(string,generate_random_string(FILE_NAME_LENGTH));
	mode = ((S_IREAD|S_IWRITE)&random_int());
	dev = random_int();
	sprintf(message,"\nmaking node: %s, with mode %d, dev %d\n",string,mode,dev);
	print_message(3,message);
	output= mknod(string,mode,dev);
	return output;
}
