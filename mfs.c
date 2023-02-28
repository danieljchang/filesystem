/**************************************************************
* File: mfs.c
*
* Description: 
*   These are the functions of the driver.
**************************************************************/

#include "mfs.h"

#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#define MAXNUMBEROFDIRECTORIES 72

int deSize;
directoryEntry *cwd;
VCB *vcb;
char entirePath[255];

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode){
    
    pathInfo info; 
    info.directory = malloc(sizeof(directoryEntry)*deSize);
    
    info = parsePath(pathname);
    //printf("\nstatus: %d\n",info.status);
    //LBAread(info.directory,calcBlocks(sizeof(directoryEntry)*deSize),info.directory[0].location);
    //printf("info status in mkdir %d\n", info.status);
    // we should be able to create a directory with the same name as the file. this is making a dir not a file
    if (info.status == 1){ // 1 if it does not exist meaning we can create the directory otherwise print error message   
        // The new directory that we are creating is a file or it is a nonexisting directory;
        // int retValue = directoryInit(info.directory, pathname);
        //printf("return value %d\n", info.directory[0].location);
        // if (retValue == -1){
        //     printf("failed to init directory");
        //     return -1;
        // }
        directoryEntry *deArray;
        deArray = malloc(sizeof(directoryEntry)*deSize);
        // creating a de array so that we can hold out new values inside of mkdir
        // we use this since we LBAread, and get cwd
		int blocksNeeded = calcBlocks(sizeof(directoryEntry)*deSize);
        directoryEntry *de;
        de = malloc(sizeof(directoryEntry)*deSize);
        int check = LBAread(de, blocksNeeded, info.directory[0].location);
        //printf("%s\n", de[2].name);
        if (check != blocksNeeded){
            //printf("failed to LBAread in mkdir\n");
            return -1;
        }
        // index starts at 2 since 0 and 1 are . and ..
        int index = 2;
		for (index; index < deSize; index++){
            // if it is empty then we are just writing
            // creating a new directory
			if (de[index].flag == 'E'){
                //printf("index %d\n", index);
				strcpy(de[index].name, pathname);
                //printf("%d extern dezsize\n", deSize);
                
				de[index].size = (sizeof(directoryEntry)*deSize);
				de[index].flag = 'D';
				//parent[i].location = (parent == NULL)?dir.location:parent[0].location;
				de[index].location = info.directory[0].location;
				//printf("%d new file\n", de[1].location);
                // double check position
				LBAwrite(de, blocksNeeded, info.directory[0].location);
                //BELOW LINE NEEDS TO BE MOVED DONT FORGET
                //LBAread(cwd,blocksNeeded,de[0].location);
				break;
			    
		    }
	    }
        // renew all the disk stuff
		writeFreeSpaceMap(blocksNeeded);
        return 0;
    }// }else if(strcmp(info.directory[info.index].name,"")==0){


    // }
    //printf("File already exists\n");
    // if (newPath.index > MAXNUMBEROFDIRECTORIES){
    //     // means that we do not have enough space in the directory.
    //     // we should make it dynamic and realloca space/increase space
    // }
    
    return 0;
}
int fs_rmdir(const char *pathname){
	pathInfo info;
    
    info.directory = malloc(sizeof(directoryEntry)*deSize);
    
    info = parsePath(pathname);

    printf("INSIDE OF REMOVE: %s\n",info.directory[info.index].name);
	
	// directory must be empty to be removed
    strcpy(info.directory[info.index].name,"");
    
    info.directory[info.index].flag = 'E';

	while(strcmp(info.directory[info.index].name, "") != 0){ // if dir is not empty, return error
		return 1;
    }
    // updating the de array because we deleted something
    //LBAwrite(&info.directory[info.index],calcBlocks(sizeof(directoryEntry)*72),info.directory[info.index].location);
    int x = LBAwrite(info.directory,calcBlocks(sizeof(directoryEntry)*72),info.directory[0].location);
    //memcpy(cwd,info.directory,sizeof(directoryEntry)*72);
    //info.index=-1;
    //info.status=1;
    //free(info.directory);

	return 0;// 0 = success
    free(info.directory);
}


