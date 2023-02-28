/**************************************************************
* Names: Daniel Chang

* GitHub Name:danieljchang
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
* This file is where you will start and initialize your system
*
**************************************************************/



#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "structs.h"
#define SIGNATURE 0x21A23A35E32897
#define DIRECTORYMAX 72

// typedef struct directoryEntry{


// we are making the spacemap global then we can check how big the space
// the space map will be put into the disk and it will tell us everything we want
// to know about where free space is on disk
int* spaceMap;
// Volume control block is global so we can access it whenever
VCB* vcb;
directoryEntry *cwd;
//every directory is an array of directory entries but every directory entry needs a directory entry
int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	//vcb = malloc(sizeof(VCB));
	// 
	//de[69];
	directoryEntry *de;
	vcb = malloc(blockSize);

	// the first thing we need to check is if the vcb is written to the disk
	// the disk will be permanent per each run. If the signature that we are
	// reading is not the siganture of the user than we need to initiate everythign again.
	// The problem is that we would essentially overwrite a directory entry of some other
	// signature.
	LBAread(vcb, 1, 0);
	if (vcb->signature == SIGNATURE){
		// because the signature is the same we need to read the vcb again and reset cwd
		// otherwise we will never have these values when running the program.
		spaceMap=malloc(numberOfBlocks*sizeof(int));
		cwd = malloc(blockSize*calcBlocks(DIRECTORYMAX*sizeof(directoryEntry)));
		strcpy(vcb->pathName, "/");
		LBAread(cwd, calcBlocks(DIRECTORYMAX*sizeof(directoryEntry)) ,vcb->rootDirectory.location);
		       // printf("return info location %d\n", cwd[0].location);
		return 0;
		//return 0;
	}
	// allocating the values to vcb that we get.
	vcb->blockCount = numberOfBlocks;
	vcb->freeBlockCount = numberOfBlocks;
	vcb->blockSize = blockSize;
	vcb->signature = SIGNATURE;
	cwd = malloc(blockSize*calcBlocks(DIRECTORYMAX*sizeof(directoryEntry)));
	//printf("size of a directory entry %d\n", sizeof(directoryEntry));
	// 66 DE, each DE is 88 Bytes
	// 1 block is 512 bytes
	// 66*88 = 5808 bytes total
	// 5808/512 = 11.34
	// (number of bytes for de + sizeof 1 block - 1)/ sizeof(de)
	// int x = sizeof(DE)*#ofDE % blocksize 
	// int y = bloxck size - x
	// 66*80+y
	// number of DE*sizeof(de) + (blocksize - sizeof(DE)*#number of DE % blocksize ) )/ sizeof(DE);
	

	// we have a pointer of directory entries because we want to check 
	// the directory entry that could have more than one file in a directory entry.
	// a Directory entry is an array of directory entries, because we can have
	// multiple directories and or files.
	// 69*88 /512 = 11.85 
	// 88/512 = 0.17
	/* TODO: Add any code you need to initialize your file system. */
	

	spaceMap=malloc(numberOfBlocks*sizeof(int));
	//printf("\n\n%li number of blocks in spaceMap\n",numberOfBlocks*sizeof(int));
	// the blocks needed here is a non 0 value, so we know how many blocks are needed
	// for the space map.
	int blocksNeeded = calcBlocks(vcb->blockCount * sizeof(int));
	//mapping all space as free initiating
	// since FAT is just a bunch of pointers, we point each index to the next
	// This makes it easy for contiguos blocks and possibly non-contiguious.
	for(int i=1;i<numberOfBlocks;i++){
		spaceMap[i]=i+1;
	}

	// marking the end of free space
	spaceMap[numberOfBlocks-1] = 0xFFFE;
	// first free block is inex 0
	vcb->firstFreeBlock = 0;
	//printf("%i first free block\n\n", vcb->firstFreeBlock);
	// calculating the blocks we would need for the directory entry array
	blocksNeeded = calcBlocks(sizeof(directoryEntry)*72 );
	//intiiating all of the . and ..
	strcpy(vcb->rootDirectory.name, ".");
	vcb->rootDirectory.size=sizeof(directoryEntry)*72;
	vcb->rootDirectory.flag = 'D';

	// To check that root and parent are different
	//After we intiate everything at we want. we can start writing, because otherwise our disk
	// will have outdated or old data. By doing this all at the end we are getting new info
	// Writing VCB

	LBAwrite(vcb, 1, 0);
	// After each LBAwrite we need to decrease or change the values of free space
	// and how much space we have for later usage.
	vcb->freeBlockCount--;
	vcb->firstFreeBlock = spaceMap[vcb->blockCount - vcb->freeBlockCount - 1];
	// Creating a flag at the end of each block, that tells us that this is the end
	// of the thing that we wrote into LBA.
	spaceMap[0]=0xAAAA;

	// Writing Free space map


	// Here we are calculating the changes and the size of the space map.
	// we reallocate all of these values because we need to keep these updated so that
	// we know where the free blocks are based on the vcb variable.
	blocksNeeded = calcBlocks(vcb->blockCount*sizeof(int));
	int spaceMapRet = LBAwrite(spaceMap, blocksNeeded, 1);
	vcb->freeBlockCount = vcb->freeBlockCount - blocksNeeded;
	vcb->firstFreeBlock = spaceMap[vcb->blockCount-vcb->freeBlockCount - 1];
	spaceMap[vcb->blockCount-vcb->freeBlockCount]=0xAAAA;
	// intializing the directory entry array
	de = malloc (sizeof(directoryEntry) * DIRECTORYMAX);
	for (int i = 0; i < DIRECTORYMAX; i++){
		strcpy(de[i].name,"");
	}


	// directory init and we pass NULL values because we do no have a parent
	// this also means we are intiialzing root.
	int address = directoryInit(NULL, "");
	if (address == -1){
		//printf("hehe");
		return -1;
	}


	// since we write inside of directoryInit, we can read the directory entry array into cwd
	// to stay updated.
	LBAread(cwd, calcBlocks(DIRECTORYMAX*sizeof(directoryEntry)), vcb->rootDirectory.location);
	//printf("this is the cwd: %s\n",cwd->name);
	// Rewriting it to update the directory entry that we just added. Spacemap

	return 0;

}



