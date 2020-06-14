#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 10240

void Quit(char* err) {
	printf("%s\n", err);
	exit(EXIT_FAILURE);
} 
int main(int argc, char* argv[])
{
	int fdr, fdw, count;
	char buffer[BUFFER_SIZE];
	mode_t mode = 0777;

	memset(buffer, 0, sizeof(BUFFER_SIZE));
	fdr = open(argv[1], O_RDONLY);
	if (fdr < 0) {
		Quit("read()");
	}

	fdw = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, mode);
    if (fdw < 0) {
		Quit("open()");
	}	

	count = read(fdr, &buffer, BUFFER_SIZE);
	write(fdw, &buffer, BUFFER_SIZE);
	while (count != 0) {
		memset(buffer, 0, sizeof(buffer));
		count = read(fdr, &buffer, BUFFER_SIZE);
		write(fdw, &buffer, BUFFER_SIZE);
	}
	close(fdr); 
	close(fdw);

	return 0;
}
