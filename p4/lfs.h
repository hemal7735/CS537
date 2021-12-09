#ifndef __LFS_H__
#define __LFS_H__
#define NUM_BLOCKS 	14				// max number of blocks per inode
#define NUM_INODES 	4096			// max number of inodes in system
#define CP_REGION_SIZE		6					// size (in blocks) of checkpoint region
#define BLOCK_SIZE	4096			// size (in bytes) of one block
#define DIRENTRY_SIZE	32		// size (in bytes) of a directory entry
#define NUM_ENTRIES	(BLOCK_SIZE/DIRENTRY_SIZE)	// number of entries per block in a directory
#define NAME_LENGTH	28			// length (in bytes) of a directory entry name

typedef struct _Inode {
	int inum;
	int size; // number of bytes in the file. a multiple of BLOCKSIZE
	int type;
	int used[NUM_BLOCKS]; // used[i] is true if blocks[i] is used
	int blocks[NUM_BLOCKS]; // address in memory of each block
} Inode;

typedef struct _DirBlock {
	char names[NUM_ENTRIES][NAME_LENGTH];
	int  inums[NUM_ENTRIES];
} DirBlock;

#ifndef __MFS_h__
#ifndef __MESSAGE_H__
#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

#endif
#endif

int Startup(char *filePath);
int Lookup(int pinum, char *name);
int Stat(int inum, MFS_Stat_t *m);
int Write(int inum, char *buffer, int block);
int Read(int inum, char *buffer, int block);
int Creat(int pinum, int type, char *name);
int Unlink(int pinum, char *name);
int Shutdown();

int inode_lookup(int inum, Inode* n);
int build_dirBlock(int firstBlock, int inum, int pinum);
void sync_CR(int inum);

#endif
