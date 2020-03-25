#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_LEN 1024

int min(int a, int b) {
	if (a < b)
		return a;
	return b;
}

int main(int argc, char* argv[]) {
	int fd;
	char* filename;
	char* data;
	int offset;
	int length;
	off_t fileSize, position, restSize, readingSize;
	char buffer[BUF_LEN];
	

	if (argc != 4) {
		fprintf(stderr, "It need 3 arguments\n");
		return 1;
	}

	filename = argv[1];
	offset = atoi(argv[2]);
	data = argv[3];
	
	if ((fd = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename);
		return 1;
	}

	length = strlen(data);
	if ((fileSize = lseek(fd, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error ocurred\n");
		return 1;
	}
	if ((position = lseek(fd, offset, SEEK_SET)) < 0) {
		fprintf(stderr, "lseek error ocurred\n");
		return 1;
	}
	if (fileSize <= position) {
		write(fd, data, length);
		return 0;
	}
	
	if ((position = lseek(fd, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error ocurred\n");
		return 1;
	}

	restSize = position - offset;
	while (restSize > 0) {
		readingSize = min(restSize, BUF_LEN);
		if (lseek(fd, -readingSize, SEEK_CUR) < 0) {
			fprintf(stderr, "lseek error ocurred\n");
			return 1;
		}
		if (read(fd, buffer, readingSize) != readingSize) {
			fprintf(stderr, "Reading error occured\n");
			return 1;
		}
		if (lseek(fd, length - readingSize, SEEK_CUR) < 0) {
			fprintf(stderr, "lseek error occured\n");
			return 1;
		}
		if (write(fd, buffer, readingSize) != readingSize) {
			fprintf(stderr, "Write error occured\n");
			return 1;
		}
		if (lseek(fd, -readingSize - length, SEEK_CUR) < 0) {
			fprintf(stderr, "lseek error ocurred\n");
			return 1;
		}
		restSize -= readingSize;
	}
	
	if (write(fd, data, length) != length) {
		fprintf(stderr, "Write error occured\n");
		return 1;
	}
	return 0;
}
