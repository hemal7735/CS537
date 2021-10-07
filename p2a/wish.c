#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

char *EXIT = "exit";

void execute(char *cmd) {

}

void interactiveMode() {
    puts("Interactive mode");
    char input[1024];

    while(1) {
        printf("wish> ");

        gets(input);

        if (strcmp(input, EXIT) == 0) {
            break;
        }

        // break;
    }
}

void batchMode(char *filePath) {
    puts("batch mode");
}

int main(int argc, char *argv[]) {
    // we just accept one parameter
    switch (argc)
    {
        case 1:
            interactiveMode();
            break;
        case 2:
            batchMode(argv[1]);
            /* code */
            break;
        default:
            break;
    }

    return 0;
}