#ifndef __PACKETS_H__
#define __PACKETS_H__
#define BUFFER_SIZE (4096)
#define MAX_NAME_SIZE (28)
#ifndef __MFS_h__

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

enum M_type {
	LOOKUP,
	STAT,
	WRITE,
	READ,
	CREAT,
	UNLINK,
	SHUTDOWN,
	REPLY // since we are using the same struct both side, we need this to populate some value
};

// we needed this struct as messages are heterogenous
typedef struct _Message {
	enum M_type m_type;
    MFS_Stat_t stat;

	int inum;
    int block;
	int type;
	int rc; // shows if the op was a success or not

    char name[MAX_NAME_SIZE];
    char buffer[BUFFER_SIZE];
} Message;

#endif