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

#include "yaffs_m18_drv.h"

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

#define FORMAT_OFFSET		(SPARE_AREA_OFFSET + CHUNKS_PER_BLOCK * \
				(SPARE_BYTES_PER_CHUNK + M18_SKIP))

#define FORMAT_VALUE		0x1234

#define DATA_BYTES_PER_CHUNK	1024
#define BLOCKS_IN_DEVICE        (8*1024/256)


#define YNOR_PREMARKER          (0xF6)
#define YNOR_POSTMARKER         (0xF0)


#if 1

/* Compile this for a simulation */
#include "ynorsim.h"

static struct nor_sim *nor_sim;

#define m18_drv_FlashInit() do {nor_sim = ynorsim_initialise("emfile-m18", BLOCKS_IN_DEVICE, BLOCK_SIZE_IN_BYTES); } while(0)
#define m18_drv_FlashDeinit() ynorsim_shutdown(nor_sim)
#define m18_drv_FlashWrite32(addr,buf,nwords) ynorsim_wr32(nor_sim,addr,buf,nwords)
#define m18_drv_FlashRead32(addr,buf,nwords) ynorsim_rd32(nor_sim,addr,buf,nwords)
#define m18_drv_FlashEraseBlock(addr) ynorsim_erase(nor_sim,addr)
#define DEVICE_BASE     ynorsim_get_base(nor_sim)

#else

/* Compile this to hook up to read hardware */
#include "../blob/yflashrw.h"
#define m18_drv_FlashInit()  do{} while(0)
#define m18_drv_FlashDeinit() do {} while(0)
#define m18_drv_FlashWrite32(addr,buf,nwords) Y_FlashWrite(addr,buf,nwords)
#define m18_drv_FlashRead32(addr,buf,nwords)  Y_FlashRead(addr,buf,nwords)
#define m18_drv_FlashEraseBlock(addr)         Y_FlashErase(addr,BLOCK_SIZE_IN_BYTES)
#define DEVICE_BASE     (32 * 1024 * 1024)
#endif


static u32 *Block2Addr(struct yaffs_dev *dev, int blockNumber)
{
	u8 *addr;
	dev=dev;

	addr = (u8*) DEVICE_BASE;
	addr += blockNumber * BLOCK_SIZE_IN_BYTES;

	return (u32 *) addr;
}

static u32 *Block2FormatAddr(struct yaffs_dev *dev,int blockNumber)
{
	u8 *addr;

	addr = (u8*) Block2Addr(dev,blockNumber);
	addr += FORMAT_OFFSET;

	return (u32 *)addr;
}

static u32 *Chunk2DataAddr(struct yaffs_dev *dev,int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	u8 *addr;

	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;

	addr = (u8*) Block2Addr(dev,block);
	addr += chunkInBlock * DATA_BYTES_PER_CHUNK;

	return (u32 *)addr;
}

static u32 *Chunk2SpareAddr(struct yaffs_dev *dev,int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	u8 *addr;

	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;

	addr = (u8*) Block2Addr(dev,block);
	addr += SPARE_AREA_OFFSET;
	addr += chunkInBlock * (SPARE_BYTES_PER_CHUNK + M18_SKIP);
	return (u32 *)addr;
}


static void m18_drv_AndBytes(u8*target, const u8   *src, int nbytes)
{
        while(nbytes > 0){
                *target &= *src;
                target++;
                src++;
                nbytes--;
        }
}

static int m18_drv_WriteChunkToNAND(struct yaffs_dev *dev,int nand_chunk,
				    const u8 *data, int data_len,
				    const u8 *oob, int oob_len)
{
        u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);
        u32 *spareAddr = Chunk2SpareAddr(dev,nand_chunk);

	struct yaffs_spare *spare = (struct yaffs_spare *)oob;
        struct yaffs_spare tmpSpare;

	(void) oob_len;

        /* We should only be getting called for one of 3 reasons:
         * Writing a chunk: data and spare will not be NULL
         * Writing a deletion marker: data will be NULL, spare not NULL
         * Writing a bad block marker: data will be NULL, spare not NULL
         */

        if(sizeof(struct yaffs_spare) != 16)
                BUG();

        if(data && oob)
        {
                if(spare->page_status != 0xff)
                        BUG();
                /* Write a pre-marker */
                memset(&tmpSpare,0xff,sizeof(tmpSpare));
                tmpSpare.page_status = YNOR_PREMARKER;
                m18_drv_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(struct yaffs_spare)/4);

                /* Write the data */
                m18_drv_FlashWrite32(dataAddr,(u32 *)data, data_len/ 4);


                memcpy(&tmpSpare,spare,sizeof(struct yaffs_spare));

                /* Write the real tags, but override the premarker*/
                tmpSpare.page_status = YNOR_PREMARKER;
                m18_drv_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(struct yaffs_spare)/4);

                /* Write a post-marker */
                tmpSpare.page_status = YNOR_POSTMARKER;
                m18_drv_FlashWrite32(spareAddr,(u32 *)&tmpSpare,sizeof(tmpSpare)/4);

        } else if(spare){
                /* This has to be a read-modify-write operation to handle NOR-ness */

                m18_drv_FlashRead32(spareAddr,(u32 *)&tmpSpare,16/ 4);

                m18_drv_AndBytes((u8 *)&tmpSpare,(u8 *)spare,sizeof(struct yaffs_spare));

                m18_drv_FlashWrite32(spareAddr,(u32 *)&tmpSpare,16/ 4);
        }
        else {
                BUG();
        }


	return YAFFS_OK;

}

