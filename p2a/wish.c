#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>

char *EXIT = "exit";
char *CD = "cd";
char *PATH = "path";
char *LOOP = "loop";

char *$LOOP = "$loop";
char *GLOBAL_PATHS[100];
int PATHS_LEN = 0;

int isNum(char* s) {
    int i;
    for(i = 0; s[i]; i++) {
        if (!isdigit(s[i])) {
            return 0;
        }
    }

    return 1;
}

void exec(char *cmdStr) {
    int pid = fork();

    if (pid < 0) { // error
        // TODO: handle error
        exit(1);
    } else if (pid != 0) { // parent
        wait(NULL);
    } else { // child
        // TODO: redirection
        // loop 2 echo hello world $loop
        char *args[3] = {NULL, NULL, NULL};

        char *token = strsep(&cmdStr, " ");
        // 1. get command and put it into the first slot
        if (token == NULL) {
            exit(1);
        } else {
            args[0] = strdup(token);
        }

        // 2. copy rest of the part only if it's not null
        if (cmdStr != NULL) {
            args[1] = strdup(cmdStr);
        }

        char tryPath[200];

        int i = 0;
        for(i = 0; i < PATHS_LEN; i++) {
            tryPath[0] = '\0';

            strcat(tryPath, GLOBAL_PATHS[i]);
            strcat(tryPath, "/");
            strcat(tryPath, args[0]);

            if (access(tryPath, X_OK) == 0) {
                // printf("found at: %d\n", i);
                // puts(fullPath);
                break;
            }
        }

        if (i != PATHS_LEN) {
            args[0] = strdup(tryPath);
        }

        if (execv(args[0], args) == -1) {
            // TODO: handle error
            puts("error: invalid command");
        }
    }
}

char* substituteLoopVariable(char *cmd, int i) {
    char *subCmd = malloc(200 * sizeof(char));
    // clear the memory as we don't know what is store there
    subCmd[0] = '\0';

    char *token = NULL;
    char val[20];
    int count = 0;

    while((token=strsep(&cmd, " ")) != NULL) {
        if (count != 0) {
            strcat(subCmd, " ");         
        }

        count++;

        if (strcmp(token, $LOOP) == 0) {
            sprintf(val, "%d", i);
            strcat(subCmd, val);
        } else {
            strcat(subCmd, token);
        }
    }

    free(token);

    return subCmd;
}

void cdExit(char *cmd) {
    if (cmd != NULL) {
        // TODO: handle error
        return;
    }
}

void cdCmd(char *cmd) {
    char *dirPath = strsep(&cmd, " ");

    if (dirPath == NULL || cmd != NULL) {
        // TODO: handle error
        // printf("Path is missing or there are more variables\n");
        // printf("path:%s, cmd:%s\n", path, cmd);
        return;
    }

    if (chdir(dirPath) != 0) {
        // TODO: handle error
        return;
    }
}

void pathCmd(char *cmd) {
    int i = 0;

    while((GLOBAL_PATHS[i] = strsep(&cmd, " ")) != NULL) {
        i++;
    }

    PATHS_LEN = i;
}

void loopCmd(char *cmd) {
    char *token = NULL;

    // 1. check for the loop counter value
    token = strsep(&cmd, " ");

    if (!isNum(token)) {
        free(cmd);
        return;
    }

    int loopCount = atoi(token);

    // 2. replace the $loop var
    int i = 0;
    char *cmdCopy = NULL;

    for(i = 0; i < loopCount; i++) {
        // replace the $loop variable if it exist
        // printf("sub for %d\n", i);
        cmdCopy = substituteLoopVariable(strdup(cmd), i);

        // printf("original:%s, sub:%s\n", cmd, cmdCopy);

        // execute the command
        exec(strdup(cmdCopy));

        free(cmdCopy);
    }
}

void parseAndExec(char *cmd) {
    // printf("Command is:%s\n", cmd);

    char *token = strsep(&cmd, " ");

    if (token == NULL) {
        // TODO: handle error
        free(token);
        free(cmd);
        return;
    }

    if (strcmp(token, EXIT) == 0) {
        cdExit(cmd);
        exit(0);
    }

    if (strcmp(token, CD) == 0) {
        cdCmd(cmd);
        return;
    }

    if (strcmp(token, PATH) == 0) {
        pathCmd(cmd);
        return;
    }

    if (strcmp(token, LOOP) == 0) {
        loopCmd(cmd);
        return;
    }

    // TODO: any other command throw error
}

void interactiveMode() {
    // puts("Interactive mode");
    char *cmd = NULL;
    size_t len = 100;
    ssize_t cmdlen;

    while(1) {
        printf("wish> ");

        cmdlen = getline(&cmd, &len, stdin);

        if (cmd[cmdlen - 1] == '\n') {
            cmd[cmdlen - 1] = '\0';
        }

        parseAndExec(strdup(cmd));
    }

    // TODO: free any allocation
    free(cmd);
}

void batchMode(char *filename) {
    // puts("batch mode");

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        puts("no file found\n");
        fclose(fp);
        return;
    }

    char *linebuff = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&linebuff, &linecap, fp)) > 0) {
        // neat trick to avoid \n last character which getline() doesn't omit
        if (linebuff[linelen - 1] == '\n') {
            linebuff[linelen - 1] = '\0';
        }

        parseAndExec(strdup(linebuff));
    }

    // TODO: free any allocation
    free(linebuff);
    fclose(fp);
}

// TODO: merge 2 input functionality common pieces?
int main(int argc, char *argv[]) {
    GLOBAL_PATHS[0] = "/bin";
    // we just accept one parameter
    switch (argc)
    {
        case 1:
            interactiveMode();
            break;
        case 2:
            batchMode(argv[1]);
            break;
        default:
            // TODO: print something for invalid command
            break;
    }

    return 0;
}