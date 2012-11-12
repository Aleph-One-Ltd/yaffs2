/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __lib_h__
#define __lib_h__
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NAME_RANGE 10
#define ROOT_PATH "/mnt/y/"
#define FILE_NAME_LENGTH 3
#define HEAD 0
#define TAIL 1

typedef struct node_temp{
	char *string;
	struct node_temp *next; 
}node;

int get_exit_on_error(void);
void set_exit_on_error(int val);
int delete_linked_list(node *head_node);
node * linked_list_add_node(int pos,node *head_node);
int random_int(void);
char * generate_random_string(unsigned int length);
void get_error_linux(void);
void check_function(int output);
void print_message(char print_level, char *message);
void set_print_level(int new_level);
int get_print_level(void);
void node_print_pointers(node *current_node);
#endif
