#include <stdio.h>
#include "udp.h"

#define BUFFER_SIZE (4096)

// server code
int main(int argc, char *argv[]) {
    if (argc != 3){
        printf("Incorrect number of arguments");
        exit(1);
    }
    int portNum = atoi(argv[1]);
    char *image = argv[2];
    int sd = UDP_Open(portNum);
    assert(sd > -1);
    int fd = open(image, O_RDWR|O_CREAT, S_IRWXU);
    assert(fd > -1);


    while (1) {
	    struct sockaddr_in addr;
	    char message[BUFFER_SIZE];
	    printf("server:: waiting...\n");
	    int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
	    printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
	    if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
	        printf("server:: reply\n");
	    }
    }
    return 0; 
}
    


 
