/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * This is an interface module for handling NOR in yaffs1 mode.
 */

/* First set up for M18 with 1k chunks and 16-byte spares.
 *
 * NB We're using the oddball M18 modes of operation here 
 * The chip is 64MB based at 0x0000, but YAFFS only going to use the top half
 * ie. YAFFS will be from 32MB to 64MB.
 *
 * The M18 has two ways of writing data. Every Programming Region (1kbytes) 
 * can be programmed in two modes:
 * * Object Mode 1024 bytes of write once data.
 * * Control Mode: 512bytes of bit-writeable data. 
 *    This is arranged as 32 * (16 bytes of bit-writable followed by 16 bytes of "dont touch")
 * 
 * The block size is 256kB, making 128 blocks in the 32MB YAFFS area.
 * Each block comprises:
 *   Offset   0k: 248 x 1k data pages
 *   Offset 248k: 248 x 32-byte spare areas implemented as 16 bytes of spare followed by 16 bytes untouched)
 *   Offset 248k + (248 * 32): Format marker
 *   
 */

const char *yaffs_norif1_c_version = "$Id: yaffs_norif1.c,v 1.6 2010-02-18 01:18:04 charles Exp $";

#include "yaffs_norif1.h"

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"

#define SPARE_BYTES_PER_CHUNK	16
#define M18_SKIP		16
#define PROG_REGION_SIZE	1024
#define BLOCK_SIZE_IN_BYTES 	(256*1024)
#define CHUNKS_PER_BLOCK	248
#define SPARE_AREA_OFFSET	(CHUNKS_PER_BLOCK * PROG_REGION_SIZE)

#define FORMAT_OFFSET		(SPARE_AREA_OFFSET + CHUNKS_PER_BLOCK * (SPARE_BYTES_PER_CHUNK + M18_SKIP))

#define FORMAT_VALUE		0x1234

#define DATA_BYTES_PER_CHUNK	1024
#define BLOCKS_IN_DEVICE        (8*1024/256)


#define YNOR_PREMARKER          (0xF6)
#define YNOR_POSTMARKER         (0xF0)


#if 1

/* Compile this for a simulation */
#include "ynorsim.h"
#define ynorif1_FlashInit() ynorsim_initialise()
#define ynorif1_FlashDeinit() ynorsim_shutdown()
#define ynorif1_FlashWrite32(addr,buf,nwords) ynorsim_wr32(addr,buf,nwords) 
#define ynorif1_FlashRead32(addr,buf,nwords) ynorsim_rd32(addr,buf,nwords) 
#define ynorif1_FlashEraseBlock(addr) ynorsim_erase(addr)
#define DEVICE_BASE     ynorsim_get_base()
#else

/* Compile this for running on blob, hacked for yaffs access */
#include "../blob/yflashrw.h"
#define ynorif1_FlashInit()  do{} while(0)
#define ynorif1_FlashDeinit() do {} while(0)
#define ynorif1_FlashWrite32(addr,buf,nwords) Y_FlashWrite(addr,buf,nwords) 
#define ynorif1_FlashRead32(addr,buf,nwords)  Y_FlashRead(addr,buf,nwords) 
#define ynorif1_FlashEraseBlock(addr)         Y_FlashErase(addr,BLOCK_SIZE_IN_BYTES)
#define DEVICE_BASE     (32 * 1024 * 1024)
#endif

u32 *Block2Addr(struct yaffs_dev *dev, int blockNumber)
{
	u32 addr;
	dev=dev;
	
	addr = (u32) DEVICE_BASE;
	addr += blockNumber * BLOCK_SIZE_IN_BYTES;
	
	return (u32 *) addr;
}

u32 *Block2FormatAddr(struct yaffs_dev *dev,int blockNumber)
{
	u32 addr;

	addr = (u32) Block2Addr(dev,blockNumber);
	addr += FORMAT_OFFSET;
	
	return (u32 *)addr;
}
u32 *Chunk2DataAddr(struct yaffs_dev *dev,int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	u32  addr;
	
	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;
	
	addr = (u32) Block2Addr(dev,block);
	addr += chunkInBlock * DATA_BYTES_PER_CHUNK;
	
	return (u32 *)addr;
}

u32 *Chunk2SpareAddr(struct yaffs_dev *dev,int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	u32 addr;
	
	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;
	
	addr = (u32) Block2Addr(dev,block);
	addr += SPARE_AREA_OFFSET;
	addr += chunkInBlock * (SPARE_BYTES_PER_CHUNK + M18_SKIP);
	return (u32 *)addr;
}


void ynorif1_AndBytes(u8*target, const u8   *src, int nbytes)
{
        while(nbytes > 0){
                *target &= *src;
                target++;
                src++;
                nbytes--;
        }
}

