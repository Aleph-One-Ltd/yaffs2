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


#include "nanddrv.h"
#include "nand_chip.h"

int nanddrv_initialise(void)
{
	return 0;
}

static void nanddrv_send_addr(struct nand_chip *this, int page, int offset)
{
	this->set_ale(this,1);
	if(offset >= 0){
	    this->write_cycle(this, offset & 0xff);
	    this->write_cycle(this, (offset>>8) & 0x0f);
	}

	if(page >= 0){
	    this->write_cycle(this, page & 0xff);
	    this->write_cycle(this, (page>>8) & 0xff);
	    this->write_cycle(this, (page>>16) & 0xff);
	}
	this->set_ale(this,0);
}

static void nanddrv_send_cmd(struct nand_chip *this, unsigned char cmd)
{
	this->set_cle(this, 1);
	this->write_cycle(this, cmd);
	this->set_cle(this, 0);
}


static inline int nanddrv_status_pass(unsigned char status)
{
	/* If bit 0 is zero then pass, if 1 then fail */
	return (status & (1 << 0)) == 0;
}

static inline int nanddrv_status_busy(unsigned char status)
{
	/* If bit 6 is zero then busy, if 1 then ready */
	return (status & (1 << 6)) == 0;
}

static unsigned char nanddrv_get_status(struct nand_chip *this,
					int wait_not_busy)
{
	unsigned char status;

	nanddrv_send_cmd(this, 0x70);
	status = this->read_cycle(this) & 0xff;
	if(!wait_not_busy)
		return status;
	while (nanddrv_status_busy(status)) {
		status = this->read_cycle(this) & 0xff;
	}
	return status;
}

int nanddrv_read_tr(struct nand_chip *this, int page,
		struct nanddrv_transfer *tr, int n_tr)
{
	unsigned char status;
	int ncycles;

	if(n_tr < 1)
		return 0;

	nanddrv_send_cmd(this, 0x00);
	nanddrv_send_addr(this, page, tr->offset);
	nanddrv_send_cmd(this, 0x30);
	status = nanddrv_get_status(this, 1);
	if(!nanddrv_status_pass(status))
		return -1;
	nanddrv_send_cmd(this, 0x30);
	while (1) {
		if(this->bus_width_shift == 0) {
			unsigned char *buffer = tr->buffer;

			ncycles = tr->nbytes;
			while (ncycles> 0) {
				*buffer = this->read_cycle(this);
				ncycles--;
				buffer++;
			}
		} else {
			unsigned short *buffer = (unsigned short *)tr->buffer;

			ncycles = tr->nbytes >> 1;
			while (ncycles> 0) {
				*buffer = this->read_cycle(this);
				ncycles--;
				buffer++;
			}
		}
		n_tr--;
		tr++;
		if(n_tr < 1)
			break;
		nanddrv_send_cmd(this, 0x05);
		nanddrv_send_addr(this, -1, tr->offset);
		nanddrv_send_cmd(this, 0xE0);
	}
	return 0;
}


/*
 * Program page
 * Cmd: 0x80, 5-byte address, data bytes,  Cmd: 0x10, wait not busy
 */
int nanddrv_write_tr(struct nand_chip *this, int page,
		struct nanddrv_transfer *tr, int n_tr)
{
	unsigned char status;
	int ncycles;

	if (n_tr < 1)
		return 0;

	nanddrv_send_cmd(this, 0x80);
	nanddrv_send_addr(this, page, tr->offset);
	while (1) {
		if(this->bus_width_shift == 0) {
			unsigned char *buffer = tr->buffer;

			ncycles = tr->nbytes;
			while (ncycles> 0) {
				this->write_cycle(this, *buffer);
				ncycles--;
				buffer++;
			}
		} else {
			unsigned short *buffer = (unsigned short *)tr->buffer;

			ncycles = tr->nbytes >> 1;
			while (ncycles> 0) {
				this->write_cycle(this, *buffer);
				ncycles--;
				buffer++;
			}
		}
		n_tr--;
		tr++;
		if (n_tr < 1)
			break;
		nanddrv_send_cmd(this, 0x85);
		nanddrv_send_addr(this, -1, tr->offset);
	}

	if(this->power_check && this->power_check(this) < 0)
		return -1;

	nanddrv_send_cmd(this, 0x10);
	status = nanddrv_get_status(this, 1);
	if(nanddrv_status_pass(status))
		return 0;
	return -1;
}

/*
 * Block erase
 * Cmd: 0x60, 3-byte address, cmd: 0xD0. Wait not busy.
 */
int nanddrv_erase(struct nand_chip *this, int block)
{
	unsigned char status;

 	nanddrv_send_cmd(this, 0x60);
	nanddrv_send_addr(this, block * this->pages_per_block, -1);

	if(this->power_check && this->power_check(this) < 0)
		return -1;

	nanddrv_send_cmd(this, 0xD0);
	status = nanddrv_get_status(this, 1);
	if(nanddrv_status_pass(status))
		return 0;
	return -1;
}



