#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "udp.h"
#include "mfs.h"
#include "requests.h"

struct sockaddr_in addr;
struct CheckpointRegion* cr;
int inodeNum;
int imageFD;


int inodeInit(int type, int size){
	int inodeMapPtr = cr->imap[inodeNum/16];
	int inodeMapOffset = inodeNum % 16;
	struct ImapPiece* mapPiece = malloc(sizeof(struct ImapPiece));
	int inodePtr = 0;
	int rc;
	if (inodeMapOffset == 0){
		mapPiece->inodes[0] = inodePtr = cr->logEnd + sizeof(struct ImapPiece);
		rc = lseek(imageFD, cr->logEnd, SEEK_SET);
		rc = write(imageFD, mapPiece, sizeof(struct ImapPiece));
		cr->imap[inodeNum/16] = cr->logEnd;
		cr->logEnd += sizeof(struct ImapPiece);
		rc = lseek(imageFD, 0, SEEK_SET);
		rc = write(imageFD, cr, sizeof(CheckpointRegion);
		rc = lseek(imageFD, cr->logEnd, SEEK_SET);
	}
	else{
		rc = lseek(imageFD, inodeMapPtr, SEEK_SET);
		rc = read(imageFD, mapPiece, sizeof(struct ImapPiece));
		mapPiece[inodeMapOffset] = inodePtr = cr->logEnd + sizeof(struct ImapPiece);
		rc = lseek(imageFD, cr->logEnd, SEEK_SET);
		rc = write(imageFD, mapPiece, sizeof(struct ImapPiece));
		cr->imap[inodeNum/16] = cr->logEnd;
		cr->logEnd += sizeof(struct ImapPiece);
		rc = lseek(imageFD, 0, SEEK_SET);
                rc = write(imageFD, cr, sizeof(CheckpointRegion);
                rc = lseek(imageFD, cr->logEnd, SEEK_SET);
	}
	inodeMap[inodeNum] = inodePtr;
	struct INode *node = malloc(sizeof(INode));
	node->type = type;
	node->size = size;
	if (node->type == MFS_DIRECTORY){
		node->blocks[0] = inodePtr + sizeof(INode);
	}
	rc = lseek(imageFD, inodePtr, SEEK_SET);
	rc = write(imageFD, node, sizeof(INode));
	cr->logEnd += sizeof(INode);
	rc = lseek(imageFD, 0, SEEK_SET);
        rc = write(imageFD, cr, sizeof(CheckpointRegion);
        rc = lseek(imageFD, cr->logEnd, SEEK_SET);
	inodeNum++;
	return inodeNum-1;
}

// Initialize file system (code from main)
int fs_init(int portNum, char* fileSystemImage)
{
    int sd = UDP_Open(portNum);
    UDP_FillSockAddr(&addr, "localhost", portNum);
    imageFD = open(fileSystemImage, O_CREAT | O_RDWR);

    // Check if file is valid
    if (imageFD < 0 || sd < 0)
    {
        perror("Could not open file\n");
	    return -1;
    }

    // make a copy
    struct stat fs;
    if (fstat(imageFD, &fs) < 0)
    {
        perror("Could not open file\n");
	    return -1;
    }

    // Set up file (unfinished)
     
    cr = (CheckpointRegion*) malloc(sizeof(CheckpointRegion));
    inodeNum = 0;

    if (fs.st_size < sizeof(CheckpointRegion))
    {   /* New file */
	    //set up checkpoint region
        cr->logEnd = sizeof(CheckpointRegion);
	int rc = lseek(imageFD, 0, SEEK_SET);
	rc = write(imageFD, cr, sizeof(CheckpointRegion));
	rc = lseek(imageFD, cr->logEnd, SEEK_SET);
	//make the root directory
	
    }
    else
    {   /* Existing file */

    }
    
    // Recieve requests
    printf("server:: waiting...\n");
    packet msg;
    while (1)
    {
        int rc = UDP_Read(sd, &addr, (char*) &msg, sizeof(packet));

        if (msg.request == WRITE)
        {
            printf("server:: read message [size:%d contents:(%s)]\n", rc, msg.block);

            packet rep;
            char reply[1000];
            sprintf(reply, "goodbye world");
            memcpy(rep.block, reply, MFS_BLOCK_SIZE);
            UDP_Write(sd, &addr, reply, sizeof(packet));
        }
    }

    return 0;
}

// server code
int main(int argc, char *argv[]) 
{
    if (argc != 3)
    {
        perror("Incorrect usage of server\n");
        return 0;
    }

    fs_init(atoi(argv[1]), argv[2]);

    return 0;
}


/* OLD SERVER
int main(int argc, char *argv[]) 
{
    int sd = UDP_Open(10000);
    assert(sd > -1);
    while (1) 
    {
        struct sockaddr_in addr;
        char message[BUFFER_SIZE];
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
        
        if (rc > 0) 
        {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
            printf("server:: reply\n");
        } 
    }
    return 0;
}*/
