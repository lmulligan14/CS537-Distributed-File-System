#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/tcp.h>
#include <netinet/in.h>

#include "mfs.h"

int
main(int argc, char *argv[])
{
	//char name[5];
    MFS_Init("localhost", atoi(argv[1]));
    //MFS_Creat(0, MFS_DIRECTORY, "test");
	char buffer[4096*4];
    int inum = MFS_Read(0, buffer, 0);
	printf("%s\n", buffer);
	printf("iNum %d\n", inum);
	//MFS_Write(inum, "Hello", 0);

	/*int index;
	for(index = 0; index < 1790; index++) {
		sprintf(name, "%d", index);
	    MFS_Creat(inum, MFS_REGULAR_FILE, name);
	}
	for(index = 0; index < 1790; index++) {
		sprintf(name, "%d", index);
	    int lookup = MFS_Lookup(inum, name);
		printf("Lookup #: %d\n", lookup);
		if(lookup < 0) {
			exit(1);
		}
	}*/


	//MFS_Unlink(0, "test");
	MFS_Read(inum, buffer, 0);
	printf("BUFFER %s\n", buffer);
	//inum = MFS_Lookup(0, "test");
	//printf("Lookup after Unlink %d\n", inum);
    MFS_Shutdown();

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
