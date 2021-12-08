#include "lfs.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int fd;
int inode_map[INODES_LIMIT];
int next_block;
char INVALID_ENTRY[] = "INVALID_ENTRY";
char DOT[] = ".";
char DOT_DOT[] = "..";

void sync_CR(int inum) {
    lseek(fd, INODES_LIMIT*sizeof(int), SEEK_SET);
	write(fd, &next_block, sizeof(int));

    if(inum != -1) {
        // inode map is written first so it is straight-forward
		lseek(fd, inum*sizeof(int), SEEK_SET);
		write(fd, &inode_map[inum], sizeof(int));
	}
}

int inode_lookup(int inum, Inode* node) {
    if (inum < 0 || inum >= INODES_LIMIT) {
        return -1;
    }

    lseek(fd, inode_map[inum] * BLOCK_SIZE, SEEK_SET);
    read(fd, node, sizeof(Inode));

    return 0;
}

int Startup(char *filePath) {
    if((fd = open(filePath, O_RDWR)) == -1) {

        if ((fd = open(filePath, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU)) == -1) {
            return -1;
        }

        // -1 means that it is not occupied
        for(int i = 0; i < INODES_LIMIT; i++) {
            inode_map[i] = -1;
        }

        next_block = CP_REGION_SIZE;

        // CR region contains the inodes and last block written size
        lseek(fd, 0, SEEK_SET);
		write(fd, inode_map, sizeof(int)*INODES_LIMIT);
		write(fd, &next_block, sizeof(int));


        // 1. prepare the datablock for directory
        DirBlock dirBlock;
		dirBlock.inums[0] = 0;
		dirBlock.inums[1] = 0;
		strcpy(dirBlock.names[0], DOT);
		strcpy(dirBlock.names[1], DOT_DOT);

		for(int i = 2; i < NENTRIES; i++) {
			strcpy(dirBlock.names[i], INVALID_ENTRY);
            dirBlock.inums[i] = -1;
		}

        // 2. dump datablock for directory
		lseek(fd, next_block*BLOCK_SIZE, SEEK_SET);
		write(fd, &dirBlock, sizeof(DirBlock));


        // 3. prepare inode for root
        Inode root_inode;

        root_inode.inum = 0;
        root_inode.type = MFS_DIRECTORY;
        root_inode.size = BLOCK_SIZE;

        root_inode.used[0] = 1;
        root_inode.blocks[0] = next_block;

        for(int i = 1; i < BLOCKS_LIMIT; i++) {
            root_inode.used[i] = -1;
            root_inode.blocks[i] = -1;
        }

        // 4. dump inode for root
        next_block++;
        inode_map[root_inode.inum] = next_block;

		lseek(fd, next_block*BLOCK_SIZE, SEEK_SET);
		write(fd, &root_inode, sizeof(Inode));
		next_block++;

		// 5. update checkpoint region
		sync_CR(root_inode.inum);

    } else {
        lseek(fd, 0, SEEK_SET);
        read(fd, inode_map, sizeof(int)*INODES_LIMIT);
        read(fd, &next_block, sizeof(int));

        printf("printig indoe \n");
        for(int i = 0; i < 5; i++) {
            printf("%d ", inode_map[i]);
        }

        printf("printig indoe done \n");

        printf("next block : %d\n", next_block);
    }

    return 0;
}

int Lookup(int pinum, char *name) {
    return 0;
}

int Stat(int inum, MFS_Stat_t *m) {
    return 0;
}

int Write(int inum, char *buffer, int block) {
    return 0;
}

int Read(int inum, char *buffer, int block) {
    return 0;
}

int Creat(int pinum, int type, char *name) {
    return 0;
}

int Unlink(int pinum, char *name) {
    return 0;
}

int Shutdown() {
    close(fd);
    return 0;
}