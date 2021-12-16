#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)
#define MAX_NUM_INODES   (4096)
#define IMAP_PIECE_SIZE  (16)
#define LEN_NAME         (60)
#define NUM_INODE_PIECES (MAX_NUM_INODES/IMAP_PIECE_SIZE) // 256

enum MFS_REQ {
  REQ_INIT,
  REQ_LOOKUP,
  REQ_STAT,
  REQ_WRITE,
  REQ_READ,
  REQ_CREAT,
  REQ_UNLINK,
  REQ_RESPONSE,
  REQ_SHUTDOWN
};

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} Stat;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  iNum;      // inode number of entry (-1 means entry not used)
} DirEnt;

// struct message/packet
typedef struct __UDP_Packet {
	enum MFS_REQ request;

	int inum;
	int block;
	int type;

	char name[LEN_NAME];
	char buffer[MFS_BLOCK_SIZE];
	Stat stat;
} UDP_Packet;


// struct for INode
typedef struct __MFS_INode_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
	int blocks[14];
    // note: no permissions, access times, etc.
} INode;

// struct for checkpoint region
typedef struct __MFS_CheckReg_t {
    int logEnd;
	int iNodeMaps[256];
} CheckReg;

// struct for imap piece
//takes inode number as input and produces the disk address
//of the most recent version of the inode
typedef struct ImapPiece {
    int inodes[IMAP_PIECE_SIZE]; // holds 16 inodes
} ImapPiece;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, Stat *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__