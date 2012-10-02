/*
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 *	WARNING--WARNING--WARNING
 *	This is not the original fsx.c. It has been modified to run with
 *      yaffs direct. Seek out the original fsx.c if you want to do anything
 *	else.
 *
 *	
 *
 *	File:	fsx.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	File system exerciser. 
 *
 *	Rewrite and enhancements 1998-2001 Conrad Minshall -- conrad@mac.com
 *
 *	Various features from Joe Sokol, Pat Dirks, and Clark Warner.
 *
 *	Small changes to work under Linux -- davej@suse.de
 *
 *	Sundry porting patches from Guy Harris 12/2001
 *
 *	Checks for mmap last-page zero fill.
 *
 *	Modified heavily by Charles Manning to exercise via the
 *	yaffs direct interface.
 *
 */

#include "yaffs_fsx.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _UWIN
# include <sys/param.h>
# include <limits.h>
# include <time.h>
# include <strings.h>
#endif
#include <fcntl.h>
#include <sys/mman.h>
#ifndef MAP_FILE
# define MAP_FILE 0
#endif
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#include "yaffsfs.h"

#define NUMPRINTCOLUMNS 32	/* # columns of data to print on each line */

/*
 *	A log entry is an operation and a bunch of arguments.
 */

struct log_entry {
	int	operation;
	int	args[3];
};

#define	LOGSIZE	1000

struct log_entry	oplog[LOGSIZE];	/* the log */
int			logptr = 0;	/* current position in log */
int			logcount = 0;	/* total ops */

/*
 *	Define operations
 */

#define	OP_READ		1
#define OP_WRITE	2
#define OP_TRUNCATE	3
#define OP_CLOSEOPEN	4
#define OP_MAPREAD	5
#define OP_MAPWRITE	6
#define OP_SKIPPED	7

int page_size;
int page_mask;

char	*original_buf;			/* a pointer to the original data */
char	*good_buf;			/* a pointer to the correct data */
char	*temp_buf;			/* a pointer to the current data */

char	fname[200];				/* name of our test file */
char	mount_name[200];

int	fd;				/* fd for our test file */

off_t		file_size = 0;
off_t		biggest = 0;
char		state[256];
unsigned long	testcalls = 0;		/* calls to function "test" */

unsigned long	simulatedopcount = 0;	/* -b flag */
int	closeprob = 0;			/* -c flag */
int	debug = 0;			/* -d flag */
unsigned long	debugstart = 0;		/* -D flag */
unsigned long	maxfilelen = 256 * 1024;	/* -l flag */
int	sizechecks = 1;			/* -n flag disables them */
int	maxoplen = 64 * 1024;		/* -o flag */
int	quiet = 0;			/* -q flag */
unsigned long progressinterval = 0;	/* -p flag */
int	readbdy = 1;			/* -r flag */
int	style = 0;			/* -s flag */
int	truncbdy = 1;			/* -t flag */
int	writebdy = 1;			/* -w flag */
long	monitorstart = -1;		/* -m flag */
long	monitorend = -1;		/* -m flag */
int	lite = 0;			/* -L flag */
int	randomoplen = 1;		/* -O flag disables it */
int	seed = 1;			/* -S flag */

int     mapped_writes = 0;	      /* yaffs direct does not support mmapped files */
int 	mapped_reads = 0;

int	fsxgoodfd = 0;
FILE *	fsxlogf = NULL;
int badoff = -1;
int closeopen = 0;



void EXIT(int x)
{
	printf("fsx wanted to exit with %d\n",x);
	while(x){}
}

char goodfile[1024];
char logfile[1024];


void
vwarnc(code, fmt, ap)
	int code;
	const char *fmt;
	va_list ap;
{
	fprintf(stderr, "fsx: ");
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(code));
}


void
warn(const char * fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(errno, fmt, ap);
	va_end(ap);
}


void
prt(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	if (fsxlogf)
		vfprintf(fsxlogf, fmt, args);
	va_end(args);
}

void
prterr(char *prefix)
{
	prt("%s%s%s\n", prefix, prefix ? ": " : "", strerror(errno));
}


