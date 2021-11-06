#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>

#define Q_SIZE 1000

int total_files;

typedef struct _input_chunk {
    char *buf;
    int size;
    int file_id;
    int page_id;
} input_chunk_t;

typedef struct _output_chunk {
    char *chars;
    int *runs;
    int size;
} output_chunk_t;

input_chunk_t input_chunks[Q_SIZE];
output_chunk_t **output_chunks;
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

int enqueue(input_chunk_t chunk) {
    // if (isQueueFull()) return -1;

    input_chunks[q_head] = chunk;
    q_head = (q_head + 1) % Q_SIZE;

    return 1;
}

input_chunk_t dequeue() {
    // if (isQueueEmpty()) return NULL;

    input_chunk_t chunk = input_chunks[q_tail];
    q_tail = (q_tail + 1)%Q_SIZE;

    return chunk;
}

void computeRLE(input_chunk_t input) {
    output_chunk_t output;
    // 1-1 mapping between character and counts
	char *chars = malloc(input.size * sizeof(char));
    int *runs = malloc(input.size * sizeof(int));
    int idx = 0;

    // TODO: remove this
    // for(int l = 0; l < input.size; l++) {
    //     if (input.buf[l] == '\n') {
    //         printf("bingo in RLE first\n");
    //     }
    // }
    int i = 0;

    while(i < input.size) {
        // puts("for loop");

        if (input.buf[i] == '\n') {
            // printf("bingo in RLE second\n");
        }

        char c = input.buf[i];

        int run = 0;

        while(i < input.size && input.buf[i] == c) {
            i++;
            run++;
        }

        // printf("[char-%c] [run-%d]\n", c, run);

        chars[idx] = c;
        runs[idx] = run;
        idx++;
    }   

    // collect info and re-align
    output.size = idx;
    // output.chars = realloc(chars, output.size);
    // output.runs = realloc(runs, output.size);

    output.chars = chars;
    output.runs = runs;

    output_chunks[input.file_id][input.page_id] = output;
}

void readFiles(char **filenames) {

    int PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
    
    for(int fid = 0; fid < total_files; fid++) {
        char *filename = filenames[fid];
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

        int total_pages = sb.st_size/PAGE_SIZE + (sb.st_size % PAGE_SIZE != 0);

        // init chunks array for output based on the number of pages
        output_chunks[fid] = (output_chunk_t*)malloc(total_pages * sizeof(output_chunk_t));
        pages_count[fid] = total_pages;

        // printf("[file:%s] [size:%ld] [total pages:%d]\n", filename, sb.st_size, total_pages);

        // TODO: what happens if the file cannot be fit inside the memory?
        char *ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fildes, 0);
        
        for(int page = 0; page < total_pages; page++) {
            
            int actual_size = PAGE_SIZE;

            // last page might not be fully filled
            if (page == total_pages - 1) {
                actual_size = sb.st_size % PAGE_SIZE;
            }

            input_chunk_t input_chunk;

            input_chunk.buf = ptr;
            input_chunk.file_id = fid;
            input_chunk.page_id = page;
            input_chunk.size = actual_size;

            // for(int l = 0; l < input_chunk.size; l++) {
            //     if (ptr[l] == '\n') {
            //         puts("bingo");
            //     }
            // }

            computeRLE(input_chunk);

            ptr += actual_size;
        }
    }

}

void dump() {
    int useFwrite = 1;
    
    for(int fid = 0; fid < total_files; fid++) {
        int pages = pages_count[fid];

        for(int page = 0; page < pages; page++) {
            output_chunk_t output = output_chunks[fid][page];
            
            int i;

            for(i = 0; i < output.size; i++) {
                int run = output.runs[i];
                char c = output.chars[i];

                if (useFwrite) {
                    fwrite(&run, sizeof(run), 1, stdout);
                    fwrite(&c, sizeof(c), 1, stdout);
                } else {
                    printf("%d%c", run, c);
                }
            }

            // printf("total chars:%d\n", i);
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
    output_chunks = (output_chunk_t**)malloc(total_files * sizeof(output_chunk_t*));

    readFiles(argv + 1);
    dump();
    
    return 0;
}