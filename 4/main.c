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
	if ((flashfp = fopen("flashdisk", "w")) == NULL) {
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
	ftl_print();
	exit(0);
}
