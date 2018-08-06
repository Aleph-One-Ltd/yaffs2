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
 * NAND Simulator for testing YAFFS
 */


#include "yramsim.h"

#include "yaffs_guts.h"
#include <string.h>



#define DATA_SIZE	2048
#define SPARE_SIZE	64
#define PAGE_SIZE	(DATA_SIZE + SPARE_SIZE)
#define PAGES_PER_BLOCK	64


typedef struct {
	unsigned char page[PAGES_PER_BLOCK][PAGE_SIZE];
	unsigned blockOk;
} Block;

typedef struct {
	Block **blockList;
	int nBlocks;
} SimData;


SimData *simDevs[N_RAM_SIM_DEVS];

static SimData *DevToSim(struct yaffs_dev *dev)
{
	return (SimData*)(dev->driver_context);
}


static void CheckInitialised(void)
{

}

static int yramsim_erase_internal(SimData *sim, unsigned blockId,int force)
{
	if(blockId < 0 || blockId >= sim->nBlocks){
		return 0;
	}

	if(!sim->blockList[blockId]){
		return 0;
	}

	if(!force && !sim->blockList[blockId]->blockOk){
		return 0;
	}

	memset(sim->blockList[blockId],0xff,sizeof(Block));
	sim->blockList[blockId]->blockOk = 1;

	return 1;
}




static int yramsim_initialise(struct yaffs_dev *dev)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	return blockList != NULL;
}


static int yramsim_deinitialise(struct yaffs_dev *dev)
{
	return 1;
}

static int yramsim_rd_chunk (struct yaffs_dev *dev, int pageId,
					  u8 *data, int dataLength,
					  u8 *spare, int spareLength,
					  enum yaffs_ecc_result *ecc_result)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;
	unsigned char * d;
	unsigned char *s;
	if(blockId >= sim->nBlocks ||
	   pageOffset >= PAGES_PER_BLOCK ||
	   dataLength >DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !blockList[blockId]->blockOk){
		   return YAFFS_FAIL;
	}

	d = blockList[blockId]->page[pageOffset];
	s = d + DATA_SIZE;

	if(data)
		memcpy(data,d,dataLength);

	if(spare)
		memcpy(spare,s,spareLength);

	if (ecc_result) 
		*ecc_result  = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;
}

static int yramsim_wr_chunk (struct yaffs_dev *dev, int pageId,
					   const u8 *data, int dataLength,
					   const u8 *spare, int spareLength)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;
	unsigned char * d;
	unsigned char *s;
	if(blockId >= sim->nBlocks ||
	   pageOffset >= PAGES_PER_BLOCK ||
	   dataLength >DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !blockList[blockId]->blockOk){
		   return YAFFS_FAIL;
	}

	d = blockList[blockId]->page[pageOffset];
	s = d + DATA_SIZE;

	if(data)
		memcpy(d,data,dataLength);

	if(spare)
		memcpy(s,spare,spareLength);

	return YAFFS_OK;
}


static int yramsim_erase(struct yaffs_dev *dev, int blockId)
{
	SimData *sim = DevToSim(dev);

	CheckInitialised();
	return yramsim_erase_internal(sim,blockId,0);
}

static int yramsim_check_block_bad(struct yaffs_dev *dev, int blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return YAFFS_FAIL;
	}

	return blockList[blockId]->blockOk ? YAFFS_OK : YAFFS_FAIL;
}

static int yramsim_mark_block_bad(struct yaffs_dev *dev, int blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return YAFFS_FAIL;
	}

	blockList[blockId]->blockOk = 0;

	return YAFFS_OK;
}


static SimData *yramsim_alloc_sim_data(u32 devId, u32 nBlocks)
{
	int ok = 1;

	Block **blockList;
	SimData *sim;
	Block *b;
	u32 i;

	if(devId >= N_RAM_SIM_DEVS)
		return NULL;

	sim = simDevs[devId];

	if(sim)
		return sim;

	sim = malloc(sizeof (SimData));
	if(!sim)
		return NULL;

	simDevs[devId] = sim;

	blockList = malloc(nBlocks * sizeof(Block *));

	sim->blockList = blockList;
	sim->nBlocks = nBlocks;
	if(!blockList){
		free(sim);
		return NULL;
	}

	for(i = 0; i < nBlocks; i++)
		blockList[i] = NULL;

	for(i = 0; i < nBlocks && ok; i++){
		b=  malloc(sizeof(Block));
		if(b){
			blockList[i] = b;
			yramsim_erase_internal(sim,i,1);
		}
		else
			ok = 0;
	}

	if(!ok){
		for(i = 0; i < nBlocks; i++)
			if(blockList[i]){
				free(blockList[i]);
				blockList[i] = NULL;
			}
		free(blockList);
		blockList = NULL;
		free(sim);
		sim = NULL;
	}

	return sim;
}


struct yaffs_dev *yramsim_CreateRamSim(const YCHAR *name,
				u32 devId, u32 nBlocks,
				u32 start_block, u32 end_block)
{
	SimData *sim;
	struct yaffs_dev *dev;
	struct yaffs_param *p;
	struct yaffs_driver *d;

	sim = yramsim_alloc_sim_data(devId, nBlocks);

	dev = malloc(sizeof(*dev));

	if(!sim || !dev){
		free(sim);
		free(dev);
		return NULL;
	}
	
	memset(dev, 0, sizeof(*dev));

	if(start_block >= sim->nBlocks)
		start_block = 0;
	if(end_block == 0 || end_block >= sim->nBlocks)
		end_block = sim->nBlocks - 1;

	p = &dev->param;
	p->name = strdup(name);
	p->start_block = start_block;
	p->end_block = end_block;
	p->total_bytes_per_chunk = DATA_SIZE;
	p->spare_bytes_per_chunk= SPARE_SIZE;
	p->chunks_per_block = PAGES_PER_BLOCK;
	p->n_reserved_blocks = 2;
	p->use_nand_ecc = 1;
	p->inband_tags = 0;
	p->is_yaffs2 = 1;
	
	d= &dev->drv;
	d->drv_initialise_fn = yramsim_initialise;
	d->drv_deinitialise_fn = yramsim_deinitialise;
	d->drv_read_chunk_fn = yramsim_rd_chunk;
	d->drv_write_chunk_fn = yramsim_wr_chunk;
	d->drv_erase_fn = yramsim_erase;
	d->drv_check_bad_fn = yramsim_check_block_bad;
	d->drv_mark_bad_fn = yramsim_mark_block_bad;
	
	dev->driver_context= (void *)sim;

	return dev;
}
