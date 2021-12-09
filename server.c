#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "udp.h"
#include "mfs.h"
#include "requests.h"

struct sockaddr_in addr;
struct CheckpointRegion* cr;

// Initialize file system (code from main)
int fs_init(int portNum, char* fileSystemImage)
{
    int sd = UDP_Open(portNum);
    UDP_FillSockAddr(&addr, "localhost", portNum);
    int fileSystem = open(fileSystemImage, O_CREAT | O_RDWR);

    // Check if file is valid
    if (fileSystem < 0 || sd < 0)
    {
        perror("Could not open file\n");
	    return -1;
    }

    // Check if file is valid
    struct stat fs;
    if (fstat(fileSystem, &fs) < 0)
    {
        perror("Could not open file\n");
	    return -1;
    }

    // Set up file (unfinished)
    cr = (CheckpointRegion*) malloc(sizeof(CheckpointRegion));
    if (fs.st_size < sizeof(CheckpointRegion))
    {   /* New file */

        cr->logEnd = sizeof(CheckpointRegion);
        for (int i = 0; i < NUM_INODE_PIECES; i++)
        {
            cr->imap[i] = -1;
        }
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