int ynorif1_WriteChunkToNAND(struct yaffs_dev *dev,int nand_chunk,const u8 *data, const struct yaffs_spare *spare)
{
        u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);
        u32 *spareAddr = Chunk2SpareAddr(dev,nand_chunk);
        
        struct yaffs_spare tmpSpare;
        
        /* We should only be getting called for one of 3 reasons:
         * Writing a chunk: data and spare will not be NULL
         * Writing a deletion marker: data will be NULL, spare not NULL
         * Writing a bad block marker: data will be NULL, spare not NULL
         */
         
        if(sizeof(struct yaffs_spare) != 16)
                BUG();
        
        if(data && spare)
        {
                if(spare->page_status != 0xff)
                        BUG();
                /* Write a pre-marker */
                memset(&tmpSpare,0xff,sizeof(tmpSpare));
                tmpSpare.page_status = YNOR_PREMARKER;
                ynorif1_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(struct yaffs_spare)/4);

                /* Write the data */            
                ynorif1_FlashWrite32(dataAddr,(u32 *)data,dev->param.total_bytes_per_chunk / 4);
                
                
                memcpy(&tmpSpare,spare,sizeof(struct yaffs_spare));
                
                /* Write the real tags, but override the premarker*/
                tmpSpare.page_status = YNOR_PREMARKER;
                ynorif1_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(struct yaffs_spare)/4);
                
                /* Write a post-marker */
                tmpSpare.page_status = YNOR_POSTMARKER;
                ynorif1_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(tmpSpare)/4);  

        } else if(spare){
                /* This has to be a read-modify-write operation to handle NOR-ness */

                ynorif1_FlashRead32(spareAddr,(u32 *)&tmpSpare,16/ 4);
                
                ynorif1_AndBytes((u8 *)&tmpSpare,(u8 *)spare,sizeof(struct yaffs_spare));
                
                ynorif1_FlashWrite32(spareAddr,(u32 *)&tmpSpare,16/ 4);
        }
        else {
                BUG();
        }
        

	return YAFFS_OK;	

}

int ynorif1_ReadChunkFromNAND(struct yaffs_dev *dev,int nand_chunk, u8 *data, struct yaffs_spare *spare)
{

	u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);
	u32 *spareAddr = Chunk2SpareAddr(dev,nand_chunk);
	
	if(data)
	{
		ynorif1_FlashRead32(dataAddr,(u32 *)data,dev->param.total_bytes_per_chunk / 4);
	}
	
        if(spare)
        {
                ynorif1_FlashRead32(spareAddr,(u32 *)spare,16/ 4);
                
                /* If the page status is YNOR_POSTMARKER then it was written properly
                 * so change that to 0xFF so that the rest of yaffs is happy.
                 */
                if(spare->page_status == YNOR_POSTMARKER)
                        spare->page_status = 0xFF;
		else if(spare->page_status != 0xff &&
			(spare->page_status | YNOR_PREMARKER) != 0xff)
			spare->page_status = YNOR_PREMARKER;
        }
        

	return YAFFS_OK;	

}

static int ynorif1_FormatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *blockAddr = Block2Addr(dev,blockNumber);
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = FORMAT_VALUE;
	
	ynorif1_FlashEraseBlock(blockAddr);
	ynorif1_FlashWrite32(formatAddr,&formatValue,1);
	
	return YAFFS_OK;
}

static int ynorif1_UnformatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = 0;
	
	ynorif1_FlashWrite32(formatAddr,&formatValue,1);
	
	return YAFFS_OK;
}

static int ynorif1_IsBlockFormatted(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue;
	
	
	ynorif1_FlashRead32(formatAddr,&formatValue,1);
	
	return (formatValue == FORMAT_VALUE);
}

int ynorif1_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber)
{

	if(blockNumber < 0 || blockNumber >= BLOCKS_IN_DEVICE)
	{
		yaffs_trace(YAFFS_TRACE_ALWAYS,
			"Attempt to erase non-existant block %d\n",
			blockNumber);
		return YAFFS_FAIL;
	}
	else
	{
		ynorif1_UnformatBlock(dev,blockNumber);
		ynorif1_FormatBlock(dev,blockNumber);
		return YAFFS_OK;
	}
	
}

int ynorif1_InitialiseNAND(struct yaffs_dev *dev)
{
	int i;
	
	ynorif1_FlashInit();
	/* Go through the blocks formatting them if they are not formatted */
	for(i = dev->param.start_block; i <= dev->param.end_block; i++){
		if(!ynorif1_IsBlockFormatted(dev,i)){
			ynorif1_FormatBlock(dev,i);
		}
	}
	return YAFFS_OK;
}

int ynorif1_Deinitialise_flash_fn(struct yaffs_dev *dev)
{
	dev=dev;	
	ynorif1_FlashDeinit();

	return YAFFS_OK;
}


