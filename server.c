#include <stdio.h>
#include <sys/stat.h>
#include "udp.h"
#include "mfs.h"
#include "structures.h"

int sd, rc, fsImage, currInodeNum;
struct sockaddr_in *addr;
CheckpointRegion *cr;
int *iNodeMap;

int updateCR() 
{
	int rc = lseek(fsImage, 0, SEEK_SET);
	if(rc < 0) 
		return -1;

	rc = write(fsImage, cr, sizeof(CheckpointRegion));
	if(rc < 0)
		return -1;

	rc = lseek(fsImage, cr->logEnd, SEEK_SET);
	if(rc < 0)
		return -1;
	
	return 0;
}

INode* FindINode(int iNodeNum) 
{
	INode *iNode = malloc(sizeof(INode));
	int iNodePtr = iNodeMap[iNodeNum];

	int rc = lseek(fsImage, iNodePtr, SEEK_SET);
	if(rc < 0)
		return NULL;
	
	rc = read(fsImage, iNode, sizeof(INode));
	if(rc < 0)
		return NULL;

	return iNode;
}

int FileWrite(int iNum, char *buffer, int block) 
{
	INode *iNode = FindINode(iNum);
	if(iNode != NULL && iNode->type == MFS_REGULAR_FILE) 
	{
		int iNodePtr = 0;
		int iNodeMapPtr = cr->iNodeMaps[iNum/16];
		int iNodeMapOffset = iNum % 16;
		int *iNodeMapPiece = malloc(16*sizeof(int));

		rc = lseek(fsImage, iNodeMapPtr, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = read(fsImage, iNodeMapPiece, 16*sizeof(int));
		if(rc < 0) 
			return -1;
			
		iNodeMapPiece[iNodeMapOffset] = iNodePtr = cr->logEnd + 16*sizeof(int);
		rc = lseek(fsImage, cr->logEnd, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = write(fsImage, iNodeMapPiece, 16*sizeof(int));
		if(rc < 0) 
			return -1;
		
		cr->iNodeMaps[iNum/16] = cr->logEnd;
		cr->logEnd += 16*sizeof(int);
		rc = updateCR();
		if(rc < 0) 
			return -1;
		
		iNodeMap[iNum] = iNodePtr;

		int i; 
		for(i = MFS_BLOCK_SIZE - 1; i >= 0; i--) 
		{
			if(buffer[i] != '\0') 
			{
				iNode->size = (MFS_BLOCK_SIZE * block) + i + 1;
				break;
			}
		}

		iNode->blocks[block] = cr->logEnd + sizeof(INode);
		int rc = lseek(fsImage, iNodePtr, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = write(fsImage, iNode, sizeof(INode));
		if(rc < 0) 
			return -1;
		
		cr->logEnd += sizeof(INode);
		rc = updateCR();
		if(rc < 0) 
			return -1;
		
		rc = lseek(fsImage, cr->logEnd, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = write(fsImage, buffer, MFS_BLOCK_SIZE);
		if(rc < 0) 
			return -1;
		
		cr->logEnd += MFS_BLOCK_SIZE;
		rc = updateCR();
		if(rc < 0) 
			return -1;
	}
	else 
		return -1;

	return 0;
}

int ParentDirGet(int pINum, char *cName) 
{
	INode *parentINode = FindINode(pINum);
	if(parentINode != NULL && parentINode->type == MFS_DIRECTORY) 
	{
		int blockIndex;		
		MFS_DirEnt_t *parentBlock = malloc(MFS_BLOCK_SIZE);
		for(blockIndex = 0; blockIndex < 14; blockIndex++) 
		{
			int parentBlockPointer = parentINode->blocks[blockIndex];
			if(parentBlockPointer > 0) 
			{
				int rc = lseek(fsImage, parentBlockPointer, SEEK_SET);
				if(rc < 0) 
					return -1;
				
				rc = read(fsImage, parentBlock, MFS_BLOCK_SIZE);
				if(rc < 0) 
					return -1;
				
				int dirEntIndex;			
				for(dirEntIndex = 0; dirEntIndex < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t); dirEntIndex++) 
				{
					MFS_DirEnt_t parentDirEnt = parentBlock[dirEntIndex];
					if(!strcmp(parentDirEnt.name, cName)) 
						return parentDirEnt.iNum;
				}
			}
		}	
	}

	return -1;
}

int ParentDirWrite(int pINum, int cINum, char *cName) 
{
	INode *parentINode = FindINode(pINum);
	if(parentINode != NULL && parentINode->type == MFS_DIRECTORY) 
	{
		int blockIndex;		
		MFS_DirEnt_t *parentBlock = malloc(MFS_BLOCK_SIZE);
		MFS_DirEnt_t *emptyBlock = malloc(MFS_BLOCK_SIZE);

		for(blockIndex = 0; blockIndex < 14; blockIndex++) 
		{
			int parentBlockPointer = parentINode->blocks[blockIndex];
			if(parentBlockPointer == 0) 
			{
				memcpy(parentBlock, emptyBlock, MFS_BLOCK_SIZE);
				int i;
				for (i = 0; i < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t); i++) 
				{
					parentBlock[i].iNum = -1;
				}
			} 
			else 
			{
				rc = lseek(fsImage, parentBlockPointer, SEEK_SET);
				if(rc < 0) 
					return -1;
				
				rc = read(fsImage, parentBlock, MFS_BLOCK_SIZE);
				if(rc < 0) 
					return -1;
				
			}
			int dirEntIndex;			
			for(dirEntIndex = 0; dirEntIndex < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t); dirEntIndex++) 
			{
				if((!strcmp(parentBlock[dirEntIndex].name, cName) && parentBlock[dirEntIndex].iNum >= 0) || (parentBlock[dirEntIndex].iNum == -1 && cINum >= 0)) 
				{
					int iNodePtr = 0;
					int iNodeMapPtr = cr->iNodeMaps[pINum/16];
					int iNodeMapOffset = pINum % 16;
					int *iNodeMapPiece = malloc(16*sizeof(int));

					rc = lseek(fsImage, iNodeMapPtr, SEEK_SET);
					if(rc < 0) 
						return -1;
					
					rc = read(fsImage, iNodeMapPiece, 16*sizeof(int));
					if(rc < 0) 
						return -1;
					
					iNodeMapPiece[iNodeMapOffset] = iNodePtr = cr->logEnd + 16*sizeof(int);
					rc = lseek(fsImage, cr->logEnd, SEEK_SET);
					if(rc < 0) 
						return -1;
					
					rc = write(fsImage, iNodeMapPiece, 16*sizeof(int));
					if(rc < 0) 
						return -1;
					
					cr->iNodeMaps[pINum/16] = cr->logEnd;
					cr->logEnd += 16*sizeof(int);
					rc = updateCR();
					if(rc < 0) 
						return -1;
					
					iNodeMap[pINum] = iNodePtr;
					parentINode->blocks[blockIndex] = cr->logEnd + sizeof(INode);

					rc = lseek(fsImage, iNodePtr, SEEK_SET);
					if(rc < 0) 
						return -1;
					
					rc = write(fsImage, parentINode, sizeof(INode));
					if(rc < 0) 
						return -1;
					
					cr->logEnd += sizeof(INode);
					rc = updateCR();
					if(rc < 0) 
						return -1;
					
					parentBlock[dirEntIndex].iNum = cINum;
					sprintf(parentBlock[dirEntIndex].name, cName, sizeof(cName));
					rc = lseek(fsImage, cr->logEnd, SEEK_SET);
					if(rc < 0) 
						return -1;
					
					rc = write(fsImage, parentBlock, MFS_BLOCK_SIZE);
					if(rc < 0) 
						return -1;
					
					cr->logEnd += MFS_BLOCK_SIZE;
					rc = updateCR();
					if(rc < 0) 
						return -1;
					
					return 0;
				}
				else if(blockIndex == 13 && dirEntIndex == (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t))-1) 
				{
					return -1;
				}
			}
			
		}	
	}
	else 
		return -1;

	return 0;
}

