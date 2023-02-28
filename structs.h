/**************************************************************
* File: structs.c
*
* Description: A place for all of our global structs and variables
* for ease of access.
*
*
**************************************************************/

#ifndef _STRUCTS_H
#define _STRUCTS_H
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
// padding is annoying, but made each de a quarter of a block for easier hexdump read
typedef struct directoryEntry{
	time_t created;	// wont use 
	time_t modified; // wont use
	time_t accessed; // wont use
	long location;
	// the number of bytes for a directory or file
	unsigned long size;
	// the number of entries that exist
	unsigned int maxNumEntries;

	char name[83];

	char flag;  // this will D = directory, F = File
}directoryEntry;

typedef struct VCB{
	long signature;
	int blockCount;
	int blockSize;
	int freeBlockCount;
	int firstFreeBlock;
	int freeFCBCount;
	int FCBPointer;
	directoryEntry rootDirectory;
	char pathName[128];

}VCB; 


typedef struct pathInfo{
	int status;
	int index;
	directoryEntry *directory;
} pathInfo;

extern VCB* vcb;
extern int deSize;
extern directoryEntry *cwd;
extern char entirePath[255];

#endif