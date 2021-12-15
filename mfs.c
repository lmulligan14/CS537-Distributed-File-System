#include "mfs.h"
#include "udp.h"
#include "requests.h"

struct sockaddr_in addr;
int sd;

int sendRequest(packet* msg, packet* res) 
{
	if (UDP_Write(sd, &addr, (char*) msg, sizeof(packet)) < 0)
        printf("client failed to send\n");
	UDP_Read(sd, &addr, (char*) res, sizeof(packet));
	return 0;
}

int MFS_Init(char *hostname, int port)
{
    sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addr, hostname, port);
    if (rc < 0) return rc;
    return 0;
}

int MFS_Lookup(int pinum, char *name)
{
    packet msg;
    packet res;

    msg.request = LOOKUP;
    msg.inum = pinum;
    strcpy(msg.name, name);
    sendRequest(&msg, &res);

    return 0;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
    packet msg;
    packet res;

    msg.request = STAT;
    msg.inum = inum;
    sendRequest(&msg, &res);

    memcpy(m, &res.stat, sizeof(MFS_Stat_t));
    m = &res.stat;

    return 0;
}

int MFS_Write(int inum, char *buffer, int block)
{
    packet msg;
    packet res;

    msg.request = WRITE;
    msg.inum = inum;
    msg.blocknum = block;
    memcpy(msg.block, buffer, sizeof(char[MFS_BLOCK_SIZE]));
    sendRequest(&msg, &res);

    return 0;
}

int MFS_Read(int inum, char *buffer, int block)
{
    packet msg;
    packet res;

    msg.request = READ;
    msg.inum = inum;
    msg.blocknum = block;
    sendRequest(&msg, &res);
    memcpy(buffer, res.block, sizeof(char[MFS_BLOCK_SIZE]));

    return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{
    packet msg;
    packet res;

    msg.request = CREAT;
    msg.inum = pinum;
    msg.type = type;
    strcpy(msg.name, name);
    sendRequest(&msg, &res);

    return 0;
}

int MFS_Unlink(int pinum, char *name)
{
    packet msg;
    packet res;

    msg.request = UNLINK;
    msg.inum = pinum;
    strcpy(msg.name, name);
    sendRequest(&msg, &res);

    return 0;
}

int MFS_Shutdown()
{
    packet msg;
    packet res;

    msg.request = SHUTDOWN;
    sendRequest(&msg, &res);

    return 0;
}