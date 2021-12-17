#include "mfs.h"

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)
#define MAX_NUM_INODES   (4096)
#define IMAP_PIECE_SIZE  (16)
#define NUM_INODE_PIECES (MAX_NUM_INODES/IMAP_PIECE_SIZE) // 256
#define MFS_BLOCK_SIZE (4096)

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

// struct message/packet
typedef struct Packet {
	enum MFS_REQ request;

	int inum;
	int block;
	int type;

	char name[64];
	char buffer[MFS_BLOCK_SIZE];
	MFS_Stat_t stat;
} Packet;


// struct for INode
typedef struct INode {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
	int blocks[14];
} INode;

// struct for checkpoint region
typedef struct CheckpointRegion {
    int logEnd;
	int iNodeMaps[256];
} CheckpointRegion;

// struct for imap piece
//takes inode number as input and produces the disk address
//of the most recent version of the inode
typedef struct ImapPiece {
    int inodes[IMAP_PIECE_SIZE]; // holds 16 inodes
} ImapPiece;