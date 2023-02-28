/**************************************************************
* Names: Daniel Chang

* GitHub Name:danieljchang
* Project: Basic File System
*
* File: b_io.c
*
* Description: this file is responsible for creating and iterating
*or editing a file. b_io means buffered input output which are 
*the two actions that we can perform on a file read or write
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"
#include"structs.h"


#define MAXFCBS 20	//max number of file descriptors
#define B_CHUNK_SIZE 512	//size of a block
#define DEFAULTFILEBLOCKS 100	//default block count of a file
VCB *vcb;

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int currByte;   //current byte in the file
	int fileLength; //amount of bytes written to file, not the size of allocated space!
	int totalNumBlocks;   //amount of space allocated for the file
	directoryEntry fileEntry; //directory entrty of the file being opened
	int perms;				//trakcs the flags associated with the file open
	int curBlock;			//current block of the file being read 1-100


	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd==-1){{
		puts("invalid descriptor");
		return -1;
	}}
	//extract the path, use the pathinfo object in order to access  the file
	pathInfo info = parsePath(filename);
	int size = strlen(filename);
	int i = size;
	char *temp = malloc(sizeof(char)*size-i);
	strcpy(temp,filename+i);
	//retrieve last part of path
	for(i;i>0;i--){

		if(filename[i]=='/'){

				break;
		}

	}
	//the path did not lead to a file
	if(info.status==-1){
		puts("THE PATH IS INVALID");
		return -1;
	}
	if(info.status==1){
		//if the file has not been created and the create flag is set
		//create directory entry and fd
		if((flags&O_CREAT)==O_CREAT){


			//intializing the directory entry
			strcpy(info.directory[info.index].name,temp);
			info.directory[info.index].location = vcb->firstFreeBlock;
			vcb->firstFreeBlock = vcb->firstFreeBlock+DEFAULTFILEBLOCKS;
			vcb->freeBlockCount = vcb->freeBlockCount - DEFAULTFILEBLOCKS;
			LBAwrite(vcb,1,0);

			info.directory[info.index].flag = 'F';
			info.directory[info.index].size = vcb->blockSize*DEFAULTFILEBLOCKS;
			LBAwrite(info.directory,calcBlocks(sizeof(directoryEntry)*72),info.directory[0].location);
			memcpy(&fcbArray[returnFd].fileEntry,&info.directory[info.index],sizeof(directoryEntry));
			fcbArray[returnFd].fileLength = 0;
			//init fd
			fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
			fcbArray[returnFd].buflen = B_CHUNK_SIZE;
			fcbArray[returnFd].perms = flags;
			fcbArray[returnFd].totalNumBlocks = DEFAULTFILEBLOCKS;
			fcbArray[returnFd].currByte = 0;
			fcbArray[returnFd].curBlock = info.directory[info.index].location;
			
			

		}else{
			puts("FILE DOES NOT EXIST AND CREATE FLAG NOT SET");
		}

	}
	if(info.status==0){
		//case in which the file exists, we just set the fd
		//make sure descriptor doesnt already exist
		//just set the values of the fcb array element at fdir to the directory entry info
		for(int i = 0;i<MAXFCBS;i++){

			if(strcmp(fcbArray[i].fileEntry.name,temp)==0){

				puts("an FD already exists for this file");
				return -1;				
			}
		}
		memcpy(&fcbArray[returnFd].fileEntry,&info.directory[info.index],sizeof(directoryEntry));
		//initializing fd to related de values
		fcbArray[returnFd].fileLength = info.directory[info.index].size;
		fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
		fcbArray[returnFd].buflen = B_CHUNK_SIZE;
		fcbArray[returnFd].perms = flags;
		fcbArray[returnFd].totalNumBlocks = DEFAULTFILEBLOCKS;
		fcbArray[returnFd].currByte = 0;
		fcbArray[returnFd].curBlock = fcbArray[returnFd].fileEntry.location;

		if((flags&O_TRUNC)==O_TRUNC){
			//if trunc flag set, set filelength to 0 EMPTY
			if((flags&O_RDONLY!=O_RDONLY)){

				fcbArray[returnFd].fileLength = 0;

			}

		}

		
		


	}
	return (returnFd);						// all set
	}


// changes which byte that is currently being viewed	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	//case in which we set our location in file to the specified value
	if(whence==SEEK_SET){

		fcbArray[fd].currByte = offset;
		
	}
	//case in which we add the specified value to our byte position in file
	if(whence==SEEK_CUR){

		fcbArray[fd].currByte+=offset;

	}
	//case in which our position begins where the data ends
	if(whence==SEEK_END){

		fcbArray[fd].currByte = fcbArray[fd].fileLength + offset;
	}
		
	return (fcbArray[fd].currByte); 
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
		//we want to read in the current block from disk into memory and capture the position (bytes) we are currently at
		
		//loop:
		//calculate the remaining bytes of space to be filled in the block
		
		//copy in the count number of bytes left to be filled from buffer to volume
		
		//if count<space remaining in the block: add count to block, write that block to disk

		//if count>space remaining in the block: fill the remaining space, write that block to disk, repeat loop.
		//the block in whichour offset resides
		int locationinFile = fcbArray[fd].curBlock+fcbArray[fd].currByte/B_CHUNK_SIZE;
		//the offset inside of the block^^
		int fileOffset = fcbArray[fd].curBlock+fcbArray[fd].currByte%B_CHUNK_SIZE;
		//TTHIS IS WHERE WE LEFT OFF


		
		
	return (fd); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{
	int blocksRead;
	int bytesReturned;
	int part1, part2, part3;
	int numberOfBlocksToCopy;
	int remainingBytesInMyBuffer;

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		// this is the number of bytes to copy from the buffer.
	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;	
	// How much we have left to deliver
	int amountAlreadyDelivered = (fcbArray[fd].curBlock * B_CHUNK_SIZE) - remainingBytesInMyBuffer;
	//int amountAlreadyDelivered = fcbArray[fd].buflen - remainingBytesInMyBuffer;
	// This lne is basically what determines the limit of the file. 
	// Now our bio is much easier to handle
	if ((count + amountAlreadyDelivered) > fcbArray[fd].fileEntry.size){
		count = fcbArray[fd].fileEntry.size - amountAlreadyDelivered;
		if (count < 0){
			printf("Error: Count: %d   Delivered: %d   CurBlock: %d", count, amountAlreadyDelivered, fcbArray[fd].curBlock);		
		}

	}
	// Initializing part123. If we have more space in our buffer then we need to read
	if (remainingBytesInMyBuffer >= count){
		part1 = count;
		part2 = 0;
		part3 = 0;
	}else{ // otherwise we fill the rest of the buffer and split it into parts
		part1 = remainingBytesInMyBuffer;
		part3 = count - remainingBytesInMyBuffer;
		numberOfBlocksToCopy = part3/B_CHUNK_SIZE;
		part2 = numberOfBlocksToCopy * B_CHUNK_SIZE;
		part3 = part3 - part2;
	}	
	//this first part the rest of the buffer given some amount of bytes that need to be read
	if (part1 > 0){
		// we memcpy the leftovers into the buffer.
		memcpy(buffer,fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
	}
	// part 2 is filling entire blocks at a time Since we have de's with
	// size of multiple blocks we use our space map to iterate through the blocks
	// and read the blocks into our buffer.
	if (part2 > 0){
		blocksRead = 0;
    	for (int i = 0; i < numberOfBlocksToCopy; i++){
      		blocksRead += LBAread(buffer + part1 + (i * vcb->blockSize), 1, fcbArray[fd].curBlock);
      		fcbArray[fd].curBlock = getNextBlock(fcbArray[fd].curBlock);
    	}
    	fcbArray[fd].index += blocksRead;
    	part2 = blocksRead * B_CHUNK_SIZE;
    }
	// part 3 are the leftovers from our buffer. the leftovers are not enough
	// to fill an entire block so we just read the rest of the buffer in, 
	// not fully filling the buffer up.
  	if (part3 > 0){
    	blocksRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].curBlock);
    	fcbArray[fd].buflen= B_CHUNK_SIZE;
		// using the space map we can find the next block that needs to be read but
		// in this case it is not a different file, we also know this because
		// curBlock will now be a pointer to some random flag value
    	fcbArray[fd].curBlock = getNextBlock(fcbArray[fd].curBlock);
    	fcbArray[fd].index += 1;
    	fcbArray[fd].index = 0;
     	memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
      	fcbArray[fd].index += part3;
    }


	// The sum of the 3 parts are the total # of blocks read.
  	return part1 + part2 + part3;
}

	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{

	}