#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "flash.h"
// 필요한 경우 헤더파일을 추가한다

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

FILE *flashfp;	// fdevicedriver.c에서 사용

//
// 이 함수는 FTL의 역할 중 일부분을 수행하는데 물리적인 저장장치 flash memory에 Flash device driver를 이용하여 데이터를
// 읽고 쓰거나 블록을 소거하는 일을 한다 (동영상 강의를 참조).
// flash memory에 데이터를 읽고 쓰거나 소거하기 위해서 fdevicedriver.c에서 제공하는 인터페이스를
// 호출하면 된다. 이때 해당되는 인터페이스를 호출할 때 연산의 단위를 정확히 사용해야 한다.
// 읽기와 쓰기는 페이지 단위이며 소거는 블록 단위이다.
// 
int main(int argc, char *argv[])
{	
	char sectorbuf[SECTOR_SIZE];
	char sparebuf[SPARE_SIZE];
	char pagebuf[PAGE_SIZE];
	char *blockbuf;
	char *flashFile;
	int blockNum, ppn, pbn;
	
	// flash memory 파일 생성: 위에서 선언한 flashfp를 사용하여 flash 파일을 생성한다. 그 이유는 fdevicedriver.c에서 
	//                 flashfp 파일포인터를 extern으로 선언하여 사용하기 때문이다.
	// 페이지 쓰기: pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스를 호출한다
	// 페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와
	//                  스페어 데이터를 분리해 낸다
	// memset(), memcpy() 등의 함수를 이용하면 편리하다. 물론, 다른 방법으로 해결해도 무방하다.

	if (argc == 1) {
		fprintf(stderr, "usage : %s <option>\n", argv[0]);
		exit(1);
	}

	if (!strcmp(argv[1], "c")) {
		if (argc != 4) {
			fprintf(stderr, "usage : %s c <flashfile> <block#>\n", argv[0]);
			exit(1);
		}

		flashFile = argv[2];
		blockNum = atoi(argv[3]);

		flashfp = fopen(flashFile, "w");

		if (flashfp == NULL) {
			fprintf(stderr, "%s open error\n", flashFile);
			exit(1);
		}
		memset(pagebuf, 0xFF, sizeof(pagebuf));

		for (int i = 0; i < blockNum; i++) {
			for (int j = 0; j < PAGE_NUM; j++) {
				fwrite(pagebuf, sizeof(pagebuf), 1, flashfp);
			}
		}
	}

	else if (!strcmp(argv[1], "w")) {
		if (argc != 6) {
			fprintf(stderr, "usage : %s w <flashfile> <ppn> <sectordat> <sparedata>\n", argv[0]);
			exit(1);
		}

		flashFile = argv[2];
		ppn = atoi(argv[3]);

		flashfp = fopen(flashFile, "r+");
		if (flashfp == NULL) {
			fprintf(stderr, "%s open error\n", flashFile);
			exit(1);
		}

		memset(pagebuf, 0xFF, sizeof(pagebuf));
		memcpy(pagebuf, argv[4], strlen(argv[4]));
		memcpy(pagebuf + SECTOR_SIZE, argv[5], strlen(argv[5]));

		dd_write(ppn, pagebuf);
	}

	else if (!strcmp(argv[1], "r")) {
		if (argc != 4) {
			fprintf(stderr, "usage : %s r <flashfile> <ppn>\n", argv[0]);
			exit(1);
		}

		flashFile = argv[2];
		ppn = atoi(argv[3]);

		flashfp = fopen(flashFile, "r");
		if (flashfp == NULL) {
			fprintf(stderr, "%s open error\n", flashFile);
			exit(1);
		}

		dd_read(ppn, pagebuf);
		memcpy(sectorbuf, pagebuf, sizeof(sectorbuf));
		memcpy(sparebuf, pagebuf + sizeof(sectorbuf), sizeof(sparebuf));

		int sectorDataSize = sizeof(sectorbuf);
		int spareDataSize = sizeof(sparebuf);
		for (int i = 0; i < sizeof(sectorbuf); i++) {
			if (sectorbuf[i] == -1) {
				sectorDataSize = i;
				break;
			}
		}

		for (int i = 0; i < sizeof(sparebuf); i++) {
			if (sparebuf[i] == -1) {
				spareDataSize = i;
				break;
			}
		}

		if (sectorDataSize > 0 || spareDataSize > 0) {
			write(1, sectorbuf, sectorDataSize);
			write(1, " ", 1);
			write(1, sparebuf, spareDataSize);
		}
	}

	else if (!strcmp(argv[1], "e")) {
		if (argc != 4) {
			fprintf(stderr, "usage : %s e <flashfile> <pbn>\n", argv[0]);
			exit(1);
		}

		flashFile = argv[2];
		flashfp = fopen(flashFile, "r+");
		if (flashfp == NULL) {
			fprintf(stderr, "%s open error\n", flashFile);
			exit(1);
		}
		pbn = atoi(argv[3]);
		dd_erase(pbn);
	}

	return 0;
}