void
log4(int operation, int arg0, int arg1, int arg2)
{
	struct log_entry *le;

	le = &oplog[logptr];
	le->operation = operation;
	if (closeopen)
		le->operation = ~ le->operation;
	le->args[0] = arg0;
	le->args[1] = arg1;
	le->args[2] = arg2;
	logptr++;
	logcount++;
	if (logptr >= LOGSIZE)
		logptr = 0;
}


void
logdump(void)
{
	int	i, count, down;
	struct log_entry	*lp;

	prt("LOG DUMP (%d total operations):\n", logcount);
	if (logcount < LOGSIZE) {
		i = 0;
		count = logcount;
	} else {
		i = logptr;
		count = LOGSIZE;
	}
	for ( ; count > 0; count--) {
		int opnum;

		opnum = i+1 + (logcount/LOGSIZE)*LOGSIZE;
		prt("%d(%d mod 256): ", opnum, opnum%256);
		lp = &oplog[i];
		if ((closeopen = lp->operation < 0))
			lp->operation = ~ lp->operation;
			
		switch (lp->operation) {
		case OP_MAPREAD:
			prt("MAPREAD\t0x%x thru 0x%x\t(0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] && badoff <
						     lp->args[0] + lp->args[1])
				prt("\t***RRRR***");
			break;
		case OP_MAPWRITE:
			prt("MAPWRITE 0x%x thru 0x%x\t(0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] && badoff <
						     lp->args[0] + lp->args[1])
				prt("\t******WWWW");
			break;
		case OP_READ:
			prt("READ\t0x%x thru 0x%x\t(0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (badoff >= lp->args[0] &&
			    badoff < lp->args[0] + lp->args[1])
				prt("\t***RRRR***");
			break;
		case OP_WRITE:
			prt("WRITE\t0x%x thru 0x%x\t(0x%x bytes)",
			    lp->args[0], lp->args[0] + lp->args[1] - 1,
			    lp->args[1]);
			if (lp->args[0] > lp->args[2])
				prt(" HOLE");
			else if (lp->args[0] + lp->args[1] > lp->args[2])
				prt(" EXTEND");
			if ((badoff >= lp->args[0] || badoff >=lp->args[2]) &&
			    badoff < lp->args[0] + lp->args[1])
				prt("\t***WWWW");
			break;
		case OP_TRUNCATE:
			down = lp->args[0] < lp->args[1];
			prt("TRUNCATE %s\tfrom 0x%x to 0x%x",
			    down ? "DOWN" : "UP", lp->args[1], lp->args[0]);
			if (badoff >= lp->args[!down] &&
			    badoff < lp->args[!!down])
				prt("\t******WWWW");
			break;
		case OP_SKIPPED:
			prt("SKIPPED (no operation)");
			break;
		default:
			prt("BOGUS LOG ENTRY (operation code = %d)!",
			    lp->operation);
		}
		if (closeopen)
			prt("\n\t\tCLOSE/OPEN");
		prt("\n");
		i++;
		if (i == LOGSIZE)
			i = 0;
	}
}


void
save_buffer(char *buffer, off_t bufferlength, int fd)
{
	off_t ret;
	ssize_t byteswritten;

	if (fd <= 0 || bufferlength == 0)
		return;

	if (bufferlength > SSIZE_MAX) {
		prt("fsx flaw: overflow in save_buffer\n");
		EXIT(67);
	}
	if (lite) {
		off_t size_by_seek = yaffs_lseek(fd, (off_t)0, SEEK_END);
		if (size_by_seek == (off_t)-1)
			prterr("save_buffer: lseek eof");
		else if (bufferlength > size_by_seek) {
			warn("save_buffer: .fsxgood file too short... will save 0x%llx bytes instead of 0x%llx\n", (unsigned long long)size_by_seek,
			     (unsigned long long)bufferlength);
			bufferlength = size_by_seek;
		}
	}

	ret = yaffs_lseek(fd, (off_t)0, SEEK_SET);
	if (ret == (off_t)-1)
		prterr("save_buffer: lseek 0");
	
	byteswritten = yaffs_write(fd, buffer, (size_t)bufferlength);
	if (byteswritten != bufferlength) {
		if (byteswritten == -1)
			prterr("save_buffer write");
		else
			warn("save_buffer: short write, 0x%x bytes instead of 0x%llx\n",
			     (unsigned)byteswritten,
			     (unsigned long long)bufferlength);
	}
}


