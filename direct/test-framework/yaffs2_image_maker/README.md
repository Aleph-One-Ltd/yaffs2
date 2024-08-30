# How to use the image maker and dumper


# Image Maker

The purpose of this image maker is to create image files that can be burned into NAND.

This image maker is set up to create image files for a flash that is:
* 2k pages used ad "chunks"
* 64 pages per block.
* Inband tags

If those are not the parameters that you require then it is easy enough to modify the tool.

The image maker takes a sub-directory and copies all the files and directories over to a yaffs NAND simulation.
The data from the NAND simulator simulates pages with 2k bytes of data area, then 64 bytes of spare.
Since the spare is not used (because inband tags are used), the final step is to copy the data part of each page
into the final image file.


The resulting inmage file is a multiple of 2k bytes.

To program this into a NAND flash:

1. Erase the NAND flash.
2. Put the flash device into ECC mode. The NAND will then write the ECC data into the spare area automatically.
3. Starting at the first block to be used for Yaffs:
   a. Check if the block is good. If it is bad skip over that block.
   b. For 64 times (ie. each page in the block)
     i.   Read a 2k byte page of data from the image file.
     ii.  Write that 2k bytes into the page of flash.
     iii. Verify if required.
     iv.  If the write fails, then mark the block as bad and rewrite the data in the next block.

Note:
 It is Ok if there are bad blocks. Yaffs is designed to handle bad blocks.


# Dumper

The purpose of the dumper is to view or extract files from a Yaffs2 flash image.

Note that the input image file must be large enough to mount which typically means at
least 8 or so blocks.

You can pad out the file using something like:

```
dd if=/dev/zero ibs=128k count=10 | LC_ALL=C tr "\000" "\377" >padded_file.bin
dd if=origin_file.bin of=padded_file.bin conv=notrunc
./yaffs2_dumper -i padded_file.bin -I -c 2048 -B 64
```
Output is something like:

```
Generating listing from file padded_file.bin
Output file is in little endian
input file padded_file.bin size is 1310720
Flash geometry is:
    chunk_size........2048
    total page size...2048
    chunks per block..64
    block size........131072
    n blocks..........10
    inband tags.......yes

yaffs_mount returned 0
yroot/lost+found inode 2 length 2032 mode 41C0 directory
yroot/lost+found/obj296 inode 296 length 2032 mode 4000 directory
yroot/lost+found/obj296/expand inode 312 length 17 mode A1FF symlink -->"../../bin/busybox"
yroot/lost+found/obj296/sv inode 313 length 17 mode A1FF symlink -->"../../bin/busybox"
yroot/lost+found/obj296/man inode 314 length 17 mode A1FF symlink -->"../../bin/busybox"

Free space in yroot is 904240
```