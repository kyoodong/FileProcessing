// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include "sectormap.h"
// 필요한 경우 헤더 파일을 추가하시오.

extern FILE *flashfp;

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

typedef struct Page {
	int num;
	struct Page *next, *prev;
} Page;

int addressMappingTable[SECTORS_PER_PAGE * PAGES_PER_BLOCK * BLOCKS_PER_DEVICE];
int freeBlock;
Page freePageList;
Page garbageBlockList;

void insert(Page *base, int ppn) {
	Page *p = malloc(sizeof(Page));
	p->num = ppn;
	p->next = base->next;
	p->prev = base;
	if (base->next != NULL)
		base->next->prev = p;
	base->next = p;
}

void delete(Page *page) {
	if (page->prev != NULL)
		page->prev->next = page->next;

	if (page->next != NULL)
		page->next->prev = page->prev;

	free(page);
}

int is_exist(Page *base, int num) {
	Page *p = base;
	while (p != NULL) {
		if (p->num == num)
			return TRUE;
		p = p->next;
	}
	return FALSE;
}

//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//
void ftl_open()
{
	//
	// address mapping table 초기화
	// free block's pbn 초기화
    	// address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	memset(addressMappingTable, -1, sizeof(addressMappingTable));
	freeBlock = DATABLKS_PER_DEVICE;
	for (int i = 0; i < PAGES_PER_BLOCK * DATABLKS_PER_DEVICE; i++) {
		insert(&freePageList, i);
	}
	return;
}

void print(Page base) {
	Page *p = base.next;
	while (p != NULL) {
		printf("%d ", p->num);
		p = p->next;
	}
	printf("\n");
}

//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	int psn = addressMappingTable[lsn];

	if (psn == -1)
		return;

	dd_read(psn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	return;
}

Page* freeGarbageBlock() {
	Page *garbageBlock = garbageBlockList.next;
	char buffer[PAGE_SIZE];
	SpareData spareData;

	if (garbageBlock == NULL)
		return NULL;

	for (int i = 0; i < PAGES_PER_BLOCK; i++) {
		int fIndex = freeBlock * PAGES_PER_BLOCK + i;
		int bIndex = garbageBlock->num * PAGES_PER_BLOCK + i;

		dd_read(bIndex, buffer);
		memcpy(&spareData, buffer + SECTOR_SIZE, sizeof(spareData));

		// addressMappingTable 이 bIndex 를 가리킨다는 것은 garbage 가 아니라는 것
		if (addressMappingTable[spareData.lpn] == bIndex) {
			addressMappingTable[spareData.lpn] = fIndex;
		}
	
	 	// garbage 인 경우
		else {
			// buffer 를 0xff로 초기화
			memset(buffer, 0xff, sizeof(buffer));

			// 해당 인덱스를 free page 리스트에 추가
			insert(&freePageList, fIndex);
		}
		dd_write(fIndex, buffer);
	}
	freeBlock = garbageBlock->num;
	dd_erase(garbageBlock->num);
	delete(garbageBlock);

	return freePageList.next;
}

void ftl_write(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	char buffer[PAGE_SIZE];
	Page *freePage, *garbageBlock;
	SpareData spareData;
	int block;

	// 가용 페이지
	freePage = freePageList.next;
	garbageBlock = garbageBlockList.next;

	// pagebuf 에 데이터 세팅
	memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
	memset(&spareData, 0xff, sizeof(spareData));
	spareData.lpn = lsn;
	spareData.is_invalid = FALSE;
	memcpy(pagebuf + SECTOR_SIZE, &spareData, sizeof(spareData));
	
	// 가용 페이지가 없는 경우
	if (freePage == NULL) {
		freePage = freeGarbageBlock();

		// 가비지 블록 마저 없는 경우 (모든 페이지를 사용중)
		if (freePage == NULL) {
			// 우회적 overwrite 보다 free block 에 수정된 데이터 블럭을 써버리고
		    // 기존 블럭을 지우는게 더 빠름
			block = addressMappingTable[lsn] / PAGES_PER_BLOCK;

			for (int i = 0; i < PAGES_PER_BLOCK; i++) {
				int fIndex = freeBlock * PAGES_PER_BLOCK + i;
				int bIndex = block * PAGES_PER_BLOCK + i;

				if (bIndex == addressMappingTable[lsn]) {
					dd_write(fIndex, pagebuf);
					addressMappingTable[lsn] = fIndex;
					continue;
				}

				dd_read(bIndex, buffer);
				memcpy(&spareData, buffer + SECTOR_SIZE, sizeof(spareData));
				addressMappingTable[spareData.lpn] = fIndex;
				dd_write(fIndex, buffer);
			}

			dd_erase(block);
			freeBlock = block;
			return;
		}
	}

	// overwrite 하는 경우
	// 기존 ppn을 garbage 로 등록
	if (addressMappingTable[lsn] != -1) {
		block = addressMappingTable[lsn] / PAGES_PER_BLOCK;
		if (!is_exist(garbageBlockList.next, block))
			insert(&garbageBlockList, block);
	}

	// 가용 ppn 을 찾아서
	dd_write(freePage->num, pagebuf);
	addressMappingTable[lsn] = freePage->num;
	delete(freePage);
	return;
}

void ftl_print()
{
	printf("lpn\tppn\n");
	for (int i = 0; i < PAGES_PER_BLOCK * DATABLKS_PER_DEVICE; i++)
		printf("%d\t%d\n", i, addressMappingTable[i]);
	printf("free block's pbn = %d\n", freeBlock);
	printf(" garbaageBlockList \n");
	print(garbageBlockList);
	printf(" free Page List \n");
	print(freePageList);
	return;
}
