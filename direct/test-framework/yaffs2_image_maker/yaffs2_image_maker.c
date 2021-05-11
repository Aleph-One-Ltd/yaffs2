/*
 * An image maker that creates a file that creates a Yaffs2 image file
 * can be programmed into NAND.
 * This assumes that the device has the following properties:
 * * 2k bytes per page
 * * 64  pages per block
 * * Yaffs is using inband tags.
 *
 * Note that this utility first generates a "working file" which is
 * a simulation of the NAND. It then generates an output file which
 * can be programmed into flash.
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


/*
 * These are the sizes in the simulator file.
 */

#define MAX_BLOCKS		3000
#define PAGES_PER_BLOCK_DEFAULT	64
#define PAGE_DATA_SIZE_DEFAULT	2048
#define PAGE_SPARE_SIZE 	64
#define FULL_PAGE_SIZE	(chunk_size + PAGE_SPARE_SIZE)


/* Some stub definitions to get building to work. */
int simulate_power_failure = 0;
int random_seed = 0;

static char *input_dir;
static char *output_file;
static char *working_file;
static char endian = 'l';
static int no_tags_ecc = 0;
static int inband_tags = 0;

static int tags_size;
static int record_size;
static int total_written;
static int chunk_size = PAGE_DATA_SIZE_DEFAULT;
static int chunks_per_block = PAGES_PER_BLOCK_DEFAULT;

static void usage(const char *prog_name)
{
	printf("Usage: %s options\n", prog_name);
	printf("\t-i name input_directory\n");
	printf("\t-o name output_file\n");
	printf("\t-w name working_file\n");
	printf("\t-b      big endian output\n");
	printf("\t-I      store tags in flash data area (inband_tags)\n");
	printf("\t-N      do not apply ECC to tags in OOB area (no_tags_ecc)\n");
	printf("\t-c val  chunk size in bytes (default %d)\n", PAGE_DATA_SIZE_DEFAULT);
	printf("\t-B val  chunks per block(default %d)\n", PAGES_PER_BLOCK_DEFAULT);
	exit(1);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "bi:c:B:o:w:hIN")) != -1) {
		switch (c) {
		default:
		case 'h': usage(argv[0]); break;
		case 'i': input_dir = strdup(optarg); break;
		case 'o': output_file = strdup(optarg); break;
		case 'w': working_file = strdup(optarg); break;
		case 'b': endian = 'b'; break;
		case 'c': chunk_size = atoi(optarg); break;
		case 'B': chunks_per_block = atoi(optarg); break;
		case 'I': inband_tags = 1; break;
		case 'N': no_tags_ecc = 1; break;
		}
	}
}


static int process_file(const char *from_path, const char *to_path, unsigned mode)
{
	int hin;
	int hout;
	unsigned char buffer[8000];
	int nread;
	int nwritten;
	int nbytes = 0;
	int ret;

	hin = open(from_path, O_RDONLY);
	if (hin < 0) {
		perror ("opening input file");
		return -1;
	}
	hout = yaffs_open(to_path, O_CREAT | O_TRUNC | O_RDWR, 0666);

	if(hout < 0) {
		printf("failed to create yaffs file %s\n", to_path);
		return -1;
	}

	while ((nread = read(hin, buffer, sizeof(buffer))) > 0) {
		nwritten = yaffs_write(hout, buffer, nread);

		if (nwritten != nread) {
			printf("Only wrote %d bytes out of %d\n", nwritten, nread);
			return -1;
		}
		nbytes += nwritten;
	}

	ret = yaffs_fdatasync(hout);
	if (ret < 0) {
		printf("data sytnc failed\n");
		return -1;
	}

	ret = yaffs_fchmod(hout, mode);
	if (ret < 0) {
		printf("chmod failed\n");
		return -1;
	}

	ret = yaffs_close(hout);
	if (ret < 0) {
		printf("close failed\n");
		return -1;
	}

	return nbytes;
}

static int process_directory(const char *from_dir, const char *to_dir)
{
	int error = 0;

	DIR *dir;
	struct dirent *entry;

	printf("Processing directory %s into %s\n", from_dir, to_dir);

	dir = opendir(from_dir);
	if(!dir) {
		printf("opendir failed on %s", from_dir);
		return -1;
	}

	while((entry = readdir(dir)) != NULL  && error >= 0) {
		char from_path[500];
		char to_path[500];
		struct stat stats;
		unsigned  mode;
		int ret;

		//printf("Got entry %s\n", entry->d_name);

		/* Ignore . and .. */
		if(strcmp(entry->d_name, ".") == 0 ||
		   strcmp(entry->d_name, "..") == 0)
		   continue;

		if (snprintf(from_path, sizeof(from_path),
			    "%s/%s", from_dir,entry->d_name) >= (int)sizeof(from_path)) {
			printf("path too long for %s/%s\n", from_dir, entry->d_name);
			error = -1;
			continue;
		}
		if (snprintf(to_path, sizeof(to_path),
			    "%s/%s",to_dir,entry->d_name) >= (int)sizeof(to_path)) {
			printf("path too long for %s/%s\n",to_dir,entry->d_name);
			error = -1;
			continue;
		}

		if (lstat(from_path,&stats) < 0)
		{
			perror("lstat");
			error = -1;
			continue;
		}

		mode = stats.st_mode & 0777;

		if (S_ISDIR(stats.st_mode)) {
			printf("Directory create %s, mode %03o\n", to_path, mode);
			ret = yaffs_mkdir(to_path, mode);
			printf("Directory create %s, mode %03o : result %d\n",
				to_path, mode, ret);

			if (ret < 0) {
				printf("directory creation failed\n");
				error = -1;
			}
			process_directory(from_path, to_path);
		} else if (S_ISREG(stats.st_mode)) {
			ret = process_file(from_path, to_path, mode);
			printf("Copy file %s to %s, mode %03o : result %d\n",
				from_path, to_path, mode, ret);
			if (ret < 0) {
				printf("file copy failed\n");
				error = -1;
			}

		} else {
			printf("Unhandled object type %d\n", stats.st_mode);
		}
	}

	closedir(dir);

	return 0;

}

