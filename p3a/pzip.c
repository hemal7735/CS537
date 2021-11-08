#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>


/**
 * I'm using the batch framework concept for parallizing.
 * Reader: Reader reads items in bunch and puts into some queue
 * Processor: Processor picks items from the queue and process it
 * Writer: Writer works synchronously, and takes items emitted by the processor
 * 
 **/


#define Q_SIZE 100000
// large value gives better performance because then you spend more time compressing
int PAGE_SIZE = 4096 * 2000; 

// variables for concurrency
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_q_empty = PTHREAD_COND_INITIALIZER, cond_q_fill = PTHREAD_COND_INITIALIZER;

int reader_done = 0;
int total_files, total_pages = 0;
int *pages_count;

// switch between printf and fwrite
int useFwrite = 1;

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

int q_head = 0, q_tail = 0;

int isQueueFull() {
    if ((q_head + 1) % Q_SIZE == q_tail) {
        return 1;
    }
    return 0;
}

int isQueueEmpty() {
    if (q_head == q_tail) {
        return 1;
    }
    return 0;
}

void enqueue(input_chunk_t chunk) {
    input_chunks[q_head] = chunk;
    q_head = (q_head + 1) % Q_SIZE;
}

input_chunk_t dequeue() {
    input_chunk_t chunk = input_chunks[q_tail];
    q_tail = (q_tail + 1)%Q_SIZE;

    return chunk;
}

void print_chunk(output_chunk_t curr_chunk) {
    for(int k = 0; k < curr_chunk.size; k++) {
        int run = curr_chunk.runs[k];
        char c = curr_chunk.chars[k];

        if (useFwrite) {
            fwrite(&run, sizeof(run), 1, stdout);
            fwrite(&c, sizeof(c), 1, stdout);
        } else {
            printf("%d%c\n", run, c);
        }
    }
}

void computeRLE(input_chunk_t input) {
    output_chunk_t output;
    // 1-1 mapping between character and counts
	char *chars = malloc(input.size * sizeof(char));
    int *runs = malloc(input.size * sizeof(int));
    int idx = 0, i = 0;

    while(i < input.size) {
        char c = input.buf[i];

        if (c == '\0') {
            i++;
            continue;
        }

        int run = 0;

        while(i < input.size) {
            if (input.buf[i] == c) {
                i++;
                run++;
            } else if (input.buf[i] == '\0') {
                i++;
            } else {
                break;
            }
        }

        chars[idx] = c;
        runs[idx] = run;
        idx++;
        // printf("[run:%d]\n", run);
    }   

    // collect info and re-align
    output.size = idx;
    output.chars = chars;
    output.runs = runs;

    // printf("[file:%d] [page:%d] [chunk_size:%d]\n", input.file_id, input.page_id, output.size);

    output_chunks[input.file_id][input.page_id] = output;
}


void* RLE_reader(void *input) {
    char **filenames = (char **)input;
    
    int fid = 0;
    int n = total_files;

    for(int i = 0; i < n; i++) {
        char *filename = filenames[i];
        int fildes = open(filename, O_RDONLY);

        if (fildes == -1) {
            // printf("INVALID file:%s\n", filename);
            total_files--;
            close(fildes);
            continue;
        }

        struct stat sb;

        if (fstat(fildes, &sb) == -1) {
            total_files--;
            close(fildes);
            continue;
        }

        // printf("[file:%s] [size:%ld]\n", filename, sb.st_size);

        int pages = sb.st_size/PAGE_SIZE + (sb.st_size % PAGE_SIZE != 0);
        total_pages += pages;

        // init chunks array for output based on the number of pages
        output_chunks[fid] = (output_chunk_t*)calloc(pages,  sizeof(output_chunk_t));
        pages_count[fid] = pages;

        // TODO: what happens if the file cannot be fit inside the memory?
        char *ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fildes, 0);
        
        for(int page_id = 0; page_id < pages; page_id++) {
            int actual_size = PAGE_SIZE;

            // last page might not be fully filled
            if (page_id == pages - 1) {
                actual_size = sb.st_size % PAGE_SIZE;
            }

            input_chunk_t input_chunk;

            input_chunk.buf = ptr;
            input_chunk.file_id = fid;
            input_chunk.page_id = page_id;
            input_chunk.size = actual_size;

            ptr += actual_size;

            // printf("[filename:%s] [actual_size:%d]\n", filename, actual_size);

            pthread_mutex_lock(&q_lock);

            while(isQueueFull()) {
                pthread_cond_broadcast(&cond_q_fill);
                pthread_cond_wait(&cond_q_empty, &q_lock);
            }

            enqueue(input_chunk);

            pthread_mutex_unlock(&q_lock);
			pthread_cond_signal(&cond_q_fill);
            // pthread_cond_broadcast(&cond_q_fill);

        }

        close(fildes);
        fid++;
    }

    reader_done = 1;
    pthread_cond_broadcast(&cond_q_fill);
    return 0;
}