int DirIsEmpty(int iNum) {
	INode *iNode = FindINode(iNum);
	if(iNode != NULL && iNode->type == MFS_DIRECTORY) 
	{
		int blockIndex;		
		MFS_DirEnt_t *block = malloc(MFS_BLOCK_SIZE);
		for(blockIndex = 0; blockIndex < 14; blockIndex++) 
		{
			int blockPointer = iNode->blocks[blockIndex];
			if(blockPointer > 0) 
			{
				int rc = lseek(fsImage, blockPointer, SEEK_SET);
				if(rc < 0) 
					return -1;
				
				rc = read(fsImage, block, MFS_BLOCK_SIZE);
				if(rc < 0) 
					return -1;
				
				int dirEntIndex;			
				for(dirEntIndex = 0; dirEntIndex < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t); dirEntIndex++) 
				{
					MFS_DirEnt_t dirEnt = block[dirEntIndex];
					if(!(dirEnt.iNum < 0 || !strcmp(dirEnt.name, ".") || !strcmp(dirEnt.name, ".."))) 
						return -1;
				}
			}
		}	
	}

	return 0;
}

int InodeInit(int type, int size) 
{
	int iNodeMapPtr = cr->iNodeMaps[currInodeNum/16];
	int iNodeMapOffset = currInodeNum % 16;
	int *iNodeMapPiece = malloc(16*sizeof(int));
	int iNodePtr = 0;
	int rc;
	if(iNodeMapOffset == 0) 
	{
		iNodeMapPiece[0] = iNodePtr = cr->logEnd + 16*sizeof(int);
		rc = lseek(fsImage, cr->logEnd, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = write(fsImage, iNodeMapPiece, 16*sizeof(int));
		if(rc < 0) 
			return -1;
		
		cr->iNodeMaps[currInodeNum/16] = cr->logEnd;
		cr->logEnd += 16*sizeof(int);
		rc = updateCR();
		if(rc < 0) 
			return -1;
		
	}
	else 
	{
		rc = lseek(fsImage, iNodeMapPtr, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = read(fsImage, iNodeMapPiece, 16*sizeof(int));
		if(rc < 0) 
			return -1;
		
		iNodeMapPiece[iNodeMapOffset] = iNodePtr = cr->logEnd + 16*sizeof(int);
		rc = lseek(fsImage, cr->logEnd, SEEK_SET);
		if(rc < 0) 
			return -1;
		
		rc = write(fsImage, iNodeMapPiece, 16*sizeof(int));
		if(rc < 0) 
			return -1;
		
		cr->iNodeMaps[currInodeNum/16] = cr->logEnd;
		cr->logEnd += 16*sizeof(int);
		rc = updateCR();
		if(rc < 0) 
			return -1;
		
	}
	iNodeMap[currInodeNum] = iNodePtr;

	INode *iNode = malloc(sizeof(INode));
	iNode->type = type;
	iNode->size = size;
	if(type == MFS_DIRECTORY) 
	{
		iNode->blocks[0] = iNodePtr + sizeof(INode);
	}
	rc = lseek(fsImage, iNodePtr, SEEK_SET);
	if(rc < 0) 
		return -1;
	
	rc = write(fsImage, iNode, sizeof(INode));
	if(rc < 0) 
		return -1;
	

	cr->logEnd += sizeof(INode);
	rc = updateCR();
	if(rc < 0) 
		return -1;
	

	currInodeNum++;
	return currInodeNum-1;
}

int FileInit(int pINum, char *name) 
{
	int iNodeNum = InodeInit(MFS_REGULAR_FILE, 0);
	if(iNodeNum < 0) 
		return -1;

	int rc = ParentDirWrite(pINum, iNodeNum, name);
	if(rc < 0) 
		return -1;

	return 0;
}

int DirInit(int pINum, char *name) 
{
	int iNodeNum = InodeInit(MFS_DIRECTORY, 2*sizeof(MFS_DirEnt_t));
	if(iNodeNum < 0) 
		return -1;
	
	MFS_DirEnt_t *block = malloc(MFS_BLOCK_SIZE);
	sprintf(block[0].name,".");
	block[0].iNum = iNodeNum;
	sprintf(block[1].name, "..");
	block[1].iNum = pINum;
	int i;
	for (i = 2; i < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t); i++) 
	{
		block[i].iNum = -1;
	}
	int rc = lseek(fsImage, cr->logEnd, SEEK_SET);
	if(rc < 0) 
		return -1;
	
	rc = write(fsImage, block, MFS_BLOCK_SIZE);
	if(rc < 0) 
		return -1;
	
	cr->logEnd += MFS_BLOCK_SIZE;
	rc = updateCR();
	if(rc < 0) 
		return -1;
	
	if(pINum != iNodeNum) 
	{
		rc = ParentDirWrite(pINum, iNodeNum, name);
		if(rc < 0) 
			return -1;
	}

	return 0;
}

int FileRead(int iNum, char *buffer, int block) 
{
	INode *iNode = FindINode(iNum);
	if(iNode == NULL) 
		return -1;
	
	int blockPointer = iNode->blocks[block];
	rc = lseek(fsImage, blockPointer, SEEK_SET);
	if(rc < 0) 
		return -1;
	
	rc = read(fsImage, buffer, MFS_BLOCK_SIZE);
	if(rc < 0) 
		return -1;
	
	return 0;
}

void FS_Creat(int pINum, int type, char *name) 
{
 	Packet reply;
	int retCode = -1;	
	if(strlen(name) <= 28) 
	{
		if(type == MFS_REGULAR_FILE) 
			retCode = FileInit(pINum, name);
		else 
			retCode = DirInit(pINum, name);
	}

	fsync(fsImage);
  	reply.inum = retCode;
  	reply.request= REQ_RESPONSE;
  	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
}

void FS_Lookup(int pINum, char *name) 
{
  	Packet reply;
	int retCode = -1;	

	INode *iNode = FindINode(pINum);
	if(iNode != NULL && iNode->type == MFS_DIRECTORY) 
	{
		retCode = ParentDirGet(pINum, name);
	}

	fsync(fsImage);
  	reply.inum = retCode;
  	reply.request = REQ_RESPONSE;
  	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
}

void FS_Write(int iNum, char *buffer, int block) 
{
  	Packet reply;
	int retCode = -1;		
	
	if(block >= 0 && block < 14) 
	{
		retCode = FileWrite(iNum, buffer, block);
	}

	fsync(fsImage);
  	reply.inum = retCode;
  	reply.request= REQ_RESPONSE;
  	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));

}

