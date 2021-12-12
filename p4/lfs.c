#include "lfs.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int fd;
int inode_map[NUM_INODES];
int next_block;
char INVALID_ENTRY[] = "INVALID_ENTRY";
char DOT[] = ".";
char DOT_DOT[] = "..";

int debug_on = 1;

void sync_CR(int inum) {
    lseek(fd, NUM_INODES * sizeof(int), SEEK_SET);
	write(fd, &next_block, sizeof(int));

    if(inum != -1) {
        // inode map is written first so it is straight-forward
		lseek(fd, inum*sizeof(int), SEEK_SET);
		write(fd, &inode_map[inum], sizeof(int));
	}
}

int inode_lookup(int inum, Inode* node) {
    if (inum < 0 || inum >= NUM_INODES) {
        return -1;
    }

    lseek(fd, inode_map[inum] * BLOCK_SIZE, SEEK_SET);
    read(fd, node, sizeof(Inode));

    return 0;
}

int build_dirBlock(int needDefaults, int inum, int pinum) {
    DirBlock dirBlock;

    for(int i = 0; i < NUM_ENTRIES; i++) {
        strcpy(dirBlock.names[i], INVALID_ENTRY);
        dirBlock.inums[i] = -1;
    }

    if (needDefaults) {
        strcpy(dirBlock.names[0], DOT);
        dirBlock.inums[0] = inum;

        strcpy(dirBlock.names[1], DOT_DOT);
        dirBlock.inums[1] = pinum;
    }

    lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
    write(fd, &dirBlock, BLOCK_SIZE);
    int block_address = next_block;

    next_block++;

    return block_address;
}