void* RLE_processor() {

    while(1) {
        pthread_mutex_lock(&q_lock);
        
        // wait for reader
        while(isQueueEmpty() && !reader_done) {
            pthread_cond_signal(&cond_q_empty);
            pthread_cond_wait(&cond_q_fill, &q_lock);
        }

        // upon state variable change figure out what happened?
        if (isQueueEmpty() && reader_done) {
            pthread_mutex_unlock(&q_lock);
            break;
        }

        input_chunk_t chunk = dequeue();

        // no need for lock anymore
        pthread_mutex_unlock(&q_lock);

        if(!reader_done){
		    pthread_cond_signal(&cond_q_empty);
		}

        computeRLE(chunk);
    }

    return 0;
}

void RLE_writer() {

    int last_file_id = total_files - 1;

    for(int fid = 0; fid <= last_file_id; fid++) {
        int last_page = pages_count[fid] - 1;

        for(int page = 0; page <= last_page; page++) {
            output_chunk_t curr_chunk = output_chunks[fid][page];
            output_chunk_t next_chunk;

            // if it is last file and last page then we can't shift count to next page
            if (!(fid == last_file_id && page == last_page)) {
                if (page == last_page) {
                    next_chunk = output_chunks[fid + 1][0];
                } else {
                    next_chunk = output_chunks[fid][page + 1];
                }

                int next_chunk_first_char = next_chunk.chars[0];
                int curr_chunk_last_char = curr_chunk.chars[curr_chunk.size - 1];

                if (curr_chunk_last_char == next_chunk_first_char) {
                    next_chunk.runs[0] += curr_chunk.runs[curr_chunk.size - 1];
                    curr_chunk.size--;
                }
            }

            print_chunk(curr_chunk);
        }
    }
}


void RLE_writer2() {

    int last_file_id = total_files - 1;

    for(int fid = 0; fid <= last_file_id; fid++) {
        char* output = malloc(pages_count[fid] * PAGE_SIZE * (sizeof(int)+sizeof(char)));
        char* ptr = output;

        int last_page = pages_count[fid] - 1;

        for(int page = 0; page <= last_page; page++) {
            output_chunk_t curr_chunk = output_chunks[fid][page];
            output_chunk_t next_chunk;

            // if it is last file and last page then we can't shift count to next page
            if (!(fid == last_file_id && page == last_page)) {
                if (page == last_page) {
                    next_chunk = output_chunks[fid + 1][0];
                } else {
                    next_chunk = output_chunks[fid][page + 1];
                }

                int next_chunk_first_char = next_chunk.chars[0];
                int curr_chunk_last_char = curr_chunk.chars[curr_chunk.size - 1];

                if (curr_chunk_last_char == next_chunk_first_char) {
                    next_chunk.runs[0] += curr_chunk.runs[curr_chunk.size - 1];
                    curr_chunk.size--;
                }
            }

            for(int k = 0; k < curr_chunk.size; k++) {
                int run = curr_chunk.runs[k];
                char c = curr_chunk.chars[k];

                *((int*)ptr) = run;
                ptr += sizeof(run);
                *((char*)ptr) = c;
                ptr += sizeof(c);
            }
        }

        if (useFwrite) {
            fwrite(output, ptr - output, 1,stdout);
        } else {
            printf("%s", output);
        }

        free(output);
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
		exit(1);
    }

    total_files = argc - 1;
    pages_count = (int*)malloc(total_files * sizeof(int));
    output_chunks = (output_chunk_t**)calloc(total_files, sizeof(output_chunk_t*));

    int processors_count = get_nprocs();
    pthread_t reader;
    pthread_t processors[processors_count];

    pthread_create(&reader, NULL, RLE_reader, argv + 1);
    for(int i = 0; i < processors_count; i++) {
        pthread_create(&processors[i], NULL, RLE_processor, NULL);
    }

    pthread_join(reader, NULL);
    for(int i = 0; i < processors_count; i++) {
        pthread_join(processors[i], NULL);
    }

    RLE_writer2();
    
    return 0;
}