void findFreeBlock(int size){// 0123, 4 
	vcb->firstFreeBlock += size;

}



int deSize = 72;
int directoryInit(directoryEntry *parent, char* newFile){
	//LBAread(vcb, 1, 0);
	// to dynamically allocate memory. to maximize the size of directory entryy
	deSize = ((DIRECTORYMAX*sizeof(directoryEntry)) + (vcb->blockSize - 
		((sizeof(directoryEntry)*DIRECTORYMAX) % vcb->blockSize)))/sizeof(directoryEntry);
	//printf("\n\n\nTHIS IS HERE+++-->%d",deSize);
	// initializing everything to being empty
	directoryEntry * de;
	de = malloc(deSize*sizeof(directoryEntry));
	for(int i =0;i<deSize;i++){
		de[i].flag = 'E';
	}
	//printf("deszie %i\n", deSize);
	// using this as a way of getting the block that we want to write to.
	int address = vcb->firstFreeBlock;
	// calculate where to put the directory array on disk
	int blocks = calcBlocks(sizeof(directoryEntry) *deSize);
	if (vcb->freeBlockCount < blocks){
		printf("Not enough space on disk, cannot allocate directoryEntry\n");
		return -1;
	}
	//initializing root . and ..
	// allocated blocks on disk for dir
	directoryEntry dir;
	dir.location = address;
	dir.size = sizeof(directoryEntry) * deSize;
	dir.flag = 'D';	

	
	vcb->rootDirectory.maxNumEntries = deSize;
	
	//de[1] = (parent == NULL)?dir:parent[0];

	
	strcpy(de[0].name, ".");
	de[0].size = dir.size;
	de[0].flag = 'D';
	de[0].location = dir.location;
	strcpy(de[1].name, "..");
	de[1].size = dir.size;
	de[1].flag = 'D';
	// determining if the directoryInit is root or something else;
	de[1].location = de[0].location;
	//de[1].location = address;
	int blocksNeeded;
	//if parent == null, its root
	// double checking if parent is null then the pointter is rootdirectory
	if (parent == NULL){
		//strcpy(de[2].name, "foo");
		memcpy(&vcb->rootDirectory,&de[0],sizeof(vcb->rootDirectory));
		//vcb->rootDirectory = de[0];
		strcpy(vcb->rootDirectory.name, ".");
		vcb->rootDirectory.flag = 'D';
		vcb->rootDirectory.location = address;
		strcpy(vcb->pathName, "/");
		//printf("vcb pathname %s\n", vcb->pathName);
		LBAwrite(vcb,1,0);
	}

	// We are rewriting it in a new position instead of overwriting the old posistion when we are in the root

	

	//int blocksNeeded = calcBlocks(sizeof(directoryEntry)*deSize);

	//printf("\n\n-------->>>>>%ld\n\n\n",parent[0].location);
	blocksNeeded = calcBlocks(sizeof(directoryEntry)*deSize);

	//printf("%s location: %ld\n", de->name, de[0].location);

	// updating directory entry into disk
	LBAwrite(de, blocksNeeded, de[0].location);
	// vcb->freeBlockCount = vcb->freeBlockCount - blocksNeeded;
	spaceMap[vcb->blockCount-vcb->firstFreeBlock]=0xAAAA;
	// vcb->firstFreeBlock = spaceMap[vcb->blockCount-vcb->freeBlockCount];
	//LBAwrite(vcb,1,0); 

	// updating space map into disk
	blocksNeeded = calcBlocks(vcb->blockCount*sizeof(int));
	int spaceMapRet = LBAwrite(spaceMap, blocksNeeded, 1);
	LBAwrite(vcb,1,0); 
	
	return address;
}
int calcBlocks(int numOfBytes){
	return (numOfBytes + vcb->blockSize - 1 )/ vcb->blockSize;
}
// function to update our disk writes everything and reads cwd
int writeFreeSpaceMap(int blocks){ // 
	LBAread(vcb, 1, 0);
	LBAread(spaceMap, calcBlocks(vcb->blockCount*sizeof(int)), 1 );
	vcb->firstFreeBlock = spaceMap[vcb->blockCount-vcb->freeBlockCount - 1];
	spaceMap[vcb->firstFreeBlock-1]=0xAAAA;
	LBAwrite(vcb, 1, 0);


	LBAwrite(spaceMap, calcBlocks(vcb->blockCount*sizeof(int)), 1);
	LBAread(cwd, calcBlocks(sizeof(directoryEntry)*deSize), cwd[0].location);
	return 0;
}
// a function for our bio since we need location of our spacemap and where we are writing.
int getNextBlock(int location){
	  return spaceMap[location];
}
void exitFileSystem ()
	{
		free(spaceMap);
		spaceMap = NULL;
		free(vcb);
		vcb = NULL;
		free(cwd);
		cwd = NULL;
	printf ("System exiting\n");

	}