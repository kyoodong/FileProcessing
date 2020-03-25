#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char* filename1;
	char* filename2;
	int fd1, fd2;
	int size;
	char buffer[1024];

	if (argc != 3) {
		fprintf(stderr, "It need 2 arguments.\n");
		return 1;
	}

	filename1 = argv[1];
	filename2 = argv[2];

	if ((fd1 = open(filename1, O_RDWR | O_APPEND)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename1);
		return 1;
	}

	if ((fd2 = open(filename2, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename2);
		return 1;
	}

	while ((size = read(fd2, buffer, 1024)) > 0) {
		if (write(fd1, buffer, size) < size) {
			fprintf(stderr, "Write error\n");
			return 1;
		}
	}
	return 0;
}
