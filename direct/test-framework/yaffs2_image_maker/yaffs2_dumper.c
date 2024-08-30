/*
 * A dumper that dumps out the contents of an image file.
 * This assumes that the device has the following properties:
 * * 2k bytes per page
 * * 64  pages per block
 * * Yaffs is using inband tags.
 *
 * Note that this utility first generates a "working file" which is
 * a simulation of the NAND from the input image file.
 * 
 * It then dumps out the files in the image.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#if 1
#undef CONFIG_YAFFS_PROVIDE_DEFS
#undef CONFIG_YAFFSFS_PROVIDE_VALUES
#endif

#include "yaffsfs.h"

#include "yaffs_flexible_file_sim.h"
#include "yaffs_guts.h"
#include "yaffs_trace.h"
#include "yaffs_packedtags2.h"


#define DEFAULT_CHUNKS_PER_BLOCK	64
#define DEFAULT_CHUNK_SIZE		2048

/* Some stub definitions to get building to work. */
int simulate_power_failure = 0;
int random_seed = 0;

static char *input_file;
static char *output_dir;
static char endian = 'l';
static int no_tags_ecc = 0;
static int inband_tags = 0;

static int tags_size;
static int record_size;
static int chunk_size = DEFAULT_CHUNK_SIZE;
static int chunks_per_block = DEFAULT_CHUNKS_PER_BLOCK;
static int block_size;
static int n_blocks;
static int page_spare_size;

static void usage(const char *prog_name)
{
	printf("Usage: %s options\n", prog_name);
	printf("\t-i name input_file name\n");
	printf("\t-o name output_directory (optional)\n");
	printf("\t-b      big endian output\n");
	printf("\t-I      store tags in flash data area (inband_tags)\n");
	printf("\t-N      do not apply ECC to tags in OOB area (no_tags_ecc)\n");
	printf("\t-c val  chunk size in bytes\n");
	printf("\t-B val  chunks per block\n");
	exit(1);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "bi:c:B:o:hIN")) != -1) {
		switch (c) {
		default:
		case 'h': usage(argv[0]); break;
		case 'i': input_file = strdup(optarg); break;
		case 'o': output_dir = strdup(optarg); break;
		case 'b': endian = 'b'; break;
		case 'c': chunk_size = atoi(optarg); break;
		case 'B': chunks_per_block = atoi(optarg); break;
		case 'I': inband_tags = 1; break;
		case 'N': no_tags_ecc = 1; break;
		}
	}
}

static void invalidate_geometry(void)
{

	chunk_size = 0;
	chunks_per_block = 0;
	block_size = 0;
	n_blocks = 0;

}

static void calc_n_blocks(char *file_name)
{
	off_t file_size;
	int h;

	if (chunk_size < 1000 || chunks_per_block < 16) {
		invalidate_geometry();
		return;
	}
	
	block_size = chunk_size * chunks_per_block;
	
	h = open(file_name, O_RDONLY);
	file_size = lseek(h, 0, SEEK_END);
	close(h);

	printf("input file %s size is %ld\n", file_name, file_size);
	
	n_blocks = file_size / block_size;
	
	if (file_size % block_size)
		printf("warning: file is not an integer number of blocks!\n");
	if (n_blocks < 5) {
		printf("too few blocks\n");
		invalidate_geometry();
	}
}

void dump_directory_tree_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[1000];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			printf("%s inode %d length %d mode %X ",
				str, s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);
							  break;
				default: printf("unknown"); break;
			}

			printf("\n");

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive)
				dump_directory_tree_worker(str,1);

		}

		yaffs_closedir(d);
	}

}

static void dump_directory_tree(const char *dname)
{
	dump_directory_tree_worker(dname,1);
	printf("\n");
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
}

int main(int argc, char *argv[])
{
	struct yaffs_dev * dev;
	int ret;
	
	(void)ret;

	yaffs_trace_mask = 0;

	parse_args(argc, argv);

	if (!input_file) {
		printf("Need input file\n");
		exit(1);
	}

	if (output_dir)
		printf("Generating output directory from file %s into directory %s\n",
			input_file, output_dir);
	else
		printf("Generating listing from file %s\n",
			input_file);
		
	printf("Output file is in %s endian\n",
		(endian == 'l') ? "little" : "big");
		
	calc_n_blocks(input_file);

	/*
	 * Determine oob tags_size.
	 */
	if (inband_tags)
		tags_size = 0;
	else if (no_tags_ecc)
		tags_size = sizeof(struct yaffs_packed_tags2_tags_only);
	else
		tags_size = sizeof(struct yaffs_packed_tags2);

	record_size = chunk_size + tags_size;
	
	printf( "Flash geometry is:\n"
		"    chunk_size........%d\n"
		"    total page size...%d\n"
		"    chunks per block..%d\n"
		"    block size........%d\n"
		"    n blocks..........%d\n"
		"    inband tags.......%s\n",
		chunk_size, record_size, chunks_per_block, block_size, 
		n_blocks,
		inband_tags ? "yes" : "no");

	printf("\n");
	/*
	 * Set up the yaffs object using the simulator.
	 */

	dev = yaffs_flexible_file_sim_create(
				"yroot",
				input_file,
				n_blocks,
				0, n_blocks - 1,
				chunks_per_block,
				chunk_size,
				page_spare_size);

	if (!dev) {
		printf("Failed to create yaffs working file from %s\n", input_file);
		exit(1);
	}


	/*
	 * Read only.
	 */
	dev->read_only = 1;

	/*
	 * Disable checkpointing to create an image without checkpoint data.
	 */
	dev->param.skip_checkpt_rd = 1;
	dev->param.skip_checkpt_wr = 1;

	/*
	 * Set up stored endian: 1 = little endian, 2 = big endian.
	 * Set no_tags_ecc
	 */
	dev->param.stored_endian = (endian == 'l') ? 1 : 2;
	dev->param.no_tags_ecc = no_tags_ecc;

	dev->param.inband_tags = inband_tags;

	ret = yaffs_mount3("yroot", 1, 1);

	printf("yaffs_mount returned %d\n", ret);

	if (ret < 0) {
		printf("Mounting yaffs simulator failed - cannot continue\n");
		exit(1);
	}

	dump_directory_tree("yroot");

	yaffs_unmount("yroot");
	
	
	return 0;
}
