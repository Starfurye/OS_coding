#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	char buf[256];
	char* dir = NULL;
	dir = getcwd(buf, sizeof(buf));
	if (dir == NULL) {
		perror("getcwd()");
		exit(EXIT_FAILURE);
	} else {
		printf("%s\n", dir);
	}

	return 0;
}
