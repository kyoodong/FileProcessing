#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	char* fname = "hi.txt";
	char buffer[1024];
	for (char a = 'a'; a <= 'z'; a++) {
		buffer[a - 'a'] = a;
	}
	int fd = open(fname, O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	printf("%s\n", buffer);
	printf("%d\n", EOF);
	return 0;
}
