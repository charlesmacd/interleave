/*
	Program:
		interleave
	File:
		main.cpp
	Author:
		Charles MacDonald
	License:
		Freeware
	Notes:
		None	
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


/*----------------------------------------------------------------------------*/
/* Definitions */
/*----------------------------------------------------------------------------*/

#define MAX_FILES	2	

typedef unsigned char uint8;

enum file_err
{
	ERR_FILE_NONE,
	ERR_FILE_OPEN,
	ERR_FILE_SIZE,
	ERR_FILE_ALLOC,
	ERR_FILE_INCOMPLETE,
	MAX_ERR
};

/*----------------------------------------------------------------------------*/
/* Test if a file exists */
/*----------------------------------------------------------------------------*/

bool fexist(const char *filename)
{
	FILE *fd = NULL;

	/* Attempt to open input file */
	fd = fopen(filename, "rb");

	/* File doesn't exist */
	if(!fd)
		return false;

	/* File exists, close handle */
	fclose(fd);
	
	return true;
}

/*----------------------------------------------------------------------------*/
/* Save binary data to a file */
/*----------------------------------------------------------------------------*/

int save_binary_file(uint8 *buf, size_t size, char *filename)
{
	FILE *fd = NULL;

	try
	{
		/* Attempt to open output file */
		fd = fopen(filename, "wb");
		if(!fd)
			throw ERR_FILE_OPEN;

		/* Write data to output file */
		if(fwrite(buf, 1, size, fd) != size)
			throw ERR_FILE_INCOMPLETE;

		/* Close output file */
		fclose(fd);			
	}
	catch(file_err status)
	{
		/* Release file handle if open */
		if(fd != NULL)
			fclose(fd);
			
		/* Close input file */
		return status;
	}
	
	return ERR_FILE_NONE;
}

/*----------------------------------------------------------------------------*/
/* Load binary data from a file */
/*----------------------------------------------------------------------------*/

int load_binary_file(uint8 *&buf, size_t &size, const char *filename)
{
	FILE *fd = NULL;

	buf = NULL;
	size = 0;

	try
	{
		/* Attempt to open file for reading */
		fd = fopen(filename, "rb");
		if(!fd)
			throw ERR_FILE_OPEN;

		/* Get file size */
		/* TODO: use stat call, faster */
		fseek(fd, 0, SEEK_END);
		size = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		
		/* If file is empty, abort */
		if(size == 0)
			throw ERR_FILE_SIZE;

		/* Attempt to allocate buffer for file data */
		buf = new uint8 [size];
		if(!buf)
			throw ERR_FILE_ALLOC;

		/* Clear out buffer */
		memset(buf, 0, size);

		/* Attempt to read file data */
		if(fread(buf, 1, size, fd) != size)
			throw ERR_FILE_INCOMPLETE;

		/* Close input file */
		fclose(fd);
	}
	catch(file_err status)
	{
		/* Release file handle if open */
		if(fd != NULL)
			fclose(fd);

		/* Deallocate file buffer if allocated */
		if(buf != NULL)
			delete []buf;

		return status;
	}

	/* No error */
	return ERR_FILE_NONE;
}

/*----------------------------------------------------------------------------*/
/* Main program */
/*----------------------------------------------------------------------------*/

int main (int argc, char *argv[])
{
	int verbose = 1;
	uint8 *buf[MAX_FILES];
	uint8 *result;
	size_t size[MAX_FILES];
	size_t result_size;

	/* Warn user if insuffient arguments given*/
	if(argc < 3)
	{
		printf("Interleave two 8-bit files into a 16-bit file.\n");
		printf("usage: %s even.bin odd.bin <output>\n", argv[0]);
		return 0;
	}

	/* Load files */
	for(int index = 0; index < MAX_FILES; index++)
	{
		if(load_binary_file(buf[index], size[index], argv[1+index]) != ERR_FILE_NONE)
		{
			printf("Error: Can't read file `%s'.\n", argv[1+index]);
			return 0;
		}
	}

	/* Warn if input file sizes are not equal */
	if(size[0] != size[1])
	{
		printf("Error: Files are not the same size.\n");
		return 0;
	}

	if(verbose)
		printf("Input size: %lu bytes.\n", (unsigned long)size[0]);

	/* Allocate result buffer */
	result_size = size[0] * 2;
	result = new uint8 [result_size];	
	if(!result)
	{
		printf("Error: Can't allocate memory for output.\n");
		return 0;
	}

	/* Merge input data */
	for(size_t index = 0; index < size[0]; index++)
	{
		result[index << 1 | 0] = buf[0][index];
		result[index << 1 | 1] = buf[1][index];
	}

	/* Determine output filename */
	char *output_name = (char *)"output.bin";	
	if(argc == 4)
		output_name = (char *)argv[3];

	/* Warn user if output file already exists */
	if(fexist(output_name))
	{
		printf("Error: Output file `%s' already exists.\n", output_name);
		return 0;
	}

	if(verbose)
		printf("Output size: %lu bytes.\n", (unsigned long)result_size);

	if(verbose)
		printf("Writing result to file `%s'.\n", output_name);

	/* Write merged file to disk */	
	save_binary_file(result, result_size, output_name);

	return 1;
}


/* End */