int is_all_ff(unsigned char *buffer, int n)
{
	while (n > 0) {
		if (*buffer != 0xff)
			return 0;
		buffer++;
		n--;
	}

	return 1;
}

/*
 * Write the data to output, skipping over the spare bytes.
 * Stop writing when we get to blank flash.
 */
int generate_output_file(const char *working_file, const char *output_file)
{
	unsigned char *buffer;
	int inh;
	int outh;
	int nread;
	int nwritten;

	buffer = malloc(FULL_PAGE_SIZE);

	inh = open(working_file, O_RDONLY);
	outh = open(output_file, O_CREAT | O_TRUNC | O_RDWR, 0666);

	while ((nread = read(inh, buffer, FULL_PAGE_SIZE)) > 0) {
		if (nread != FULL_PAGE_SIZE) {
			printf("working file not properly sized\n");
			return -1;
		}

		if(is_all_ff(buffer, chunk_size)) {
			printf("End of data found\n");
			break;
		}

		/* Write the data part. */
		nwritten = write(outh, buffer, chunk_size);

		if (nwritten != chunk_size) {
			printf("short write\n");
			return -1;
		}

		total_written += nwritten;

		/* Now if there are OOB tags then write the OOB tags part too */
		if (tags_size > 0) {
			/* Read the oob bytes. In the simulator these are
			 * stored at offset 0 in the 64-byte spare area.
			 * We must therefore copy them from this location to
			 * the output file.
			 */
			nwritten = write(outh, buffer + chunk_size, tags_size);

			if (nwritten != tags_size) {
				printf("short write\n");
				return -1;
			}

			total_written += nwritten;
		}
	}

	close(inh);
	close(outh);

	return total_written;
}


int main(int argc, char *argv[])
{
	struct yaffs_dev * dev;
	int ret;

	yaffs_trace_mask = 0;

	parse_args(argc, argv);

	if (!input_dir || !output_file || !working_file) {
		printf("Need input directory , output file and working file\n");
		exit(1);
	}

	printf("Generating image from %s into file %s using working file %s\n",
		input_dir, output_file, working_file);
	printf("Output file is in %s endian\n",
		(endian == 'l') ? "little" : "big");

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

	printf("\n");
	/*
	 * Create the Yaffs working file using the simulator.
	 */
	unlink(working_file);

	dev = yaffs_flexible_file_sim_create(
					"yroot",
				working_file,
				MAX_BLOCKS,
				0, MAX_BLOCKS - 1,
				chunks_per_block,
				chunk_size,
				PAGE_SPARE_SIZE);

	if (!dev) {
		printf("Failed to create yaffs working file\n");
		exit(1);
	}

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

	ret = yaffs_mount("yroot");

	printf("yaffs_mount returned %d\n", ret);

	if (ret < 0) {
		printf("Mounting yaffs simulator failed - cannot continue\n");
		exit(1);
	}

	process_directory(input_dir, "yroot");

	yaffs_unmount("yroot");


	printf("Generating output file: %s\n", output_file);

	ret = generate_output_file(working_file, output_file);

	printf("Done\n\n");
	printf("Wrote %d bytes (%d pages of %d bytes each) to output file.\n",
		total_written, total_written/record_size, record_size);
	printf("This images has %d bytes per chunk, %d chunks per block.\n",
		chunk_size, chunks_per_block);
	if (inband_tags) {
		printf("The image has inband tags.\n");
		printf("This means it is structured a records of %d bytes per page\n", record_size);
		printf("This should be written to the data portion of the page\n");
	} else {
		printf("The image has out of band tags with%s ECC on the tags\n",
			no_tags_ecc ? " NO" : "");
		printf("This means it has %d tags bytes per record to be written in the oob area\n", tags_size);
		printf("Each record is %d bytes.\n"
			"The first %d bytes are data for the data datea and\n"
			"the last %d bytes are tags for the oob (spare) area\n",
			record_size, chunk_size, tags_size);
	}
	if (ret < 0)
		exit(1);

	return 0;
}