// Directory iteration functions
//fsdir is a file descriptor for a directory 
//parse path, create fdDir object from DE object,return pointer to object
fdDir * fs_opendir(const char *pathname){

    // setting up our mallocs for data that we will use
    pathInfo info;
    fdDir* descriptor = malloc(sizeof(fdDir));

    info.directory = malloc(sizeof(directoryEntry)*deSize);
    directoryEntry * de = malloc(sizeof(directoryEntry)*deSize);
    //printf("parsepath\n");
    // using parsepath we get the cwd or root
    info = parsePath(pathname);
    //printf("opening status %d\n info location is %d\n", info.status, info.directory[0].location);
    //printf("should be 0 index: %d",info.index);
    // couldnt open a directory since it does not exist
    if(info.status == -1){
        printf("not found");
        return NULL;
    }
    // if directory exists
           // printf("this location 154 not : %i\n",info.directory[info.index].location);


    
    if (info.status == 3){ // this means that we found the parent and we succeeded
        if(info.directory[0].flag == 'D'){
        //fdDir descriptor;
            //printf("vcb blocksize %i\n", vcb->blockSize);
            //printf("this location 154 not : %i\n",info.directory[info.index].location);

            int blocks = calcBlocks(MAXNUMBEROFDIRECTORIES*sizeof(directoryEntry));
            //int blocks = 69;
            //char buf[vcb->blockSize*blocks];
            //printf("\n%i\n", sizeof(directoryEntry));
            //printf("\nBLOCK CoUNT IN OPENDIR %i\ninfo index %d\n", blocks, info.index);
            
            //printf("this location 154 not : %i\n",info.directory[info.index].location);
            //printf("should be 0 index: %d\n",info.index);
            // we should be error checking here but it is fine
            int retValue = LBAread(de, blocks, info.directory[0].location);
            //printf("BLOCKS: %d\n",blocks);
            //printf("de name: %s\n",de[0].name);

            // updating all of the fdir items info
            descriptor->d_reclen = sizeof(fdDir);
            //descriptor->d_reclen = de->size;
            descriptor->directoryStartLocation = info.directory[info.index].location;
            descriptor->dirEntryPosition = info.index;
            descriptor->index = 0;
            // now that we updated it we memcpy our directory entry array into the 
            // descriptors array so that we have cwd for the rest
            descriptor->directory = malloc(MAXNUMBEROFDIRECTORIES*sizeof(directoryEntry));
            memcpy(descriptor->directory,de,MAXNUMBEROFDIRECTORIES*sizeof(directoryEntry));
            //printf("getting to the end of opendir\n");
        }else {
            printf("status is not 3");
            return NULL;
        }   
        
        // printf("directory entry 0 %s\n", descriptor->directory[0].name);
        // printf("directory entry 1 %s\n", descriptor->directory[1].name);
        // printf("directory entry 2 %s\n", descriptor->directory[2].name);
        // printf("directory entry 3 %s\n", descriptor->directory[3].name);

    }
    
    
    //printf("this is freed?\n");
    // free(opening.directory);
    //     printf("this is freed?\n");

    // opening.directory = NULL;
    //printf("_______________desciptor values______________ \n\n %i index\n", descriptor->index);
    //printf("hello\n");
    return descriptor;
}
//return the fs_diriteminfo cooresponding to the fdDir passed in

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    // readdir works amazingly
    struct fs_diriteminfo *itemInfo = malloc(sizeof(struct fs_diriteminfo));
    // The pointer to itself
    int entry=0;
    while(entry == 0){
        // we found a file
        // we find the entry of where we want to be. If it is empty then we dont reade it,
        // if we find an entry then we can return that entry, but we must allocate the
        // location properly so that we do not restart but start where we left off
        if (strcmp(dirp->directory[dirp->dirEntryPosition].name, "") != 0){
            strncpy(itemInfo->d_name , dirp->directory[dirp->dirEntryPosition].name,
                sizeof(itemInfo->d_name));
            itemInfo->fileType = (dirp->directory[dirp->dirEntryPosition].flag == 'D') ? FT_DIRECTORY : FT_REGFILE;
            

            itemInfo->d_reclen = dirp->directory[dirp->dirEntryPosition].size;
            //itemInfo->d_reclen = dirp->d_reclen;
            entry = 1;
        }
        if (dirp->dirEntryPosition > MAXNUMBEROFDIRECTORIES-1){
            return NULL;
        }
        // holds onto this new index in the array
        dirp->dirEntryPosition++;
        
    }
    return itemInfo;

}

