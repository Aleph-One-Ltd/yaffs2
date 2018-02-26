/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Nand simulator modelled on a Samsung K9K2G08U0A 8-bit
 *
 *  Need to implement basic commands first:
 *  Status
 *  Reset
 *  Read
 *  Random read (ie set read position within current page)
 *  Write
 *  Erase
 */

#include "nandstore_file.h"
#include "nand_store.h"
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern int random_seed;
extern int simulate_power_failure;
static int remaining_ops;
static int nops_so_far;

struct nandstore_file_private {
	char backing_file[500];
	int handle;
	unsigned char * buffer;
	int buff_size;
};

static void maybe_power_fail(unsigned int nand_chunk, int fail_point)
{

   nops_so_far++;

   remaining_ops--;
   if(simulate_power_failure &&
      remaining_ops < 1){
       printf("Simulated power failure after %d operations\n",nops_so_far);
       printf("  power failed on nand_chunk %d, at fail point %d\n",
       			nand_chunk, fail_point);
    	exit(0);
  }
}

static void power_fail_init(void)
{
	remaining_ops = (rand() % 1000) * 5;
}

static int nandstore_file_store(struct nand_store *this, int page,
				const unsigned char * buffer)
{
	struct nandstore_file_private *nsfp =
		(struct nandstore_file_private *)this->private_data;
	int pos = nsfp->buff_size * page;
	int i;

	maybe_power_fail(page, __LINE__);

	lseek(nsfp->handle, pos, SEEK_SET);
	read(nsfp->handle, nsfp->buffer, nsfp->buff_size);
	for(i = 0; i < nsfp->buff_size; i++)
		nsfp->buffer[i] &= buffer[i];
	lseek(nsfp->handle, pos, SEEK_SET);
	write(nsfp->handle, nsfp->buffer, nsfp->buff_size);

	maybe_power_fail(page, __LINE__);

	return 0;
}

static int nandstore_file_retrieve(struct nand_store *this, int page,
				unsigned char * buffer)
{
	struct nandstore_file_private *nsfp =
		(struct nandstore_file_private *)this->private_data;

	lseek(nsfp->handle, nsfp->buff_size * page, SEEK_SET);
	read(nsfp->handle, buffer, nsfp->buff_size);
	return 0;
}
static int nandstore_file_erase(struct nand_store *this, int page)
{
	int block = page / this->pages_per_block;
	struct nandstore_file_private *nsfp =
		(struct nandstore_file_private *)this->private_data;
	int i;

	maybe_power_fail(page, __LINE__);

	lseek(nsfp->handle,
		block * nsfp->buff_size * this->pages_per_block, SEEK_SET);
	memset(nsfp->buffer, 0xff, nsfp->buff_size);
	for(i = 0; i < this->pages_per_block; i++)
		write(nsfp->handle, nsfp->buffer, nsfp->buff_size);
	return 0;
}

static int nandstore_file_shutdown(struct nand_store *this)
{
	struct nandstore_file_private *nsfp =
		(struct nandstore_file_private *)this->private_data;
	close(nsfp->handle);
	nsfp->handle = -1;
	return 0;
}

struct nand_store *
nandstore_file_init(const char *fname,
				int blocks,
				int pages_per_block,
				int data_bytes_per_page,
				int spare_bytes_per_page)
{
	int fsize;
	int nbytes;
	int i;
	struct nand_store *ns;
	struct nandstore_file_private *nsfp;
	unsigned char *buffer;
	int buff_size = data_bytes_per_page + spare_bytes_per_page;

	ns = malloc(sizeof(struct nand_store));
	nsfp = malloc(sizeof(struct nandstore_file_private));
	buffer = malloc(buff_size);


	if (!ns || !nsfp || !buffer) {
		free(ns);
		free(nsfp);
		free(buffer);
		return NULL;
	}

	memset(ns, 0, sizeof(*ns));
	memset(nsfp, 0, sizeof(*nsfp));
	nsfp->buffer = buffer;
	nsfp->buff_size = buff_size;
	ns->private_data = nsfp;

	ns->store = nandstore_file_store;
	ns->retrieve = nandstore_file_retrieve;
	ns->erase = nandstore_file_erase;
	ns->shutdown = nandstore_file_shutdown;

	ns->blocks = blocks;
	ns->pages_per_block = pages_per_block;
	ns->data_bytes_per_page = data_bytes_per_page;
	ns->spare_bytes_per_page = spare_bytes_per_page;

	strncpy(nsfp->backing_file, fname, sizeof(nsfp->backing_file));

	nsfp->handle = open(nsfp->backing_file, O_RDWR | O_CREAT, 0666);
	if(nsfp->handle >=0){
		fsize = lseek(nsfp->handle,0,SEEK_END);
		nbytes = ns->blocks * ns->pages_per_block *
			(ns->data_bytes_per_page + ns->spare_bytes_per_page);
		if (fsize != nbytes) {
			printf("Initialising backing file.\n");
			ftruncate(nsfp->handle,0);
			for(i = 0; i < ns->blocks; i++)
				nandstore_file_erase(ns,
					i * ns->pages_per_block);
		}
	}

	power_fail_init();

	return ns;


}

