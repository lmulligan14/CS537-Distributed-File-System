#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (1000)

// client code (testing)
int main(int argc, char *argv[]) 
{
    if (argc != 3){
	    printf("Incorrect usage");
	    return 0;
    }
    char *hostName = strdup(argv[1]);
    int portNum = atoi(argv[2]);
	MFS_Init("hostName", portNum);
    char message[MFS_BLOCK_SIZE];
    sprintf(message, "hello world");

    MFS_Write(0, message, 0);

    return 0;
}


/* OLD CLIENT
int main(int argc, char *argv[]) 
{
    struct sockaddr_in addrSnd, addrRcv;

    int sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, "localhost", 10000);

    char message[BUFFER_SIZE];
    sprintf(message, "hello world");

    printf("client:: send message [%s]\n", message);
    rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }

    printf("client:: wait for reply...\n");
    rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    return 0;
}*/
