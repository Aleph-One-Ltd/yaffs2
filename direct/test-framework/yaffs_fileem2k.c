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
 * This provides a YAFFS nand emulation on a file for emulating 2kB pages.
 * This is only intended as test code to test persistence etc.
 */

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_flashif2.h"
#include "yaffs_guts.h"
#include "yaffs_fileem2k.h"
#include "yaffs_packedtags2.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if 1

#define SIZE_IN_MB 32
/* #define SIZE_IN_MB 128 */

#define PAGE_DATA_SIZE (2048)
#define PAGE_SPARE_SIZE  (64)
#define PAGE_SIZE  (PAGE_DATA_SIZE + PAGE_SPARE_SIZE)
#define PAGES_PER_BLOCK (64)
#define BLOCK_DATA_SIZE (PAGE_DATA_SIZE * PAGES_PER_BLOCK)
#define BLOCK_SIZE (PAGES_PER_BLOCK * (PAGE_SIZE))
#define BLOCKS_PER_MB ((1024*1024)/BLOCK_DATA_SIZE)
#define SIZE_IN_BLOCKS (BLOCKS_PER_MB * SIZE_IN_MB)

#else

#define SIZE_IN_MB 128
#define PAGE_DATA_SIZE (512)
#define SPARE_SIZE  (16)
#define PAGE_SIZE  (PAGE_DATA_SIZE + SPARE_SIZE)
#define PAGES_PER_BLOCK (32)
#define BLOCK_DATA_SIZE (PAGE_SIZE * PAGES_PER_BLOCK)
#define BLOCK_SIZE (PAGES_PER_BLOCK * (PAGE_SIZE))
#define BLOCKS_PER_MB ((1024*1024)/BLOCK_DATA_SIZE)
#define SIZE_IN_BLOCKS (BLOCKS_PER_MB * SIZE_IN_MB)

#endif

#define REPORT_ERROR 0

typedef struct
{
	u8 data[PAGE_SIZE]; // Data + spare
} yflash_Page;

typedef struct
{
	yflash_Page page[PAGES_PER_BLOCK]; // The pages in the block

} yflash_Block;



#define MAX_HANDLES 20
#define BLOCKS_PER_HANDLE (32*8)

typedef struct
{
	int handle[MAX_HANDLES];
	int nBlocks;
} yflash_Device;

static yflash_Device filedisk;

int yaffs_test_partial_write = 0;

extern int random_seed;
extern int simulate_power_failure;
static int remaining_ops;
static int nops_so_far;

int ops_multiplier;

static void yflash2_MaybePowerFail(unsigned int nand_chunk, int failPoint)
{

   nops_so_far++;


   remaining_ops--;
   if(simulate_power_failure &&
      remaining_ops < 1){
       printf("Simulated power failure after %d operations\n",nops_so_far);
       printf("  power failed on nand_chunk %d, at fail point %d\n",
       			nand_chunk, failPoint);
    	exit(0);
  }
}

static char *NToName(char *buf,int n)
{
	sprintf(buf,"emfile-2k-%d",n);
	return buf;
}

static char dummyBuffer[BLOCK_SIZE];

