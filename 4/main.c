#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "sectormap.h"

void ftl_open();
void ftl_read(int lsn, char *sectorbuf);
void ftl_write(int lsn, char *sectorbuf);
void ftl_print();

FILE *flashfp;

int main(int argc, char *argv[]) {
	char pagebuf[PAGE_SIZE];
	char sectorbuf[SECTOR_SIZE];

	// 파일 생성
	if ((flashfp = fopen("flashdisk", "w+")) == NULL) {
		fprintf(stderr, "flashdisk file open error\n");
		exit(1);
	}

	memset(pagebuf, 0xff, sizeof(pagebuf));

	for (int i = 0; i < BLOCKS_PER_DEVICE; i++) {
		for (int j = 0; j < PAGES_PER_BLOCK; j++) {
			fwrite(pagebuf, sizeof(pagebuf), 1, flashfp);
		}
	}

	ftl_open();

	for (int i = 0; i < PAGES_PER_BLOCK * DATABLKS_PER_DEVICE; i++) {
		char buf[SECTOR_SIZE];
		memset(buf, 0xff, sizeof(buf));
		buf[0] = i % ('z' - 'a') + 'a';
		ftl_write(i, buf);
	}

	while (1) {
		printf(">>> ");
		char op = getchar();

		if (op == 'p') {
			ftl_print();
			getchar();
			continue;
		}

		int lsn;
		char buffer[SECTOR_SIZE];
		memset(buffer, 0xff, sizeof(buffer));
		if (op == 'r') {
			scanf("%d", &lsn);
			ftl_read(lsn, buffer);
			for (int i = 0; i < sizeof(buffer); i++) {
				if (buffer[i] == -1)
					break;
				printf("%c", buffer[i]);
			}
		}
		else if (op =='w') {
			scanf("%d", &lsn);
			scanf("%s", buffer);
			ftl_write(lsn, buffer);
		}
		else {
			break;
		}
		getchar();
		/*
		printf("\n\n");
		ftl_print();
		*/
	}
	exit(0);
}
