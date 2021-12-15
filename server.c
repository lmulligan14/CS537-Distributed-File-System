#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "udp.h"
#include "mfs.h"
#include "requests.h"

struct sockaddr_in addr;
struct CheckpointRegion* cr;
int imageFD;

void updateCR(){
    int rc = lseek(imageFD, 0, SEEK_SET);
    rc = write(imageFD, cr, sizeof(CheckpointRegion));
    if (rc == -1)
        perror("Could not update CR\n");
    rc = lseek(imageFD, cr->logEnd, SEEK_SET);
}

int inodeInit(int type, int size)
{
    // Parse through Imap and find first free inode number
    int rc;
    int inodeNum;
    int inodeMapPtr;
    struct ImapPiece* mapPiece = malloc(sizeof(ImapPiece));
    int inodeMapOffset = 0;
    for (int i = 0; i < NUM_INODE_PIECES; i++)
    {
        inodeMapPtr = cr->imap[i];
        if (inodeMapPtr != -1)
        {
            rc = lseek(imageFD, inodeMapPtr, SEEK_SET);
            rc = read(imageFD, mapPiece, sizeof(ImapPiece));
            for (int j = 0; j < IMAP_PIECE_SIZE; j++)
            {
                if (mapPiece->inodes[j] == -1)
                {
                    inodeNum = (i * IMAP_PIECE_SIZE) + j;
                    inodeMapOffset = j;
                    break;
                }
            }
        }
        else
        {
            inodeNum = (i * IMAP_PIECE_SIZE);
            break;
        }
    }

    // Update Imap piece and add to file image
	int inodePtr = 0;
	if (inodeMapPtr == -1)
    {   /* New Imap piece */
        for (int i = 0; i < NUM_INODE_PIECES; i++)
        {
            mapPiece->inodes[i] = -1;
        }
		mapPiece->inodes[0] = inodePtr = cr->logEnd + sizeof(struct ImapPiece);
		rc = lseek(imageFD, cr->logEnd, SEEK_SET);
		rc = write(imageFD, mapPiece, sizeof(struct ImapPiece));
		cr->imap[inodeNum/16] = cr->logEnd;
		cr->logEnd += sizeof(struct ImapPiece);
	}
	else
    {   /* Existing Imap piece */
		rc = lseek(imageFD, inodeMapPtr, SEEK_SET);
		rc = read(imageFD, mapPiece, sizeof(struct ImapPiece));
		mapPiece->inodes[inodeMapOffset] = inodePtr = cr->logEnd + sizeof(struct ImapPiece);
		rc = lseek(imageFD, cr->logEnd, SEEK_SET);
		rc = write(imageFD, mapPiece, sizeof(struct ImapPiece));
		cr->imap[inodeNum/16] = cr->logEnd;
		cr->logEnd += sizeof(struct ImapPiece);
	}

    // Make and add Inode to file image
	struct INode* node = malloc(sizeof(INode)); // CRASHES HERE
	node->type = type;
	node->size = size;
	node->blocks[0] = inodePtr + sizeof(INode); // inodePtr -> end of Inode/start of data block
	rc = lseek(imageFD, inodePtr, SEEK_SET);
	rc = write(imageFD, node, sizeof(INode));
    if (rc == -1)
        perror("Error");
    cr->logEnd += sizeof(INode);

	updateCR();
    free(mapPiece);
    free(node);
	return inodeNum;
}

int dirInit(int pInum, char *name)
{
    int inodeNum = inodeInit(MFS_DIRECTORY, sizeof(DirBlock));
    DirBlock *block = malloc(sizeof(DirBlock));

    // Curr dir
    sprintf(block->entries[0].name, ".");
    block->entries[0].inum = inodeNum;

    // Parent dir
    sprintf(block->entries[1].name, "..");
    if (pInum == -1) // root dir (no parent)
        block->entries[1].inum = inodeNum;
    else
        block->entries[1].inum = pInum;
    
    // Set unused entries
    for (int i = 2; i < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); i++){
        block->entries[i].inum = -1;
    }

    // Write to file image
    int rc = lseek(imageFD, cr->logEnd, SEEK_SET);
    rc = write(imageFD, block, MFS_BLOCK_SIZE);
    if (rc == -1)
        perror("Error");
    cr->logEnd += MFS_BLOCK_SIZE;

    free(block);
    updateCR();
    return 0;
}

int fs_read(int inum, char* buffer, int block)
{
    int pieceNum = inum / 16;
    int inodePtr;

    // Check for valid args
    if (inum < 0 || inum > MAX_NUM_INODES)
        perror("Invalid Inode number\n");
    if (cr->imap[pieceNum] == -1)
        perror("Invalid Inode number\n");
    if (block < 0 || block > 13)
        perror("Invalid block number\n");

    // Find and allocate Imap piece
    ImapPiece* mapPiece = malloc(sizeof(ImapPiece));
    lseek(imageFD, cr->imap[pieceNum], SEEK_SET);
    read(imageFD, mapPiece, sizeof(ImapPiece));
    inodePtr = mapPiece->inodes[inum%16];
    if (inodePtr == -1)
        perror("Invalid Inode number\n");

    // Find and allocate inode
    INode* inode = malloc(sizeof(INode));
    lseek(imageFD, inodePtr, SEEK_SET);
    read(imageFD, inode, sizeof(INode));

    // Find and read block into buffer
    lseek(imageFD, inode->blocks[block], SEEK_SET);
    read(imageFD, buffer, MFS_BLOCK_SIZE);

    return 0;
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

    // Make a copy
    struct stat fs;
    if (fstat(imageFD, &fs) < 0)
    {
        perror("Could not open file\n");
	    return -1;
    }

    // Set up file (unfinished)
    cr = malloc(sizeof(CheckpointRegion));
    if (fs.st_size < sizeof(CheckpointRegion))
    {   /* New file */

	    // Set up checkpoint region
        cr->logEnd = sizeof(CheckpointRegion);
        for (int i = 0; i < NUM_INODE_PIECES; i++)
        {
            cr->imap[i] = -1;
        }
        updateCR();

	    // Make the root directory
        dirInit(-1, "root");

        printf("server:: new file image created\n");
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
        
        if (msg.request == INIT)
        {
            // Why would someone call INIT after already starting the program?
        }
        else if (msg.request == LOOKUP)
        {

        }
        else if (msg.request == STAT)
        {
            
        }
        else if (msg.request == WRITE)
        {
            printf("server:: read message [size:%d contents:(%s)]\n", rc, msg.block);

            packet res;
            char reply[1000];
            sprintf(reply, "goodbye world");
            memcpy(res.block, reply, MFS_BLOCK_SIZE);
            UDP_Write(sd, &addr, reply, sizeof(packet));
        }
        else if (msg.request == READ)
        {
            packet res;
            char buffer[MFS_BLOCK_SIZE];

            fs_read(msg.inum, buffer, msg.blocknum);
            memcpy(res.block, buffer, MFS_BLOCK_SIZE);
            UDP_Write(sd, &addr, (char*) &res, sizeof(packet));
        }
        else if (msg.request == CREAT)
        {
            
        }
        else if (msg.request == UNLINK)
        {
            
        }
        else if (msg.request == SHUTDOWN)
        {
            free(cr);
        }
        else
            perror("Invalid request\n");
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