#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "udp.h"
#include "structures.h"
#include "mfs.h"

int sendRequest( Packet *msg, Packet *rep, char *hostname, int port)
{

    int sd = UDP_Open(0);
    if(sd < -1)
    {
        perror("sendRequest: failed to open socket.");
        return -1;
    }

    struct sockaddr_in addr, addr2;
    int rc = UDP_FillSockAddr(&addr, hostname, port);
    if(rc < 0)
    {
        perror("upd_send: failed to find host");
        return -1;
    }

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec=3;
    tv.tv_usec=0;

    int trial_limit = 5;	/* trial = 5 */
    do {
        FD_ZERO(&rfds);
        FD_SET(sd,&rfds);
        UDP_Write(sd, &addr, (char*)msg, sizeof(Packet));
        if(select(sd+1, &rfds, NULL, NULL, &tv))
        {
            rc = UDP_Read(sd, &addr2, (char*)rep, sizeof(Packet));
            if(rc > 0)
            {
                UDP_Close(sd);
                return 0;
            }
        }else {
            trial_limit --;
        }
    }while(1);
}

/******************** MFS start ********************/

char* server_host = NULL;
int server_port = 3000;
int online = 0;



int MFS_Init(char *hostname, int port) {
	server_host = strdup(hostname); /* gw: tbc dubious  */
	server_port = port;
	online = 1;
	return 0;
}


int MFS_Lookup(int pinum, char *name){
	if(!online)
		return -1;
	
	if(strlen(name) > 60 || name == NULL)
		return -1;

	Packet msg;
	Packet rep;

	msg.inum = pinum;
	msg.request = REQ_LOOKUP;

	strcpy((char*)&(msg.name), name);

	if(sendRequest
( &msg, &rep, server_host, server_port) < 0)
	  return -1;
	else
	  return rep.inum;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
	if(!online)
		return -1;

	Packet msg;
	msg.inum = inum;
	msg.request = REQ_STAT;


	Packet rep;
	if(sendRequest
( &msg, &rep, server_host, server_port) < 0)
		return -1;
	m->type = rep.stat.type;
	m->size = rep.stat.size;

	return 0;
}

int MFS_Write(int inum, char *buffer, int block){
	int i = 0;
	if(!online)
		return -1;
	
	Packet msg;
	Packet rep;

	msg.inum = inum;

	for(i=0; i<MFS_BLOCK_SIZE; i++)
	  msg.buffer[i]=buffer[i];

	msg.block = block;
	msg.request = REQ_WRITE;
	
	if(sendRequest
( &msg, &rep, server_host, server_port) < 0)
		return -1;
	
	return rep.inum;
}

int MFS_Read(int inum, char *buffer, int block){
  int i = 0;
  if(!online)
    return -1;
	
  Packet msg;


  msg.inum = inum;
  msg.block = block;
  msg.request = REQ_READ;

  Packet rep;	
  if(sendRequest
( &msg, &rep, server_host, server_port) < 0)
    return -1;

  if(rep.inum > -1) {
    for(i=0; i<MFS_BLOCK_SIZE; i++)
      buffer[i]=rep.buffer[i];
  }

	
  return rep.inum;
}

int MFS_Creat(int pinum, int type, char *name){
	if(!online)
		return -1;
	
	//	if(checkName(name) < 0)
	if(strlen(name) > 60 || name == NULL)
		return -1;

	Packet msg;

	strcpy(msg.name, name);
	msg.inum = pinum;
	msg.type = type;
	msg.request = REQ_CREAT;

	Packet rep;	
	if(sendRequest
( &msg, &rep, server_host, server_port) < 0)
		return -1;

	return rep.inum;
}

int MFS_Unlink(int pinum, char *name){
	if(!online)
		return -1;
	
	if(strlen(name) > 60 || name == NULL)
		return -1;
	
	Packet msg;

	msg.inum = pinum;
	msg.request = REQ_UNLINK;
	strcpy(msg.name, name);

	Packet rep;	
	if(sendRequest
( &msg, &rep,server_host, server_port ) < 0)
		return -1;

	return rep.inum;
}

int MFS_Shutdown(){
  Packet msg;
	msg.request = REQ_SHUTDOWN;

	Packet rep;
	if(sendRequest
( &msg, &rep,server_host, server_port) < 0)
		return -1;
	
	return 0;
}