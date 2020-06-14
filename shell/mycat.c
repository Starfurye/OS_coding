#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 10240

int main(int argc, char* argv[])
{
	int fd, i = 0, count;
	char buffer[BUFFER_SIZE];

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open()");
		exit(EXIT_FAILURE);
	}
	count = read(fd, &buffer, BUFFER_SIZE);
	buffer[count] = '\0';
	printf("%s", buffer);
	while (count != 0) {
		count = read(fd, &buffer, BUFFER_SIZE);
		buffer[count] = '\0';
		printf("%s", buffer);
	}
	close(fd);

	return 0;
}