static int GetBlockFileHandle(int n)
{
	int h;
	int requiredSize;

	char name[40];
	NToName(name,n);
	int fSize;
	int i;

	h =  open(name, O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if(h >= 0){
		fSize = lseek(h,0,SEEK_END);
		requiredSize = BLOCKS_PER_HANDLE * BLOCK_SIZE;
		if(fSize < requiredSize){
		   for(i = 0; i < BLOCKS_PER_HANDLE; i++)
		   	if(write(h,dummyBuffer,BLOCK_SIZE) != BLOCK_SIZE)
				return -1;

		}
	}

	return h;

}

static int  CheckInit(void)
{
	static int initialised = 0;
	int i;

	int blk;


	if(initialised)
	{
		return YAFFS_OK;
	}

	initialised = 1;


	srand(random_seed);
	remaining_ops = (rand() % 1000) * 5;
  	memset(dummyBuffer,0xff,sizeof(dummyBuffer));



	filedisk.nBlocks = SIZE_IN_MB * BLOCKS_PER_MB;

	for(i = 0; i <  MAX_HANDLES; i++)
		filedisk.handle[i] = -1;

	for(i = 0,blk = 0; blk < filedisk.nBlocks; blk+=BLOCKS_PER_HANDLE,i++)
		filedisk.handle[i] = GetBlockFileHandle(i);


	return 1;
}


int yflash2_GetNumberOfBlocks(void)
{
	CheckInit();

	return filedisk.nBlocks;
}


int yaffs_check_all_ff(const u8 *ptr, int n)
{
	while(n)
	{
		n--;
		if(*ptr!=0xFF) return 0;
		ptr++;
	}
	return 1;
}


static int yflash2_WriteChunk(struct yaffs_dev *dev, int nand_chunk,
				   const u8 *data, int data_len,
				   const u8 *oob, int oob_len)
{
	int retval = YAFFS_OK;
	int pos;
	int h;
	int nwritten;

	(void) dev;

	if (data && data_len) {
		pos = (nand_chunk % (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE)) * PAGE_SIZE;
		h = filedisk.handle[(nand_chunk / (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE))];
		lseek(h,pos,SEEK_SET);
		nwritten = write(h,data,data_len);
		if(nwritten != data_len)
			retval = YAFFS_FAIL;
	}

	if (oob && oob_len) {
		pos = (nand_chunk % (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE)) * PAGE_SIZE + PAGE_DATA_SIZE;
		h = filedisk.handle[(nand_chunk / (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE))];
		lseek(h,pos,SEEK_SET);
		nwritten = write(h,oob,oob_len);
		if(nwritten != oob_len)
			retval = YAFFS_FAIL;
	}

	yflash2_MaybePowerFail(nand_chunk,3);

	return retval;
}

static int yflash2_ReadChunk(struct yaffs_dev *dev, int nand_chunk,
				   u8 *data, int data_len,
				   u8 *oob, int oob_len,
				   enum yaffs_ecc_result *ecc_result)
{
	int retval = YAFFS_OK;
	int pos;
	int h;
	int nread;

	(void) dev;

	if (data && data_len) {
		pos = (nand_chunk % (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE)) * PAGE_SIZE;
		h = filedisk.handle[(nand_chunk / (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE))];
		lseek(h,pos,SEEK_SET);
		nread = read(h,data,data_len);
		if(nread != data_len)
			retval = YAFFS_FAIL;
	}

	if (oob && oob_len) {
		pos = (nand_chunk % (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE)) * PAGE_SIZE + PAGE_DATA_SIZE;
		h = filedisk.handle[(nand_chunk / (PAGES_PER_BLOCK * BLOCKS_PER_HANDLE))];
		lseek(h,pos,SEEK_SET);
		nread = read(h,oob,oob_len);
		if(nread != oob_len)
			retval = YAFFS_FAIL;
	}

	if (ecc_result)
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return retval;
}

static int yflash2_EraseBlock(struct yaffs_dev *dev, int block_no)
{
	u32 i;
	int h;

	CheckInit();

	if(block_no < 0 || block_no >= filedisk.nBlocks)
	{
		yaffs_trace(YAFFS_TRACE_ALWAYS,"Attempt to erase non-existant block %d",block_no);
		return YAFFS_FAIL;
	}
	else
	{

		u8 pg[PAGE_SIZE];
		int syz = PAGE_SIZE;

		memset(pg,0xff,syz);


		h = filedisk.handle[(block_no / ( BLOCKS_PER_HANDLE))];
		lseek(h,((block_no % BLOCKS_PER_HANDLE) * dev->param.chunks_per_block) * PAGE_SIZE,SEEK_SET);
		for(i = 0; i < dev->param.chunks_per_block; i++)
		{
			write(h,pg,PAGE_SIZE);
		}

		return YAFFS_OK;
	}
}

static int yflash2_MarkBad(struct yaffs_dev *dev, int block_no)
{
	int written;
	int h;

	struct yaffs_packed_tags2 pt;

	CheckInit();

	memset(&pt,0,sizeof(pt));
	h = filedisk.handle[(block_no / ( BLOCKS_PER_HANDLE))];
	lseek(h,((block_no % BLOCKS_PER_HANDLE) * dev->param.chunks_per_block) * PAGE_SIZE + PAGE_DATA_SIZE,SEEK_SET);
	written = write(h,&pt,sizeof(pt));

	if(written != sizeof(pt))
		return YAFFS_FAIL;

	return YAFFS_OK;

}

static int yflash2_CheckBad(struct yaffs_dev *dev, int block_no)
{
	(void) dev;
	(void) block_no;

	return YAFFS_OK;
}

static int yflash2_Initialise(struct yaffs_dev *dev)
{
	(void) dev;

	CheckInit();

	return YAFFS_OK;
}

struct yaffs_dev *yflash2_install_drv(const char *name)
{
	struct yaffs_dev *dev = NULL;
	struct yaffs_param *param;
	struct yaffs_driver *drv;

	dev = malloc(sizeof(*dev));

	if(!dev)
		return NULL;

	memset(dev, 0, sizeof(*dev));

	dev->param.name = strdup(name);

	if(!dev->param.name) {
		free(dev);
		return NULL;
	}

	drv = &dev->drv;

	drv->drv_write_chunk_fn = yflash2_WriteChunk;
	drv->drv_read_chunk_fn = yflash2_ReadChunk;
	drv->drv_erase_fn = yflash2_EraseBlock;
	drv->drv_mark_bad_fn = yflash2_MarkBad;
	drv->drv_check_bad_fn = yflash2_CheckBad;
	drv->drv_initialise_fn = yflash2_Initialise;

	param = &dev->param;

	param->total_bytes_per_chunk = 2048;
	param->chunks_per_block = 64;
	param->start_block = 0;
	param->end_block = yflash2_GetNumberOfBlocks()-1;
	param->is_yaffs2 = 1;
	param->use_nand_ecc=1;

	param->n_reserved_blocks = 5;
	param->wide_tnodes_disabled=0;
	param->refresh_period = 1000;
	param->n_caches = 10; // Use caches

	param->enable_xattr = 1;

	/* dev->driver_context is not used by this simulator */

	yaffs_add_device(dev);

	return dev;
}