static int m18_drv_ReadChunkFromNAND(struct yaffs_dev *dev,int nand_chunk,
					u8 *data, int data_len,
					u8 *oob, int oob_len,
					enum yaffs_ecc_result *ecc_result)
{
	struct yaffs_spare *spare = (struct yaffs_spare *)oob;

	u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);
	u32 *spareAddr = Chunk2SpareAddr(dev,nand_chunk);

	(void) data_len;

	if(data)
	{
		m18_drv_FlashRead32(dataAddr,(u32 *)data,dev->param.total_bytes_per_chunk / 4);
	}

        if(oob)
        {
                m18_drv_FlashRead32(spareAddr,(u32 *)spare, oob_len/ 4);

                /* If the page status is YNOR_POSTMARKER then it was written properly
                 * so change that to 0xFF so that the rest of yaffs is happy.
                 */
                if(spare->page_status == YNOR_POSTMARKER)
                        spare->page_status = 0xFF;
		else if(spare->page_status != 0xff &&
			(spare->page_status | YNOR_PREMARKER) != 0xff)
			spare->page_status = YNOR_PREMARKER;
        }

	if(ecc_result)
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;

}


static int m18_drv_FormatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *blockAddr = Block2Addr(dev,blockNumber);
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = FORMAT_VALUE;

	m18_drv_FlashEraseBlock(blockAddr);
	m18_drv_FlashWrite32(formatAddr,&formatValue,1);

	return YAFFS_OK;
}

static int m18_drv_UnformatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = 0;

	m18_drv_FlashWrite32(formatAddr,&formatValue,1);

	return YAFFS_OK;
}

static int m18_drv_IsBlockFormatted(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue;


	m18_drv_FlashRead32(formatAddr,&formatValue,1);

	return (formatValue == FORMAT_VALUE);
}

static int m18_drv_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber)
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
		m18_drv_UnformatBlock(dev,blockNumber);
		m18_drv_FormatBlock(dev,blockNumber);
		return YAFFS_OK;
	}

}

static int m18_drv_InitialiseNAND(struct yaffs_dev *dev)
{
	u32 i;

	m18_drv_FlashInit();
	/* Go through the blocks formatting them if they are not formatted */
	for(i = dev->param.start_block; i <= dev->param.end_block; i++){
		if(!m18_drv_IsBlockFormatted(dev,i)){
			m18_drv_FormatBlock(dev,i);
		}
	}
	return YAFFS_OK;
}

static int m18_drv_Deinitialise_flash_fn(struct yaffs_dev *dev)
{
	dev=dev;

	m18_drv_FlashDeinit();

	return YAFFS_OK;
}


struct yaffs_dev *yaffs_m18_install_drv(const char *name)
{

	struct yaffs_dev *dev = malloc(sizeof(struct yaffs_dev));
	char *name_copy = strdup(name);
	struct yaffs_param *param;
	struct yaffs_driver *drv;


	if(!dev || !name_copy) {
		free(name_copy);
		free(dev);
		return NULL;
	}

	param = &dev->param;
	drv = &dev->drv;

	memset(dev, 0, sizeof(*dev));

	param->name = name_copy;

	param->total_bytes_per_chunk = 1024;
	param->chunks_per_block =248;
	param->n_reserved_blocks = 2;
	param->start_block = 0; // Can use block 0
	param->end_block = 31; // Last block
	param->use_nand_ecc = 0; // use YAFFS's ECC

	drv->drv_write_chunk_fn = m18_drv_WriteChunkToNAND;
	drv->drv_read_chunk_fn = m18_drv_ReadChunkFromNAND;
	drv->drv_erase_fn = m18_drv_EraseBlockInNAND;
	drv->drv_initialise_fn = m18_drv_InitialiseNAND;
	drv->drv_deinitialise_fn = m18_drv_Deinitialise_flash_fn;

	param->n_caches = 10;
	param->disable_soft_del = 1;

	dev->driver_context = (void *) 1;	// Used to identify the device in fstat.

	yaffs_add_device(dev);

	return NULL;
}
