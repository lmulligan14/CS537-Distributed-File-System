#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include <ctype.h>

#define BUFFER_SIZE (4096)

char *sHost = NULL;
int sPort = -1;
int online = 0;

int MSF_Init(char *hostname, int port){
    host = strdup(hostname);
    port = port;
    online = 1;
    return 0;
}
int MFS_Lookup(int pinum, char *name){
    if (online != 1){
        return -1;
    }

    
    return -1;
}
int MFS_Stat(int inum, MFS_Stat_t *m){
    return -1;
}
int MFS_Write(int inum, char *buffer, int block){
    return -1;
}
int MFS_Read(int inum, char *buffer, int block){
    return -1;
}
int MFS_Creat(int pinum, int type, char *name){
    return -1;
}
int MFS_Unlink(int pinum, char *name){
    return -1;
}
int MFS_Shutdown(){

}