#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


#define LIMIT 1024 * 1024

int main(int argc, char* argv[]) {
	char* filename;
	int offset;
	int numOfReadingBytes;
	int fd;
	char buffer[LIMIT];
	memset(buffer, 0, sizeof(buffer));

	if (argc != 4) {
		fprintf(stderr, "It need 3 arguments\n");
		return 1;
	}

	filename = argv[1];
	if ((fd = open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename);
		return 1;
	}

	offset = atoi(argv[2]);
	numOfReadingBytes = atoi(argv[3]);
	if (numOfReadingBytes >= LIMIT) {
		fprintf(stderr, "Reading bytes limitation is %d bytes.\n", LIMIT - 1);
		return 1;
	}

	lseek(fd, offset, SEEK_SET);
	if (read(fd, buffer, numOfReadingBytes) < 0) {
		fprintf(stderr, "Fail to read file\n");
		return 1;
	}
	printf("%s", buffer);
	return 0;
}
