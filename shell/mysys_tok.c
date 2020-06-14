#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int mysys(char *command) {
	char *argv[100] = {0}, buffer[100] = {0};
	char *token, *bp = buffer;
	int i = 0;
	strcpy(buffer, command);
	token = strtok(bp, " ");
	while (token != NULL) {
		argv[i++] = token;
		token = strtok(NULL, " ");
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
