#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

char *EXIT = "exit";
char *LOOP = "loop";
char *$LOOP = "$loop";

int isNum(char* s) {
    int i;
    for(i = 0; s[i]; i++) {
        if (!isdigit(s[i])) {
            return 0;
        }
    }

    return 1;
}

char* substituteLoopVariable(char *cmd, int i) {
    char *subCmd = malloc(200 * sizeof(char));
    // clear the memory as we don't know what is store there
    memset(subCmd, 0, 200);

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

void parseAndExec(char *cmd) {
    printf("Command is:%s\n", cmd);

    char *token = NULL;
    int loopCount = 0;

    // 1. check for the "loop" command
    token = strsep(&cmd, " ");

    if (token == NULL || strcmp(token, LOOP) != 0) {
        free(token);
        free(cmd);
        return;
    }

    // 2. check for the loop counter value
    token = strsep(&cmd, " ");

    if (!isNum(token)) {
        free(cmd);
        return;
    }

    loopCount = atoi(token);

    // replace the $loop va
    int i = 0;
    char *cmdCopy = NULL;

    for(i = 0; i < loopCount; i++) {
        // replace the $loop variable if it exist
        // printf("sub for %d\n", i);
        cmdCopy = substituteLoopVariable(strdup(cmd), i);

        printf("original:%s, sub:%s\n", cmd, cmdCopy);

        // execute the command
        exec(cmdCopy);

        free(cmdCopy);
    }

}

void interactiveMode() {
    puts("Interactive mode");
    char *cmd = NULL;
    size_t len = 100;
    ssize_t cmdlen;

    while(1) {
        printf("wish> ");

        cmdlen = getline(&cmd, &len, stdin);

        if (cmd[cmdlen - 1] == '\n') {
            cmd[cmdlen - 1] = '\0';
        }

        if (strcmp(cmd, EXIT) == 0) {
            break;
        }

        parseAndExec(strdup(cmd));
    }

    // TODO: free any allocation
    free(cmd);
}

void batchMode(char *filename) {
    puts("batch mode");

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