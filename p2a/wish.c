#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>

char *EXIT = "exit";
char *CD = "cd";
char *PATH = "path";
char *LOOP = "loop";

char *$LOOP = "$loop";
char *GLOBAL_PATHS[100];
int PATHS_LEN = 1;

char errorMsg[30] = "An error has occurred\n";

int isEmpty(char *s) {
    if (s == NULL || strcmp(s, "") == 0) return 1;
    return 0;
}

void safeFree(char *s) {
    if (s != NULL) {
        free(s);
    }
}

void safeClose(FILE *fp) {
    if (fp != NULL) {
        fclose(fp);
    }
}

void handleError() {
    write(STDERR_FILENO, errorMsg, strlen(errorMsg)); 
}

char *trim(char *s) {
    if (s == NULL) return s;

    char *end;

    while(*s != 0 && isspace((unsigned char)*s)) {
        s++;
    }

    if(*s == 0)
        return s;

    end = s + strlen(s) - 1;

    while(end > s && isspace((unsigned char)*end)) {
        end--;
    }
    // terminate string
    end[1] = '\0';

    return s;
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


// should follow the below guidelines
//          1. only one file
//          2. should be only 1 >
//          3. should be command before >
// returns length of parts
//          -1: if invalid
//          0: if there is no >
//          2: if valid (some command > filename)
int parseForIndirection(char **args, char *cmd) {
    // base condition
    if (cmd == NULL) return 0;

    int i = 0;
    char *token = NULL;

    while((token = trim(strsep(&cmd, ">"))) != NULL) {
        args[i] = token;
        i++;
    }

    if (i <= 1) return 0; // no > found
    if (i > 2) return -1; // more than 1 > found

    // first part cannot be empty
    if (isEmpty(args[0])) return -1;

    // if there are only 2 tokens then we need to check if there are multiple files present
    char *outputPart = strdup(args[i - 1]);

    token = trim(strsep(&outputPart, " "));

    // there should be atleast one filename
    if (isEmpty(token)) return -1;

    token = trim(strsep(&outputPart, " "));
    // it cannot have more than 1 filenames
    if (!isEmpty(token)) return -1;

    return i;
}

void nativeCmd(char *cmdStr) {
    int pid = fork();

    if (pid < 0) { // error
        // TODO: handle error
        handleError();
        exit(1);
    } else if (pid != 0) {
        wait(NULL);
    } else { // child
        char *indirectionArgs[50];

        int len = parseForIndirection(indirectionArgs, strdup(cmdStr));
        if (len == -1) {
            // TODO: remove this
            // printf("invalid as per > stand. len: %d\n", len);
            // puts("error with >");
            handleError();
            return;
        } else {
            // TODO: remove this
            // printf("valid. len: %d\n", len);
        }

        // override the original command with anything before >
        if (len == 2) {
            cmdStr = indirectionArgs[0];
            close(STDOUT_FILENO);
            if (open(indirectionArgs[1], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU) < 0) {
                // puts("error with redirection");
                handleError();
                return;
            }
        }

        char *args[100];
        memset(args, 0, sizeof args);

        char *token = NULL;
        int i = 0;

        while((token = trim(strsep(&cmdStr, " "))) != NULL) {
            args[i] = token;
            i++;
        }

        // get correct path
        char tryPath[200];

        i = 0;
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

        // TODO: remove this
        // printf("tried path is:%s\n", tryPath);

        char *cmdPath = args[0];

        if (i != PATHS_LEN) {
            cmdPath = strdup(tryPath);
        }

        // puts(args[0]);
        // puts(args[1]);
        // args[0] = "/bin/rm";
        // args[1] = "-rf";
        // args[2] = "./abc";
        // args[3] = NULL;

        // char cwd[1000];

        // if (getcwd(cwd, 1000) != NULL) {
        //     printf("Current working dir: %s\n", cwd);
        // } else {
        //     puts("cwd error");
        //     return;
        // }
        // puts(args[0]);
        // puts(args[1]);
        // puts(args[2]);
        // puts(args[3]);
        if (execv(cmdPath, args) == -1) {
            // TODO: remove this
            // puts("could not exec command");
            // puts("error with execv");
            handleError();
            exit(0);
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

    // TODO: free
    // free(token);

    return subCmd;
}

void exitCmd(char *cmd) {
    if (cmd != NULL) {
        // puts("error with exit");
        handleError();
    }
    exit(0);
}

void cdCmd(char *cmd) {
    char *dirPath = strsep(&cmd, " ");
    if (isEmpty(dirPath)|| !isEmpty(cmd)) {
        // TODO: handle error
        // printf("Path is missing or there are more variables\n");
        // printf("path:%s, cmd:%s\n", path, cmd);
        // puts("error with path");
        handleError();
        return;
    }

    if (chdir(dirPath) != 0) {
        // TODO: handle error
        // puts("error with path");
        handleError();
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

    if (isEmpty(token) || !isNum(token)) {
        handleError();
        // TODO: free
        // free(cmd);
        return;
    }

    int loopCount = atoi(token);

    if (loopCount < 0) {
        handleError();
        return;
    }

    // 2. replace the $loop var
    int i = 0;
    char *cmdCopy = NULL;

    for(i = 1; i <= loopCount; i++) {
        // replace the $loop variable if it exist
        // printf("sub for %d\n", i);
        cmdCopy = substituteLoopVariable(strdup(cmd), i);

        // printf("original:%s, sub:%s\n", cmd, cmdCopy);

        // execute the command
        nativeCmd(strdup(cmdCopy));

        // TODO: free
        // free(cmdCopy);
    }
}

void parseAndExec(char *cmd) {
    if (isEmpty(cmd)) return;

    // TODO: remove this
    // printf("Command is:%s\n", cmd);

    char *token = trim(strsep(&cmd, " "));

    if (token == NULL) {
        // puts("null cmd");
        // puts("error with parseAndExec");
        handleError();
        // TODO: free
        // free(token);
        // free(cmd);
        return;
    }

    cmd = trim(cmd);

    if (strcmp(token, EXIT) == 0) {
        exitCmd(cmd);
        return;
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

    // we need to copy the command
    char command[200] = "";

    strcat(command, token);

    if (cmd != NULL) {
        strcat(command, " ");
        strcat(command, cmd);
    }

    // printf("native comamnd:%s\n", command);
    nativeCmd((char *)&command);
}

void interactiveMode() {
    char *cmd = NULL;
    size_t len = 100;
    ssize_t cmdlen;

    while(1) {
        printf("wish> ");

        cmdlen = getline(&cmd, &len, stdin);

        if (cmd[cmdlen - 1] == '\n') {
            cmd[cmdlen - 1] = '\0';
        }

        parseAndExec(strdup(trim(cmd)));

        // TODO: verify
    }
}

void batchMode(char *filename) {
    // checking bad file
    if (access(filename, F_OK | R_OK) != 0) {
        handleError();
        exit(1);
    }

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        fclose(fp);
        handleError();
        exit(1);
    }

    char *linebuff = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&linebuff, &linecap, fp)) > 0) {
        // neat trick to avoid \n last character which getline() doesn't omit
        if (linebuff[linelen - 1] == '\n') {
            linebuff[linelen - 1] = '\0';
        }

        // TODO: remove this
        if (linebuff[0] == '#') continue;

        parseAndExec(strdup(trim(linebuff)));
    }

    safeClose(fp);
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
            handleError();
            exit(1);
            break;
    }

    return 0;
}