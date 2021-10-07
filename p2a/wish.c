#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

char *EXIT = "exit";

void execute(char *cmd) {
    printf("Command is:%s\n", cmd);


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

        execute(strdup(cmd));
    }

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

        execute(strdup(linebuff));
    }

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