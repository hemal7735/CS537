#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>

void readFile(char *filename) {
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

    puts("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
		exit(1);
    }

    readFile(argv[1]);
    
    return 0;
}