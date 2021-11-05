#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>

#define Q_SIZE 3

int total_files;

typedef struct _input_chunk {
    char *buf;
    int size;
    int file_id;
    int page_id;
} input_chunk;

typedef struct _output_chunk {
    char *buf;
    int size;
} output_chunk;

input_chunk input_chunks[Q_SIZE];
output_chunk **output_chunks;
int *pages_count;

int q_size = 0;
int q_head = 0, q_tail = 0;
int queue[Q_SIZE];

// 0 - not full
// 1 - full
int isQueueFull() {
    if ((q_head + 1) % Q_SIZE == q_tail) {
        return 1;
    }
    return 0;
}

// 0 - not empty
// 1 - empty
int isQueueEmpty() {
    if (q_head == q_tail) {
        return 1;
    }
    return 0;
}

// 0 - failure
// 1 - success
int enqueue(int val) {
    if (isQueueFull()) return -1;

    queue[q_head] = val;
    q_head = (q_head + 1) % Q_SIZE;

    return 1;
}

// -1 - failure
int dequeue() {
    if (isQueueEmpty()) return -1;

    int val = queue[q_tail];
    q_tail = (q_tail + 1)%Q_SIZE;

    return val;
}

void readFile2(char *filename) {
    int fildes = open(filename, O_RDONLY);

    if (fildes == -1) {
        printf("error getting fstats\n");
        close(fildes);
        exit(1);
    }

    struct stat sb;

    if (fstat(fildes, &sb) == -1) {
        printf("error getting fstats\n");
        close(fildes);
        exit(1);
    }

    int PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    int total_pages = sb.st_size/PAGE_SIZE + (sb.st_size % PAGE_SIZE != 0);

    printf("[file:%s] [size:%ld] [total pages:%d]\n", filename, sb.st_size, total_pages);

    // TODO: what happens if the file cannot be fit inside the memory?
    
    int offset = 0;
    
    for(int page = 1; page <= total_pages; page++) {
        
        printf("START: [page:%d]\n", page);

        int limit = PAGE_SIZE;

        if (page == total_pages - 1) {
            limit = sb.st_size % PAGE_SIZE;
        }

        char *ptr = mmap(NULL, limit, PROT_READ, MAP_PRIVATE, fildes, offset);

        for(int i = 0; i < limit; i++) {
            printf("%c", ptr[i]);
        }
        
        offset += PAGE_SIZE;

        printf("\nEND: [page:%d]\n", page);
    }
}


void readFiles(char **filenames) {

    int PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
    
    for(int fid = 0; fid < total_files; fid++) {
        char *filename = filenames[fid];
        int fildes = open(filenames[0], O_RDONLY);

        if (fildes == -1) {
            printf("error getting fstats\n");
            close(fildes);
            exit(1);
        }

        struct stat sb;

        if (fstat(fildes, &sb) == -1) {
            printf("error getting fstats\n");
            close(fildes);
            exit(1);
        }

        int total_pages = sb.st_size/PAGE_SIZE + (sb.st_size % PAGE_SIZE != 0);

        // init chunks array for output based on the number of pages
        output_chunks[fid] = (output_chunk*)malloc(total_pages * sizeof(output_chunk));
        pages_count[fid] = total_pages;

        printf("[file:%s] [size:%ld] [total pages:%d]\n", filename, sb.st_size, total_pages);

        // TODO: what happens if the file cannot be fit inside the memory?
        char *ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fildes, 0);
        
        for(int page = 1; page <= total_pages; page++) {
            
            printf("START: [page:%d]\n", page);

            int limit = PAGE_SIZE;

            if (page == total_pages - 1) {
                limit = sb.st_size % PAGE_SIZE;
            }

            for(int i = 0; i < limit; i++) {
                printf("%c", ptr[i]);
            }

            ptr += limit;

            printf("\nEND: [page:%d]\n", page);
        }
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
		exit(1);
    }

    total_files = argc - 1;
    pages_count = (int*)malloc(total_files * sizeof(int));
    output_chunks = (output_chunk**)malloc(total_files * sizeof(output_chunk*));

    readFiles(argv + 1);

    for(int i = 0; i < total_files; i++) {
        printf("[page_id:%d] [pages:%d]\n", i, pages_count[i]);
    }
    
    return 0;
}