void
report_failure(int status)
{
	logdump();
	
	if (fsxgoodfd) {
		if (good_buf) {
			save_buffer(good_buf, file_size, fsxgoodfd);
			prt("Correct content saved for comparison\n");
			prt("(maybe hexdump \"%s\" vs \"%s.fsxgood\")\n",
			    fname, fname);
		}
		close(fsxgoodfd);
	}
	prt("Exiting with %d\n",status);
	EXIT(status);
}


#define short_at(cp) ((unsigned short)((*((unsigned char *)(cp)) << 8) | \
					*(((unsigned char *)(cp)) + 1)))

void
check_buffers(unsigned offset, unsigned size)
{
	unsigned char c, t;
	unsigned i = 0;
	unsigned n = 0;
	unsigned op = 0;
	unsigned bad = 0;

	if (memcmp(good_buf + offset, temp_buf, size) != 0) {
		prt("READ BAD DATA: offset = 0x%x, size = 0x%x\n",
		    offset, size);
		prt("OFFSET\tGOOD\tBAD\tRANGE\n");
		while (size > 0) {
			c = good_buf[offset];
			t = temp_buf[i];
			if (c != t) {
				if (n == 0) {
					bad = short_at(&temp_buf[i]);
					prt("0x%5x\t0x%04x\t0x%04x", offset,
					    short_at(&good_buf[offset]), bad);
					op = temp_buf[offset & 1 ? i+1 : i];
				}
				n++;
				badoff = offset;
			}
			offset++;
			i++;
			size--;
		}
		if (n) {
			prt("\t0x%5x\n", n);
			if (bad)
				prt("operation# (mod 256) for the bad data may be %u\n", ((unsigned)op & 0xff));
			else
				prt("operation# (mod 256) for the bad data unknown, check HOLE and EXTEND ops\n");
		} else
			prt("????????????????\n");
		report_failure(110);
	}
}


void
check_size(void)
{
	struct yaffs_stat	statbuf;
	off_t	size_by_seek;

	if (yaffs_fstat(fd, &statbuf)) {
		prterr("check_size: fstat");
		statbuf.st_size = -1;
	}
	size_by_seek = yaffs_lseek(fd, (off_t)0, SEEK_END);
	if (file_size != statbuf.st_size || file_size != size_by_seek) {
		prt("Size error: expected 0x%llx stat 0x%llx seek 0x%llx\n",
		    (unsigned long long)file_size,
		    (unsigned long long)statbuf.st_size,
		    (unsigned long long)size_by_seek);
		report_failure(120);
	}
}


void
check_trunc_hack(void)
{
	struct yaffs_stat statbuf;

	yaffs_ftruncate(fd, (off_t)0);
	yaffs_ftruncate(fd, (off_t)100000);
	yaffs_fstat(fd, &statbuf);
	if (statbuf.st_size != (off_t)100000) {
		prt("no extend on truncate! not posix!\n");
		EXIT(130);
	}
	yaffs_ftruncate(fd, (off_t)0);
}


void
doread(unsigned offset, unsigned size)
{
	off_t ret;
	unsigned iret;

	offset -= offset % readbdy;
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size read\n");
		log4(OP_SKIPPED, OP_READ, offset, size);
		return;
	}
	if (size + offset > file_size) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping seek/read past end of file\n");
		log4(OP_SKIPPED, OP_READ, offset, size);
		return;
	}

	log4(OP_READ, offset, size, 0);

	if (testcalls <= simulatedopcount)
		return;

	if (!quiet && ((progressinterval &&
			testcalls % progressinterval == 0) ||
		       (debug &&
			(monitorstart == -1 ||
			 (offset + size > monitorstart &&
			  (monitorend == -1 || offset <= monitorend))))))
		prt("%lu read\t0x%x thru\t0x%x\t(0x%x bytes)\n", testcalls,
		    offset, offset + size - 1, size);
	ret = yaffs_lseek(fd, (off_t)offset, SEEK_SET);
	if (ret == (off_t)-1) {
		prterr("doread: lseek");
		report_failure(140);
	}
	iret = yaffs_read(fd, temp_buf, size);
	if (iret != size) {
		if (iret == -1)
			prterr("doread: read");
		else
			prt("short read: 0x%x bytes instead of 0x%x\n",
			    iret, size);
		report_failure(141);
	}
	check_buffers(offset, size);
}





