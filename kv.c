#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char DB_FILE_NAME[] = "database.txt";
char PUT[] = "p";
char GET[] = "g";
char DELETE[] = "d";
char CLEAR[] = "c";
char ALL[] = "a";

// reads a file from the disk and convert that to data-structure
void load() {
    FILE *fp = fopen(DB_FILE_NAME, "r"); 

    // no file found. Initialize the data structure manually
    if (fp == NULL) {
        printf("No file found\n");
    } else {
        // read the file and parse the data
        char *linebuff = NULL, *value = NULL;
        size_t linecap = 0;
        ssize_t linelen;
        int key = 0;

        while ((linelen = getline(&linebuff, &linecap, fp)) > 0) {
            // first token is key - integer
            // atoi is safe to use here as we know what we inserted into the DB
            key = atoi(strsep(&linebuff, ","));
            
            // second token is value - string
            value = strsep(&linebuff, ",");

            // TODO: add data in the DS
            // printf("key: %d, value:%s\n", key, value);
        }

        // TODO: free value?
        free(linebuff);
    }

    fclose(fp);
}

// transform data-structure to the external file on the disk
void persist() {
    FILE *fp = fopen(DB_FILE_NAME, "w");

    fputs("ssup1", fp);
   
    fclose(fp);
}

void put(char *cmd) {
    char *token;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL) {
        // TODO: error
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    
    // 2. grab value
    token = strsep(&cmd, ",");
    if (token == NULL) {
        // TODO: error
        return;
    }

    printf("key:%d , value: %s\n", key, token);

    // 3. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        // TODO: error
        return;
    }
}

void get(char *cmd) {
    char *token;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL) {
        // TODO: error
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    printf("searching by the key:%d\n", key);

    // TODO: find value and print

    // 2. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        // TODO: error
        return;
    }
}

void delete(char *cmd) {
    char *token;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL) {
        // TODO: error
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    
    // TODO: find value and delete it

    // 2. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        // TODO: error
        return;
    }
}

void clear(char *cmd) {
    // check if there is any more token. it should not ideally
    char *token = strsep(&cmd, ",");
    if (token != NULL) {
        // TODO: error
        return;
    }
}

void all(char *cmd) {
     // check if there is any more token. it should not ideally
    char *token = strsep(&cmd, ",");
    if (token != NULL) {
        // TODO: error
        return;
    }


}

// prints error if the operation cannot be executed
// routes to the proper function where further verification can be done
void executeCmd(char *cmd) {
    // printf("Verifying cmd: %s\n", cmd);
    // no need to verify for empty or NULL string at this point
    // as the prior checks should have taken care of that
    char* op = strsep(&cmd, ",");

    if (strcmp(op, PUT) == 0) {
        put(cmd);
    } else if (strcmp(op, GET) == 0) {
        get(cmd);
    } else if (strcmp(op, DELETE) == 0) {
        delete(cmd);
    } else if (strcmp(op, CLEAR) == 0) {
        clear(cmd);
    } else if (strcmp(op, ALL) == 0) {
        all(cmd);
    } else {
        printf("ERROR: Unsupported command found. Actual command was: %s\n", cmd);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("You need to pass at least one argument!\n");
        return -1;
    }

    printf("total operations: %d\n", argc - 1);

    // TODO: early or lazy load DB?
    load();

    int i;
    for(i = 1; i < argc; i++) {
        executeCmd(argv[i]);
    }

    // TODO: persist only if there was a modification?
    persist();

    return 0;
}