void FS_Stat(int iNum)
{
  	Packet reply;
	int retCode = -1;

    MFS_Stat_t *stat = malloc(sizeof(MFS_Stat_t));
	INode *iNode = FindINode(iNum);
	if(iNode != NULL) 
	{
		stat = (MFS_Stat_t *)iNode;		
		retCode = 0;
	}

	fsync(fsImage);
  	reply.stat.size = stat->size;
  	reply.stat.type = stat->type;
  	reply.inum = retCode;
  	reply.request= REQ_RESPONSE;
  	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
}

void FS_Read(int iNum, int block) 
{
  	Packet reply;
	int retCode = -1;

	char *buffer = malloc(MFS_BLOCK_SIZE);
	if(block >= 0 && block < 14) 
	{
		retCode = FileRead(iNum, buffer, block);
	}

	fsync(fsImage);
	memcpy(reply.buffer, buffer, MFS_BLOCK_SIZE);
  	reply.inum = retCode;
  	reply.request= REQ_RESPONSE;
 	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
}

void FS_Unlink(int pINum, char *name) 
{
  	Packet reply;
	int retCode = -1;
	
	int iNum = ParentDirGet(pINum, name);
	if(iNum >= 0) 
	{
		retCode = DirIsEmpty(iNum);
		if(retCode == 0) 
			retCode = ParentDirWrite(pINum, -1, name);
		
	}
	else 
		retCode = 0;
	

	fsync(fsImage);
  	reply.inum = retCode;
  	reply.request= REQ_RESPONSE;
  	rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
}