//should be as simple as freeing memory?
int fs_closedir(fdDir *dirp){
    // printf("invalid free??????"); 
    if(dirp !=NULL){
     //free(dirp->directory);
     //dirp->directory = NULL;
     free(dirp);
     dirp = NULL;
     return 0;
    }else{
        printf("\n CANNOT CLOSE DIR\n");
        return 1;
    }

}

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size){
    //printf("%s\n", pathname);
    //printf("sizeof %i\n", sizeof(pathname));
    if ((pathname!= NULL) && pathname[0] == '\0'){
        //printf("being set\n");
        strcpy(entirePath, vcb->pathName);
    }else{
        //printf("compared\n");
        // might have to call a parsepath
        // if it exists return good value 3, else cant find it. and return global.entirepath
        // which should be the previous cwd
        strcpy(entirePath, pathname);

        // strncpy(global.entirePath, pathname);
    }
    //printf("size %i", size);
    //printf("pathname %s\n", pathname);
    //printf("global entire path %s\n", global.entirePath);
    
    return entirePath;



} // get current working directory
int fs_setcwd(char *pathname){
    //
    pathInfo info;
    directoryEntry *de = malloc (sizeof(directoryEntry)*deSize);
    char *path = malloc(sizeof(pathname)+1);
    strcpy(path, pathname);
    info.directory = malloc(sizeof(directoryEntry)*MAXNUMBEROFDIRECTORIES);
    info = parsePath(path);
    char ** pathArray = malloc(255);
    //printf("\n---Set Current Working Directory---\n");
	// Parse Path
    char* save;
    char* last;
    char* temp = malloc(64);
    int index = 0;
    // if we are at root, or if pathname is nothing then we can jsut read from the root directory
    if (strcmp(pathname, "/") == 0 || strcmp(pathname, "") == 0){
        // means that we are going to the root
        LBAread(cwd, calcBlocks(sizeof(directoryEntry)*deSize), vcb->rootDirectory.location);
    }else{
        // otherwise we need to find the new location of this directory to change our cwd
        pathInfo info;
        info.directory = malloc(sizeof(directoryEntry)*deSize);
        info = parsePath(pathname);
        // if the path does not exist then we failed
        if (info.status == -1){
            printf("Path is incorrect");
            return -1;
        }
        // we are trying to find the last value of the pathname.
        last = strtok_r(path, "/", &save);
        while (last != NULL){
            strcpy(temp, last);
            last = strtok_r(NULL,"/", &save);
        }
        // once we get the last value we also try to get the index of this path inside the cwd
        for (int index; index < deSize; index++){
            if (strcmp(last, info.directory[index].name) == 0){
                break;
            }
        }
        // we should bet setting cwd to this index of the directory entry so that
        // we can find the new cwd
        //printf("inside cd index: %s \n flag: %c", last, info.directory[index].flag);
        if (info.directory[index].flag != 'D'){
            printf("cannot find a directory\n");
            return -1;
        }
        // updating our de with the accurate location
        LBAread(info.directory, calcBlocks(sizeof(directoryEntry)*deSize), info.directory[index].location);
    }


    // THIS FINDS THE ENTIRE PATH SETTING THE CW
	
    int count = 0;
     // read the current working directory into memory
     // ressetting de
    LBAread(de, calcBlocks(deSize*sizeof(directoryEntry)), de[0].location);
    // . is eqqual to the .. so that means we are in root
    if (de[0].location == de[1].location){
        strcpy(entirePath, "/");
        return 0;
    }
    // if the direcotry is a flag then we are doing something right
    // we want it to be a directory since cd can only change to a directory
	if(info.directory[info.index].flag == 'D'){
        int search = de[0].location;
        int size = 0;
		memcpy(cwd, info.directory, sizeof(directoryEntry)*MAXNUMBEROFDIRECTORIES);
        // if these are equal then we are at root building the path backwards
        while (de[0].location != de[1].location){
            // iterates through the path backwards. and setting these values into 
            // our array to build later
            LBAread(de, sizeof(directoryEntry)*deSize, de[1].location);
            for (int i = 2; i < deSize; i++){
                if (de[i].location == search){
                    int size_of_name = strlen(de[i].name) + 1;
                    size += size_of_name;
                    pathArray[count] = malloc(size);
                    strcpy(pathArray[count], de[i].name);
                    count ++;
                    break;
                }
            }
    // set a new seach location for directory for the new parent directory
            search = de[0].location;
        }
        // building the strcat the pathname for getcwd
        strcpy(path, "/");
    for (int i = count - 1; i >= 0; i--){
        strcat(path, pathArray[i]);
        if (i > 0){
            strcat(path, "/");
        }
    }
    strcpy(entirePath, path);
    return 0;
    } else {
        printf("No existing directory\n");
		return -1;
	}

}   //linux chdir
int fs_isFile(char * filename){

    pathInfo dirfil = parsePath(filename);
    // checking our parsepath flag
    if(dirfil.directory->flag == 'F'){
        return 1;
    }
    return 0;

        //global->cwd[i];
}
	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname){
    // printf("THIS IS THE PATH IN MKDIR: %s\n",pathname);
    pathInfo dirfil = parsePath(pathname);
    //LBAread(dirfil.directory,calcBlocks(malloc(sizeof(directoryEntry)*72)),dirfil.directory[0].location);
    // checking our parsepath flag

    if(dirfil.directory->flag == 'D'){
        return 1;
    }
    return 0;
}		//return 1 if directory, 0 otherwise


