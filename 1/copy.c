#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char readFilename[512], writeFilename[512];
	char buffer[110];
	int readfd, writefd;
	int size;

	if (argc != 3) {
		fprintf(stderr, "Number of arguments must be 3.\n");
		return 1;
	}

	memset(buffer, 0, sizeof(buffer));
	strcpy(readFilename, argv[1]);

	if ((readfd = open(readFilename, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot open %s\n", readFilename);
		return 1;
	}

	strcpy(writeFilename, argv[2]);
	if ((writefd = open(writeFilename, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
		fprintf(stderr, "Cannot open %s\n", writeFilename);
		return 1;
	}

	while ((size = read(readfd, buffer, 100)) > 0) {
		if (write(writefd, buffer, size) < size) {
			fprintf(stderr, "Error occured while copying!\n");
			return 1;
		}
	}
	return 0;
}