void FS_Shutdown(int fd) 
{
  	Packet reply;

  	reply.request = REQ_RESPONSE;
  	UDP_Write(sd, addr, (char*)&reply, sizeof(Packet));
	fsync(fd);
	close(fd);
	exit(0);
}

int server_init(int portnum, char* image)
{
    sd = UDP_Open(portnum);
    assert(sd > -1);

	fsImage = open(image, O_RDWR|O_CREAT, S_IRWXU);
	assert(fsImage >= 0);

	currInodeNum = 0;
	addr = malloc(sizeof(struct sockaddr_in));
	cr = malloc(sizeof(CheckpointRegion));
	iNodeMap = malloc(4096*sizeof(int));
	struct stat imageStat;
	stat(image, &imageStat);
	if (!imageStat.st_size) 
	{
		cr->logEnd = sizeof(CheckpointRegion);
		updateCR();
		DirInit(currInodeNum, "");
		fsync(fsImage);
	} 
	else 
	{
		read(fsImage, cr, sizeof(CheckpointRegion));
		int i;
		for (i = 0; i < 256; i++) 
		{
			int iNodeMapPtr = cr->iNodeMaps[i];
			if(iNodeMapPtr > 0) 
			{			
				lseek(fsImage, iNodeMapPtr, SEEK_SET);
				read(fsImage, iNodeMap + (16*i), 16*sizeof(int));
			}
			else 
			{
				int j;
				for(j = 0; j < 16; j++) 
				{
					if(iNodeMap[(16*(i-1)) + j] == 0) 
					{
						currInodeNum = (16*(i-1)) + j;
						break;
					}
				}
				if(currInodeNum == 0) 
				{
					currInodeNum = 16*i;
				}
				break;
			}
		}
	}

    printf("server: waiting...\n");
    Packet msg;
    while (1) 
    {
      int rc = UDP_Read(sd, addr, (char *)&msg, sizeof(Packet));
      if (rc > 0) 
      {
        if (msg.request == REQ_LOOKUP)
        {
          FS_Lookup(msg.inum, msg.name);
        }
        else if(msg.request == REQ_STAT)
        {
          FS_Stat(msg.inum);
        }
        else if(msg.request == REQ_WRITE)
        {
          FS_Write(msg.inum, msg.buffer, msg.block);
        }
        else if(msg.request == REQ_READ)
        {
          FS_Read(msg.inum, msg.block);
        }
        else if(msg.request == REQ_CREAT)
        {
          FS_Creat(msg.inum, msg.type, msg.name);
        }
        else if(msg.request == REQ_UNLINK)
        {
          FS_Unlink(msg.inum, msg.name);
        }
        else if(msg.request == REQ_SHUTDOWN) 
        {
          FS_Shutdown(fsImage);
        }
        else 
		{
          perror("server_init: unknown request");
          return -1;
        }
      }
    }

    return 0;
}

int
main(int argc, char *argv[])
{
	if (argc != 3)
		perror("Incorrect number of arguments\n");

	server_init(atoi(argv[1]), argv[2]);

    return 0;
}