int Startup(char *filePath) {
    if((fd = open(filePath, O_RDWR)) == -1) {

        if ((fd = open(filePath, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU)) == -1) {
            return -1;
        }

        // -1 means that it is not occupied
        for(int i = 0; i < NUM_INODES; i++) {
            inode_map[i] = -1;
        }

        next_block = CP_REGION_SIZE;

        // CR region contains the inodes and last block written size
        lseek(fd, 0, SEEK_SET);
		write(fd, inode_map, NUM_INODES * sizeof(int));
		write(fd, &next_block, sizeof(int));


        // 1. prepare the datablock for directory
        DirBlock dirBlock;
		dirBlock.inums[0] = 0;
		dirBlock.inums[1] = 0;
		strcpy(dirBlock.names[0], DOT);
		strcpy(dirBlock.names[1], DOT_DOT);

		for(int i = 2; i < NUM_ENTRIES; i++) {
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

        for(int i = 1; i < NUM_BLOCKS; i++) {
            root_inode.used[i] = 0;
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
        read(fd, inode_map, NUM_INODES * sizeof(int));
        read(fd, &next_block, sizeof(int));

        // printf("printig indoe \n");
        // for(int i = 0; i < 5; i++) {
        //     printf("%d ", inode_map[i]);
        // }

        // printf("printig indoe done \n");

        // printf("next block : %d\n", next_block);
    }

    return 0;
}

int Lookup(int pinum, char *name) {
    Inode parent_inode;

    // parent does not exist
    if (inode_lookup(pinum, &parent_inode) == -1) {
        return -1;
    }

    for(int block = 0; block < NUM_BLOCKS; block++) {
        if (parent_inode.used[block]) {
            DirBlock dirblock;

            lseek(fd, parent_inode.blocks[block] * BLOCK_SIZE, SEEK_SET);
			read(fd, &dirblock, BLOCK_SIZE);

            for(int entry = 0; entry < NUM_ENTRIES; entry++) {
				if(dirblock.inums[entry] != -1 
                    && strcmp(name, dirblock.names[entry]) == 0) {
					return dirblock.inums[entry];
				}
			}
        }
    }

    return -1;
}

int Stat(int inum, MFS_Stat_t *stat) {
    Inode inode;

	if(inode_lookup(inum, &inode) == -1)
		return -1;

	stat->type = inode.type;
	stat->size = inode.size;

	return 0;
}

int Write(int inum, char *buffer, int block) {
    // invalid block check
    if(block < 0 || block >= NUM_BLOCKS) {
		return -1;
    }
    
    Inode inode;

    // invalid inode
    if (inode_lookup(inum, &inode) == -1) {
        return -1;
    }

    // we can only write to the file
    if (inode.type != MFS_REGULAR_FILE) {
        return -1;
    }
    

    int newSize = (block + 1) * BLOCK_SIZE;
    // if new size is greater than the existing size, extend the current size
    if (newSize > inode.size) {
        inode.size = newSize;
    }

    inode.used[block] = 1;
    inode.blocks[block] = next_block;

    // 1. write buffer
	lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
	write(fd, buffer, BLOCK_SIZE);

    // next block is inode, write inode chunk
    next_block++;    
	lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
	write(fd, &inode, BLOCK_SIZE);
	inode_map[inum] = next_block;

    // advance next_block pointer
	next_block++;

	// write checkpoint region
	sync_CR(inum);
    return 0;
}

int Read(int inum, char *buffer, int block) {
    Inode inode;

    // invalid inode
    if (inode_lookup(inum, &inode) == -1) {
        if (debug_on) {
            perror("Read::inode lookup\n");
        }
        return -1;
    }

    // invalid block check
    if(block < 0 || block >= NUM_BLOCKS || !inode.used[block]) {
        if (debug_on) {
            perror("Read::invalid block number\n");
        }
        return -1;
    }

    // different handling for different types
    if(inode.type == MFS_DIRECTORY){
		DirBlock dirBlock;																				// read dirBlock
		lseek(fd, inode.blocks[block] * BLOCK_SIZE, SEEK_SET);
		read(fd, &dirBlock, BLOCK_SIZE);

        // convert DirBlock to MRS_DirEnt_t
		MFS_DirEnt_t dir_entries[NUM_ENTRIES];

		for(int i = 0; i < NUM_ENTRIES; i++) {
			MFS_DirEnt_t dir_entry;
			strcpy(dir_entry.name, dirBlock.names[i]);
			dir_entry.inum = dirBlock.inums[i];

			dir_entries[i] = dir_entry;
		}

		memcpy(buffer, dir_entries, NUM_ENTRIES * sizeof(MFS_DirEnt_t));
	} else {
		if(lseek(fd, inode.blocks[block] * BLOCK_SIZE, SEEK_SET) == -1) {
            if (debug_on) {
                perror("Server_Read: lseek:");
                printf("Server_Read: lseek failed\n");
            }
            return -1;
		}
		
		if(read(fd, buffer, BLOCK_SIZE) == -1) {
			if (debug_on) {
                perror("Server_Read: read:");
                printf("Server_Read: read failed\n");
            }
            return -1;
		}
    }

    return 0;
}

// Failure modes: pinum does not exist, 
// or name is too long. 
// If name already exists, return success (think about why).
int Creat(int pinum, int type, char *name) {
    if (strlen(name) > NAME_LENGTH) {
        return -1;
    }

    // it has to be either directory or file
    if (!(type == MFS_DIRECTORY || type == MFS_REGULAR_FILE)) {
        return -1;
    }
    
    // don't create if already exists
    if (Lookup(pinum, name) != -1) {
        return 0;
    }

    Inode parent_inode;
    if (inode_lookup(pinum, &parent_inode) == -1) {
        return -1;
    }

    if (parent_inode.type != MFS_DIRECTORY) {
        return -1;
    }

    int free_inum = -1;
    for(int i = 0; i < NUM_INODES; i++) {
        if (inode_map[i] == -1) {
            free_inum = i;
            break;
        }
    }

    if (free_inum == -1) {
        return -1;
    }

    int block = 0, entry, block_num;
    DirBlock dirBlock;

    while (block < NUM_BLOCKS) {
        if (parent_inode.used[block]) {
            lseek(fd, parent_inode.blocks[block] * BLOCK_SIZE, SEEK_SET);
            read(fd, &dirBlock, sizeof(DirBlock));

            for(entry = 0; entry < NUM_ENTRIES; entry++) {
                if (dirBlock.inums[entry] == -1) {
                    goto found_block;
                }
            }

            block++;

        } else {
            // create a directory block but don't assign a parent yet
            int block_address = build_dirBlock(0, free_inum, -1);

            // 1. modify parent inode
            parent_inode.blocks[block] = block_address;
            parent_inode.size += BLOCK_SIZE;
            parent_inode.used[block] = 1;

            // 2. write parent inode
            lseek(fd, inode_map[pinum] * BLOCK_SIZE, SEEK_SET);
            write(fd, &parent_inode, BLOCK_SIZE);
        }
    }

    // if we reached here that means that there is no space for new entry
    return -1;

    found_block:

    // 1. register name and inode entry to block entry list
    strcpy(dirBlock.names[entry], name);
    dirBlock.inums[entry] = free_inum; // inode with this num will be created in a moment

    // 2. write it out    
    lseek(fd, parent_inode.blocks[block] * BLOCK_SIZE, SEEK_SET);
	write(fd, &dirBlock, BLOCK_SIZE);

    // all the registration is done at the moment
    // now we need to handle file and directory sep
    // both will have an inode and data blocks
    Inode inode;

    inode.type = type;
    inode.size = 0;
    inode.inum = free_inum;

    for(int i = 0; i < NUM_BLOCKS; i++) {
        inode.used[i] = 0;
        inode.blocks[i] = -1;
    }

    if (type == MFS_DIRECTORY) {
        block_num = build_dirBlock(1, free_inum, pinum);
        inode.used[0] = 1;
        inode.blocks[0] = block_num;
        inode.size += BLOCK_SIZE;
    }

    inode_map[free_inum] = next_block;

    // write inode
	lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
	write(fd, &inode, sizeof(Inode));
	next_block++;

    sync_CR(free_inum);

    return 0;
}

int Unlink(int pinum, char *name) {
    if (strlen(name) > NAME_LENGTH) {
        return -1;
    }
    
    Inode parent_inode;

    if (inode_lookup(pinum, &parent_inode) == -1) {
        return -1;
    }

    int cinum = Lookup(pinum, name);
    
    Inode child_inode;
    // it is fine if it does not exist
    if (inode_lookup(cinum, &child_inode) == -1) {
        return 0;
    }

    // we have some common operation for directory and file
    // but for directory, we need to make sure that it is empty

    if (child_inode.type == MFS_DIRECTORY) {
        for(int block = 0; block < NUM_BLOCKS; block++) {

            if (child_inode.used[block]) {
                DirBlock dirBlock;
                lseek(fd, child_inode.blocks[block] * BLOCK_SIZE , SEEK_SET);
                read(fd, &dirBlock, BLOCK_SIZE);

                for(int entry = 0; entry < NUM_ENTRIES; entry++) {
                    if (dirBlock.inums[entry] != -1
                        && strcmp(dirBlock.names[entry], DOT) != 0
                        && strcmp(dirBlock.names[entry], DOT_DOT) != 0) {
                        return -1;
                    }
                }
            }
            
        }
    }

    // 1. remove entry from the parent
    // 2. update parent and write the new block, update block map

    int block = 0, entry = 0;

    for(block = 0; block < NUM_BLOCKS; block++) {
        if (parent_inode.used[block]) {
            DirBlock dirBlock;

            lseek(fd, parent_inode.blocks[block] * BLOCK_SIZE, SEEK_SET);
            read(fd, &dirBlock, BLOCK_SIZE);
 
            for(entry = 0; entry < NUM_ENTRIES; entry++) {
                if (dirBlock.inums[entry] != -1
                    && strcmp(name, dirBlock.names[entry]) == 0) {
                        // clean up entry
                        dirBlock.inums[entry] = -1;
						strcpy(dirBlock.names[entry], INVALID_ENTRY);

                        // write this block
                        lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
                        write(fd, &dirBlock, BLOCK_SIZE);
                        

                        // update parent with new block
                        parent_inode.blocks[block] = next_block;

                        next_block++;

                        // write new parent inode and update that in inode_map
                        lseek(fd, next_block * BLOCK_SIZE, SEEK_SET);
                        write(fd, &parent_inode, BLOCK_SIZE);
                        inode_map[pinum] = next_block;

                        next_block++;

                        sync_CR(pinum);

                        goto parent_update_done;
                }
            }
        }
    }


    parent_update_done:

    // 3. set -1 in inode map for inode
    inode_map[cinum] = -1;

    // 4. sync CR
    sync_CR(cinum);
    return 0;
}

int Shutdown() {
    fsync(fd);
    close(fd);
    exit(0);
    return 0;
}