// returns -1 if fail, 1 if success
int fs_delete(char* filename){
    int index;
    // finding the file and deleteing it did not do since bio was not working
    for (index = strlen(filename); index >= 0; index--){
        if (filename[index] == '/'){
            break;
        }
    }
    // get the substring this does not work properly
    char *trueName = &filename[index+1];
    // truename is the filename so we can parse it.
    //global->entirePath
    
    pathInfo temp = parsePath(filename);
    if(temp.directory!=NULL){

        // if 1 it is true if 0 false
        int i = fs_isFile(filename);

        int fileLocation;
        // we found the location of the file and delete it given the directory
        for (fileLocation = 2 ; fileLocation < temp.directory->maxNumEntries ; fileLocation++){
            if(strcmp(temp.directory->name, trueName) == 0){
                //found in directory
                break;
            }
        }
        if(i!=1){
            return -1;
        }
        //TODO DELETE FILE
        // Update the free space, 
        // increase the freeblockcount.... 
        vcb->freeBlockCount += temp.directory[fileLocation].maxNumEntries;
        // remove flag in freespacemap 
        // go into parent(array) get rid of that directory entry.
        // set the name to to null or empty string
        
        return 1;
    }

}	//removes a file

// showing off stats
int fs_stat(const char *path, struct fs_stat *buf){

    pathInfo temp = parsePath(path);
    buf->st_accesstime = time(0);
    buf->st_createtime = time(0);
    buf->st_modtime = time(0);
    buf->st_size = temp.directory->size;
    buf->st_blocks = vcb->blockCount;
    return 0;
}
