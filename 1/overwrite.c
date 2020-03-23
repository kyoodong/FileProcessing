#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
	int offset;
	char* filename;
	char* data;
	int fd;
	int length;

	if (argc != 4) {
		fprintf(stderr, "It need 3 arguments.\n");
		return 1;
	}
	filename = argv[1];
	offset = atoi(argv[2]);
	data = argv[3];

	if ((fd = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename);
		return 1;
	}

	lseek(fd, offset, SEEK_SET);
	length = strlen(data);
	if (write(fd, data, length) < length) {
		fprintf(stderr, "Write error occured!\n");
		return 1;
	}
	return 0;
}
