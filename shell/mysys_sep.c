#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void mysys(char *command) {
        char *argv[100] = {0};
        char buffer[100] = {0};
        char *token, *p;
        int i = 0;

        strcpy(buffer, command);
        p = buffer;
        token = strsep(&p, " ");
        while (token != NULL) {
                argv[i++] = token;
                token = strsep(&p, " ");
        }
        argv[i] = NULL;

        int pid = fork();
        if (pid == 0) {
                int error = execvp(argv[0], argv);      
                if (error < 0) perror("execvp()");
        }
        wait(NULL);
}

int main()
{
        printf("--------------------------------------------------\n");

    mysys("echo HELLO WORLD");

    printf("--------------------------------------------------\n");

    mysys("ls /");

    printf("--------------------------------------------------\n");

    return 0;
}
