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

#include "lib.h"
static char message[200];
static int  PRINT_LEVEL = 3;
static int exit_on_error_val =1;
char string[FILE_NAME_LENGTH+1];


int get_exit_on_error(void)
{
	return exit_on_error_val;
}

void set_exit_on_error(int val)
{
	exit_on_error_val=val;
}

node * linked_list_add_node(int pos,node *head_node)
{
	node *new_node=NULL;
	if (pos==HEAD){
		new_node=malloc(sizeof(node));
		memset(new_node, 0, sizeof(node));
		new_node->string=NULL;
		new_node->next=head_node;
		return new_node;
	}
	return NULL;
}

void node_print_pointers(node *current_node)
{
	while (current_node != NULL){
		sprintf(message,"current_node: %p, string: %s next_node: %p\n",current_node,current_node->string,current_node->next);
		print_message(3,message);
		current_node=current_node->next;
	}
}

int delete_linked_list(node *head_node)
{
	node *next_node=NULL;
	node *current_node=head_node;

		while (current_node != NULL){
			next_node=current_node->next;
			free(current_node);
			current_node=next_node;
		}

	return 1;
}

char * generate_random_string(unsigned int length)
{

	unsigned int x;
	for (x=0;x<(length-1);x++)
	{
		string[x]=(rand() % NAME_RANGE)+65;
	}
	string[x]='\0';
	return string;
}

void set_print_level(int new_level)
{
	PRINT_LEVEL=new_level;
}
int get_print_level(void)
{
	return PRINT_LEVEL;
}
void print_message(char print_level,char *message)
{
	if (print_level <= PRINT_LEVEL){
		printf("%s",message);
	}
}
int random_int(void)
{
	return (random()%1000000); 
}

void check_function(int output)
{
	if (output>=0){
		print_message(3,"test_passed\n");
	} else {
		print_message(3,"test_failed\n");
		get_error_linux();
	}
}

void get_error_linux(void)
{
	int error_code=0;
	char message[30];
	message[0]='\0';

	error_code=errno;
	sprintf(message,"linux_error code %d\n",error_code);
	print_message(1,message);
	
	strcpy(message,"error is");
	perror(message);
//	sprintf(message,"error is : %s\n",yaffs_error_to_str(error_code));
	//perror(message);	
	//print_message(1,message);
}
