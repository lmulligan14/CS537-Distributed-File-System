#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "requests.h"

struct sockaddr_in addr;

// server code
int main(int argc, char *argv[]) 
{
    int sd = UDP_Open(10000);
    printf("server:: waiting...\n");

    packet msg;
    while (1)
    {
        int rc = UDP_Read(sd, &addr, (char*) &msg, sizeof(packet));

        if (msg.request == WRITE)
        {
            printf("server:: read message [size:%d contents:(%s)]\n", rc, msg.block);

            packet rep;
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            memcpy(rep.block, reply, MFS_BLOCK_SIZE);
            UDP_Write(sd, &addr, reply, sizeof(packet));
        }
    }

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