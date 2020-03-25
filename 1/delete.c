#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_LEN 100

int min(int a, int b) {
	if (a < b)
		return a;
	return b;
}

int main(int argc, char* argv[]) {
	char* filename;
	int offset;
	int deletingBytes;
	int fd;
	int size;
	off_t fileSize, position, restSize;
	char buffer[BUF_LEN];

	if (argc != 4) {
		fprintf(stderr, "It need 3 arguments\n");
		return 1;
	}

	filename = argv[1];
	offset = atoi(argv[2]);
	deletingBytes = atoi(argv[3]);

	if ((fd = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "Cannot open %s\n", filename);
		return 1;
	}

	if ((fileSize = lseek(fd, 0, SEEK_END)) < 0) {
		fprintf(stderr, "lseek error occured\n");
		return 1;
	}

	if (offset >= fileSize)
		return 0;

	if (offset + deletingBytes >= fileSize) {
		if (ftruncate(fd, offset) < 0) {
			fprintf(stderr, "ftruncate error occurred\n");
			return 1;
		}
		return 0;
	}

	if ((position = lseek(fd, offset + deletingBytes, SEEK_SET)) < 0) {
		fprintf(stderr, "lseek error occurred\n");
		return 1;
	}

	restSize = fileSize - position;
	while (restSize > 0) {
		size = min(BUF_LEN, restSize);
		if (read(fd, buffer, size) != size) {
			fprintf(stderr, "read error occurred\n");
			return 1;
		}
		restSize -= size;
		if (lseek(fd, -size - deletingBytes, SEEK_CUR) < 0) {
			fprintf(stderr, "lseek error occurred\n");
			return 1;
		}
		if (write(fd, buffer, size) != size) {
			fprintf(stderr, "write error occurred\n");
			return 1;
		}
		if (lseek(fd, -restSize, SEEK_END) < 0) {
			fprintf(stderr, "lseek error occurred\n");
			return 1;
		}
	}
	
	if (ftruncate(fd, fileSize - deletingBytes) < 0) {
		fprintf(stderr, "ftruncate error occurred\n");
		return 1;
	}
	return  0;
}