void
gendata(char *original_buf, char *good_buf, unsigned offset, unsigned size)
{
	while (size--) {
		good_buf[offset] = testcalls % 256; 
		if (offset % 2)
			good_buf[offset] += original_buf[offset];
		offset++;
	}
}


void
dowrite(unsigned offset, unsigned size)
{
	off_t ret;
	unsigned iret;

	offset -= offset % writebdy;
	if (size == 0) {
		if (!quiet && testcalls > simulatedopcount)
			prt("skipping zero size write\n");
		log4(OP_SKIPPED, OP_WRITE, offset, size);
		return;
	}

	log4(OP_WRITE, offset, size, file_size);

	gendata(original_buf, good_buf, offset, size);
	if (file_size < offset + size) {
		if (file_size < offset)
			memset(good_buf + file_size, '\0', offset - file_size);
		file_size = offset + size;
		if (lite) {
			warn("Lite file size bug in fsx!");
			report_failure(149);
		}
	}

	if (testcalls <= simulatedopcount)
		return;

	if (!quiet && ((progressinterval &&
			testcalls % progressinterval == 0) ||
		       (debug &&
			(monitorstart == -1 ||
			 (offset + size > monitorstart &&
			  (monitorend == -1 || offset <= monitorend))))))
		prt("%lu write\t0x%x thru\t0x%x\t(0x%x bytes)\n", testcalls,
		    offset, offset + size - 1, size);
	ret = yaffs_lseek(fd, (off_t)offset, SEEK_SET);
	if (ret == (off_t)-1) {
		prterr("dowrite: lseek");
		report_failure(150);
	}
	iret = yaffs_write(fd, good_buf + offset, size);
	if (iret != size) {
		if (iret == -1)
			prterr("dowrite: write");
		else
			prt("short write: 0x%x bytes instead of 0x%x\n",
			    iret, size);
		report_failure(151);
	}
}



void
dotruncate(unsigned size)
{
	int oldsize = file_size;

	size -= size % truncbdy;
	if (size > biggest) {
		biggest = size;
		if (!quiet && testcalls > simulatedopcount)
			prt("truncating to largest ever: 0x%x\n", size);
	}

	log4(OP_TRUNCATE, size, (unsigned)file_size, 0);

	if (size > file_size)
		memset(good_buf + file_size, '\0', size - file_size);
	file_size = size;

	if (testcalls <= simulatedopcount)
		return;
	
	if ((progressinterval && testcalls % progressinterval == 0) ||
	    (debug && (monitorstart == -1 || monitorend == -1 ||
		       size <= monitorend)))
		prt("%lu trunc\tfrom 0x%x to 0x%x\n", testcalls, oldsize, size);
	if (yaffs_ftruncate(fd, (off_t)size) == -1) {
		prt("ftruncate1: %x\n", size);
		prterr("dotruncate: ftruncate");
		report_failure(160);
	}
}


void
writefileimage()
{
	ssize_t iret;

	if (yaffs_lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1) {
		prterr("writefileimage: lseek");
		report_failure(171);
	}
	iret = yaffs_write(fd, good_buf, file_size);
	if ((off_t)iret != file_size) {
		if (iret == -1)
			prterr("writefileimage: write");
		else
			prt("short write: 0x%x bytes instead of 0x%llx\n",
			    iret, (unsigned long long)file_size);
		report_failure(172);
	}
	if (lite ? 0 : yaffs_ftruncate(fd, file_size) == -1) {
		prt("ftruncate2: %llx\n", (unsigned long long)file_size);
		prterr("writefileimage: ftruncate");
		report_failure(173);
	}
}


