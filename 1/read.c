#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


#define BUF_LEN 10

int min(int a, int b) {
	if (a < b)
		return a;
	return b;
}

int main(int argc, char* argv[]) {
	char* filename;
	int offset;
	int numOfReadingBytes;
	int fd;
	char buffer[BUF_LEN];
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
	if (numOfReadingBytes <= 0) {
		fprintf(stderr, "Reading bytes must be positive number\n");
		return 1;
	}

	lseek(fd, offset, SEEK_SET);
	while (numOfReadingBytes > 0) {
		int size = min(numOfReadingBytes, sizeof(buffer) - 1);
		int n;
		if ((n =read(fd, buffer, size)) < 0) {
			fprintf(stderr, "Fail to read file\n");
			return 1;
		} else if (n == 0) {
			break;
		}
		numOfReadingBytes -=size;
		buffer[size] = '\0';
		printf("%s", buffer);
	}
	return 0;
}
