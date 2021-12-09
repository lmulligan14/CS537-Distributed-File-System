#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)
#define MAX_NUM_INODES   (4096)
#define IMAP_PIECE_SIZE  (16)
#define NUM_INODE_PIECES (MAX_NUM_INODES/IMAP_PIECE_SIZE) // 256

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

// struct message/packet
typedef struct packet {
    int request;
    char block[MFS_BLOCK_SIZE];
    int inum;
    int blocknum;
    char name[64];
    int type;
    MFS_Stat_t stat;
} packet;

// struct for INode
typedef struct INode {
    int type;
    int size;
    int blocks[14];
} INode;

// struct for checkpoint region
typedef struct CheckpointRegion {
    int logEnd;
    int imap[NUM_INODE_PIECES]; // holds 256 imap pieces
} CheckpointRegion;

// struct for imap piece
typedef struct ImapPiece {
    int inodes[IMAP_PIECE_SIZE]; // holds 16 inodes
} ImapPiece;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__