/**************************************************************
* File: parsepath.c
*
* Description: The file where we parse the path string
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
VCB* vcb;
directoryEntry *cwd;
char entirePath[255];

int deSize;
// SHOULD BE THE SAME THING as fsInit.c
pathInfo parsePath(const char* path){
    // By creating a copy, we will have a copy of the path and we can parse it ahead of
    // our actual parse string. to check if the parser has nothing left.
    
    char *savePointer;
    char *tempPath = malloc(sizeof(path)+1);

    char *deName = malloc(sizeof(path));
    pathInfo info;
        //printf("strtok_r is failing\n");
    // allocating and intializing everything to a value so we dont get random NULL values
    info.directory = malloc(sizeof(directoryEntry)*deSize);
    strcpy(tempPath,path);
    info.index = -1;
    info.status = -1;
    LBAread(vcb, 1, 0);    
    // means the path we passed in is rrot directory. can just reutrn root directory from vcb
    if (strcmp(path, "/") == 0){
        //printf("location of info %d \nlocation of root directory %d\n",info.directory[0].location, vcb->rootDirectory.location);
        //printf("location of cwd %d\n", cwd[0].location);
    // could do an lbaread to make sure this is updated.
        info.directory[0] = vcb->rootDirectory;
        
        //printf("location of info %d \nlocation of root directory %d\n",info.directory[0].location, vcb->rootDirectory.location);
        info.status = 3;
        info.index = 0;
        return info;
    }
    // have to make a new directory entry so that we can return the de's
    //printf("49\n");
    directoryEntry* tempDirPointer;
    tempDirPointer = malloc(deSize*sizeof(directoryEntry));
    if (tempPath[0] == '/'){
        // absolute path
       // printf("4999999999999999\n");
        LBAread(tempDirPointer, calcBlocks(sizeof(directoryEntry)*deSize), vcb->rootDirectory.location);
        //tempDirPointer->flag='D';
        //printf("%s \tname of file \n %i \tlocation\n", dirPointer->name, dirPointer->location);

    }else{ 
        // relative path
       // printf("44444444444449\n");
        //printf("tempdirpointer values %s\n", tempDirPointer[0].name);
        // if it is relative we used cwd to organize where we are.
        memcpy(tempDirPointer, cwd, sizeof(directoryEntry)*deSize);
        // we really didnt nneed the memcpy since we could have just used cwd to get the location
        LBAread(tempDirPointer,calcBlocks(sizeof(directoryEntry)*deSize),tempDirPointer[0].location);
        //tempDirPointer->flag='D';
        //printf("pointer name == %s\n",tempDirPointer[0].name);
        
    }
   // printf("62\n");
    // gets us the head of the path that is passed in.
    char ** pathArray;
    pathArray = malloc(sizeof(path)*sizeof(char*));
    //printf("strtok_r is failing\n");
    deName = strtok_r(tempPath, "/", &savePointer);
    int count = 0;
    // if it is NULL then we stop parsing because we need n-1
    while (deName != NULL){
        pathArray[count] = deName;
        deName = strtok_r(NULL, "/", &savePointer);
        count++;
    }
       // printf("strtok_r is failing\n");
     

    // i is the number of things
    //printf("count %d\n", count);
// everytime we will now check the entire de to see if the desired name exists. 
    for (int i = 0; i < count; i++){
        
        for (int j = 0; j < deSize; j++){
            
            //printf("tempdirpointer name at all indexes %s", tempDirPointer[j].name);
            // found some existing directory entry with that name.
            //printf("NAME: %s\n",tempDirPointer[j].name);
            //LBAread(tempDirPointer,calcBlocks(sizeof(directoryEntry)*deSize),tempDirPointer[0].location);
            if (strcmp(pathArray[i], tempDirPointer[j].name) == 0){
                
                // if it does exist then we save the index and the status is 
                //0 which means we succeded
                info.index = j;
                info.status = 0;
                memcpy(info.directory,tempDirPointer,sizeof(directoryEntry)*deSize); 
                //info.directory[j].flag='D';

                //printf("BIGKATTTT\n");
                break;
            }
        }
        //printf("sdeoncd loop \n");
        // The DE does not exist
        if (info.index == -1){
            info.status = 1;
            //printf("getting here inside of 1 status\n");
            // if index is 1 that means we got all the way to the last index but 
            //the directory that we wanted does not exist;
            LBAread(info.directory,calcBlocks(sizeof(directoryEntry)*deSize),tempDirPointer[0].location);
            //printf("info directory information %s \n%d\n%d\n", info.directory[0].name, 
            //info.directory[0].location, info.directory[0].size);
            //printf("in this bishh\n");
            //memcpy(info.directory, cwd, sizeof(directoryEntry)*deSize);
            //printf("info directory information %s \n%d\n%d\n", info.directory[0].name,
            //  info.directory[0].location, info.directory[0].size);
            break;
        }

    }
    if (count == 0){
        // if count is 0 then we dont need to parse path and just use the cwd
        info.status = 1;
        memcpy(info.directory, cwd, sizeof(directoryEntry)*deSize);
        LBAread(info.directory,calcBlocks(sizeof(directoryEntry)*deSize),cwd[0].location);
        

    }
    // if(info.index==0){
    //     //LBAread(info.directory,calcBlocks(sizeof(directoryEntry)*deSize),info.directory[0].location);
    //     if(strcpy(info.directory[info.index].name,"")==0){

    //         info.index = 1;
    //     }
    // }
       // printf("89\n");
   // printf("info index %d\n", info.index);
    // if (info.index > 0){
    //     int retValue = LBAread(info.directory, calcBlocks(deSize*sizeof(directoryEntry)), info.directory[info.index].location);
    //     if (retValue != calcBlocks(deSize*sizeof(directoryEntry))){
    //         printf("LBAread failed\n");
    //         info.directory = NULL;
      
    //         return info;
    //     }
    // }

    // Keeping the entire path if found, as a global variable, so we can use it 
    // when we want to see the pathname later on.
    strcpy(entirePath, path);

   
    
    return info;
}
