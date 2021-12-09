#include "mfs.h"
#include "udp.h"
#include "requests.h"

struct sockaddr_in addr;
int sd;

int command(packet* msg, packet* res) 
{
	if (UDP_Write(sd, &addr, (char*) msg, sizeof(packet)) < 0)
        printf("client failed to send\n");
	UDP_Read(sd, &addr, (char*) res, sizeof(packet));
	return 0;
}

int MFS_Init(char *hostname, int port)
{
    sd = UDP_Open(20000);
    UDP_FillSockAddr(&addr, hostname, port);
    return 0;
}

int MFS_Lookup(int pinum, char *name)
{
    return 0;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
    return 0;
}

int MFS_Write(int inum, char *buffer, int block)
{
    packet msg;
    packet res;
    msg.request = WRITE;
    memcpy(msg.block, buffer, sizeof(char[MFS_BLOCK_SIZE]));
    command(&msg, &res);
    return 0;
}

int MFS_Read(int inum, char *buffer, int block)
{
    return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{
    return 0;
}

int MFS_Unlink(int pinum, char *name)
{
    return 0;
}

int MFS_Shutdown()
{
    return 0;
}