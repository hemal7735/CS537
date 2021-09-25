#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>

struct Node {
    int key;
    char *value;
    struct Node *next;
};

struct Node *hashmap[(int)1e5 + 7];

char DB_FILE_NAME[] = "database.txt";
char PUT[] = "p";
char GET[] = "g";
char DELETE[] = "d";
char CLEAR[] = "c";
char ALL[] = "a";

int getSize() {
    return sizeof(hashmap)/sizeof(struct Node *);
}

int getSlot(int key) {
    return key%getSize();
}

int isNum(char* s) {
    int i;
    for(i = 0; s[i]; i++) {
        if (!isdigit(s[i])) {
            return 0;
        }
    }

    return 1;
}

struct Node* lookup(int key) {
    int slot = getSlot(key);

    struct Node* curr = hashmap[slot];

    while(curr != NULL) {
        if (curr->key == key) {
            return curr;
        }

        curr = curr->next;
    }

    return curr;
}

void putInSlot(int key, char *value) {
    int slot = getSlot(key);
    struct Node *newnode = malloc(sizeof(struct Node));
    newnode->key = key;
    newnode->value = value;
    newnode->next = NULL;

    if (hashmap[slot] == NULL) {
        hashmap[slot] = newnode;
    } else {
        struct Node *node = lookup(key);

        if (node == NULL) {
            newnode->next = hashmap[slot];
            hashmap[slot] = newnode;
        } else {
            node->value = value;
        }
    }
}

void put(char *cmd) {
    char *token, *value;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL || !isNum(token)) {
        puts("bad command");
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    
    // 2. grab value
    value = strsep(&cmd, ",");
    if (value == NULL) {
        puts("bad command");
        return;
    }

    // 3. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        puts("bad command");
        return;
    }

    putInSlot(key, strdup(value));
}

void get(char *cmd) {
    char *token;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL || !isNum(token)) {
        puts("bad command");
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    // TODO: find value and print

    // 2. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        puts("bad command");
        return;
    }

    struct Node* node = lookup(key);

    if (node == NULL) {
        printf("%d not found\n", key);
    } else {
        printf("%d,%s\n", key, node->value);
    }
}

void delete(char *cmd) {
    char *token;
    // 1. grab key
    token = strsep(&cmd, ",");
    if (token == NULL || !isNum(token)) {
        puts("bad command");
        return;
    }

    // TODO: atoi is not safe, find better function?
    int key = atoi(token);
    
    // TODO: find value and delete it

    // 2. check if there is any more token. it should not ideally
    token = strsep(&cmd, ",");
    if (token != NULL) {
        puts("bad command");
        return;
    }

    int slot = getSlot(key);

    struct Node *curr = hashmap[slot], *prev;

    if (curr == NULL) {
        printf("%d not found\n", key);
        return;
    }

    // special case where the first key itself is the one to delete
    if (curr != NULL && curr->key == key) {
        hashmap[slot] = curr->next;
        // free(curr->value);
        // free(curr);
    } else {
        while(curr->next != NULL && curr->key != key) {
            prev = curr;
            curr = curr->next;
        }

        if (curr->key == key) {
            prev->next = curr->next;
            // free(curr->value);
            // free(curr);
        } else {
            printf("%d not found\n", key);
        }
    }
}

void clear(char *cmd) {
    // check if there is any more token. it should not ideally
    char *token = strsep(&cmd, ",");
    if (token != NULL) {
        puts("bad command");
        return;
    }

    memset(hashmap, 0, sizeof hashmap);
}

void all(char *cmd) {
     // check if there is any more token. it should not ideally
    char *token = strsep(&cmd, ",");
    if (token != NULL) {
        puts("bad command");
        return;
    }

    int size = getSize();
    int i;
    struct Node *head;

    int isEmpty = 1;

    for(i = 0; i < size; i++) {
        // TODO: get the first node and then iterate until NULL
        head = hashmap[i];

        while (head != NULL) {
            isEmpty = 0;
            printf("%d,%s\n", head->key, head->value);
            head = head->next;
        }

    }

    if (isEmpty) {
        // puts("Oops, seems like DB is empty!");
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
        puts("bad command");
    }
}

// reads a file from the disk and convert that to data-structure
void load() {
    FILE *fp = fopen(DB_FILE_NAME, "r"); 
    // no file found. Initialize the data structure manually
    if (fp == NULL) {
        // printf("No file found\n");
        return;
    } else {
        // read the file and parse the data
        char *linebuff = NULL, *value = NULL;
        size_t linecap = 0;
        ssize_t linelen;
        int key = 0, len;

        while ((linelen = getline(&linebuff, &linecap, fp)) > 0) {
            // neat trick to avoid \n last character which getline() doesn't omit
            if (linebuff[linelen - 1] == '\n') {
                linebuff[linelen - 1] = '\0';
            }

            // first token is key - integer
            // atoi is safe to use here as we know what we inserted into the DB
            key = atoi(strsep(&linebuff, ","));
            
            // second token is value - string
            value = strsep(&linebuff, ",");

            putInSlot(key, value);
        }

        // TODO: free value?
        free(linebuff);
        fclose(fp);
    }
}

// transform data-structure to the external file on the disk
void persist() {
    FILE *fp = fopen(DB_FILE_NAME, "w");

    int size = getSize();
    int i;
    struct Node *head;

    for(i = 0; i < size; i++) {
        // TODO: get the first node and then iterate until NULL
        head = hashmap[i];

        while (head != NULL) {
            fprintf(fp, "%d,%s\n", head->key, head->value);
            head = head->next;
        }
    }
   
    fclose(fp);
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        return 0;
    }

    // printf("total operations: %d\n", argc - 1);

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