void
docloseopen(void)
{ 
	if (testcalls <= simulatedopcount)
		return;

	if (debug)
		prt("%lu close/open\n", testcalls);
	if (yaffs_close(fd)) {
		prterr("docloseopen: close");
		report_failure(180);
	}
	fd = yaffs_open(fname, O_RDWR, 0);
	if (fd < 0) {
		prterr("docloseopen: open");
		report_failure(181);
	}
}


void
yaffs_fsx_do_op(void)
{
	unsigned long	offset;
	unsigned long	size = maxoplen;
	unsigned long	rv = random();
	unsigned long	op = rv % (3 + !lite + mapped_writes);

	/* turn off the map read if necessary */

	if (op == 2 && !mapped_reads)
	    op = 0;

	if (simulatedopcount > 0 && testcalls == simulatedopcount)
		writefileimage();

	testcalls++;

	if (closeprob)
		closeopen = (rv >> 3) < (1 << 28) / closeprob;

	if (debugstart > 0 && testcalls >= debugstart)
		debug = 1;

	if (!quiet && testcalls < simulatedopcount && testcalls % 100000 == 0)
		prt("%lu...\n", testcalls);

	/*
	 * READ:	op = 0
	 * WRITE:	op = 1
	 * MAPREAD:     op = 2
	 * TRUNCATE:	op = 3
	 * MAPWRITE:    op = 3 or 4
	 */
	if (lite ? 0 : op == 3 && (style & 1) == 0) /* vanilla truncate? */
		dotruncate(random() % maxfilelen);
	else {
		if (randomoplen)
			size = random() % (maxoplen+1);
		if (lite ? 0 : op == 3)
			dotruncate(size);
		else {
			offset = random();
			if (op == 1 || op == (lite ? 3 : 4)) {
				offset %= maxfilelen;
				if (offset + size > maxfilelen)
					size = maxfilelen - offset;
				dowrite(offset, size);
			} else {
				if (file_size)
					offset %= file_size;
				else
					offset = 0;
				if (offset + size > file_size)
					size = file_size - offset;
				doread(offset, size);
			}
		}
	}
	if (sizechecks && testcalls > simulatedopcount)
		check_size();
	if (closeopen)
		docloseopen();
}


void
cleanup(sig)
	int	sig;
{
	if (sig)
		prt("signal %d\n", sig);
	prt("testcalls = %lu\n", testcalls);
	EXIT(sig);
}


void
usage(void)
{
	fprintf(stdout, "usage: %s",
		"fsx [-dnqLOW] [-b opnum] [-c Prob] [-l flen] [-m start:end] [-o oplen] [-p progressinterval] [-r readbdy] [-s style] [-t truncbdy] [-w writebdy] [-D startingop] [-N numops] [-P dirpath] [-S seed] fname\n\
	-b opnum: beginning operation number (default 1)\n\
	-c P: 1 in P chance of file close+open at each op (default infinity)\n\
	-d: debug output for all operations\n\
	-l flen: the upper bound on file size (default 262144)\n\
	-m startop:endop: monitor (print debug output) specified byte range (default 0:infinity)\n\
	-n: no verifications of file size\n\
	-o oplen: the upper bound on operation size (default 65536)\n\
	-p progressinterval: debug output at specified operation interval\n\
	-q: quieter operation\n\
	-r readbdy: 4096 would make reads page aligned (default 1)\n\
	-s style: 1 gives smaller truncates (default 0)\n\
	-t truncbdy: 4096 would make truncates page aligned (default 1)\n\
	-w writebdy: 4096 would make writes page aligned (default 1)\n\
	-D startingop: debug output starting at specified operation\n\
	-L: fsxLite - no file creations & no file size changes\n\
	-N numops: total # operations to do (default infinity)\n\
	-O: use oplen (see -o flag) for every op (default random)\n\
	-P dirpath: save .fsxlog and .fsxgood files in dirpath (default ./)\n\
	-S seed: for random # generator (default 1) 0 gets timestamp\n\
	fname: this filename is REQUIRED (no default)\n");
	EXIT(90);
}


int
getnum(char *s, char **e)
{
	int ret = -1;

	*e = (char *) 0;
	ret = strtol(s, e, 0);
	if (*e)
		switch (**e) {
		case 'b':
		case 'B':
			ret *= 512;
			*e = *e + 1;
			break;
		case 'k':
		case 'K':
			ret *= 1024;
			*e = *e + 1;
			break;
		case 'm':
		case 'M':
			ret *= 1024*1024;
			*e = *e + 1;
			break;
		case 'w':
		case 'W':
			ret *= 4;
			*e = *e + 1;
			break;
		}
	return (ret);
}



extern int random_seed;
extern int simulate_power_failure;


int mounted_by_fsx = 0;

int
yaffs_fsx_init(const char *mount_pt)
{
	int	i;

	goodfile[0] = 0;
	logfile[0] = 0;

	page_size = getpagesize();
	page_mask = page_size - 1;

	setvbuf(stdout, (char *)0, _IOLBF, 0); /* line buffered stdout */

	strcpy(mount_name,mount_pt);
	strcpy(fname,mount_name);
	strcat(fname,"/fsxdata");
	
#if 0
	signal(SIGHUP,	cleanup);
	signal(SIGINT,	cleanup);
	signal(SIGPIPE,	cleanup);
	signal(SIGALRM,	cleanup);
	signal(SIGTERM,	cleanup);
	signal(SIGXCPU,	cleanup);
	signal(SIGXFSZ,	cleanup);
	signal(SIGVTALRM,	cleanup);
	signal(SIGUSR1,	cleanup);
	signal(SIGUSR2,	cleanup);
#endif

	initstate(seed, state, 256);
	setstate(state);
	fd = yaffs_open(fname, O_RDWR|(lite ? 0 : O_CREAT|O_TRUNC), 0666);
	if (fd < 0) {
		prterr(fname);
		EXIT(91);
	}
	strncat(goodfile, fname, 256);
	strcat (goodfile, ".fsxgood");
	fsxgoodfd = yaffs_open(goodfile, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fsxgoodfd < 0) {
		prterr(goodfile);
		EXIT(92);
	}
	strncat(logfile, "fsx", 256);
	strcat (logfile, ".fsxlog");
	fsxlogf = fopen(logfile, "w");
	if (fsxlogf == NULL) {
		prterr(logfile);
		EXIT(93);
	}
	if (lite) {
		off_t ret;
		file_size = maxfilelen = yaffs_lseek(fd, (off_t)0, SEEK_END);
		if (file_size == (off_t)-1) {
			prterr(fname);
			warn("main: lseek eof");
			EXIT(94);
		}
		ret = yaffs_lseek(fd, (off_t)0, SEEK_SET);
		if (ret == (off_t)-1) {
			prterr(fname);
			warn("main: lseek 0");
			EXIT(95);
		}
	}
	original_buf = (char *) malloc(maxfilelen);
	for (i = 0; i < maxfilelen; i++)
		original_buf[i] = random() % 256;
	good_buf = (char *) malloc(maxfilelen);
	memset(good_buf, '\0', maxfilelen);
	temp_buf = (char *) malloc(maxoplen);
	memset(temp_buf, '\0', maxoplen);
	if (lite) {	/* zero entire existing file */
		ssize_t written;

		written = yaffs_write(fd, good_buf, (size_t)maxfilelen);
		if (written != maxfilelen) {
			if (written == -1) {
				prterr(fname);
				warn("main: error on write");
			} else
				warn("main: short write, 0x%x bytes instead of 0x%x\n",
				     (unsigned)written, maxfilelen);
			EXIT(98);
		}
	} else 
		check_trunc_hack();
		
	printf("yaffs_fsx_init done\n");
		
	return 0;
}


int yaffs_fsx_complete(void)
{
	if (yaffs_close(fd)) {
		prterr("close");
		report_failure(99);
	}
	
	yaffs_close(fsxgoodfd);
	
	prt("All operations completed A-OK!\n");

	EXIT(0);
	return 0;
}

int
yaffs_fsx_main(const char *mount_pt, int numops)
{
	yaffs_fsx_init(mount_pt);
	while (numops == -1 || numops--)
		yaffs_fsx_do_op();
	yaffs_fsx_complete();
	
	return 0;
}
