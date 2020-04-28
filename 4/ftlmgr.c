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
	int ppn;
	struct Page *next, *prev;
} Page;

int addressMappingTable[SECTORS_PER_PAGE * PAGES_PER_BLOCK * BLOCKS_PER_DEVICE];
int freeBlock;
Page freePageList;
Page garbagePageList;

void insert(Page *base, int ppn) {
	Page *p = malloc(sizeof(Page));
	p->ppn = ppn;
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

void ftl_write(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	char buffer[PAGE_SIZE];
	Page *freePage, *garbagePage;
	SpareData spareData;
	int block;

	// 가용 페이지
	freePage = freePageList.next;
	garbagePage = garbagePageList.next;

	// pagebuf 에 데이터 세팅
	memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
	spareData.lpn = lsn;
	spareData.is_invalid = FALSE;
	memcpy(pagebuf + SECTOR_SIZE, &spareData, sizeof(spareData));
	
	// 가용 페이지가 없는 경우
	if (freePage == NULL) {
		// 가비지 페이지 마저 없는 경우 (모든 페이지를 사용중)
		// 이럴 때는 어쩔 수 없이 우회적 덮어쓰기를 함
		if (garbagePage == NULL) {
			block = addressMappingTable[lsn] / PAGES_PER_BLOCK;
			for (int i = 0; i < PAGES_PER_BLOCK; i++) {
				int fIndex = freeBlock * PAGES_PER_BLOCK + i;
				int bIndex = block * PAGES_PER_BLOCK + i;
				if (bIndex == addressMappingTable[lsn])
					continue;
				dd_read(bIndex, buffer);
				dd_write(fIndex, buffer);
			}
			dd_erase(block);

			for (int i = 0; i < PAGES_PER_BLOCK; i++) {
				int fIndex = freeBlock * PAGES_PER_BLOCK + i;
				int bIndex = block * PAGES_PER_BLOCK + i;
				if (bIndex == addressMappingTable[lsn]) {
					dd_write(bIndex, pagebuf);
					continue;
				}
				dd_read(fIndex, buffer);
				dd_write(bIndex, buffer);
			}

			dd_erase(freeBlock);
			return;
		}

		// free block
		block = garbagePage->ppn / PAGES_PER_BLOCK;
		for (int i = 0; i < PAGES_PER_BLOCK; i++) {
			int fIndex = freeBlock * PAGES_PER_BLOCK + i;
			int bIndex = block * PAGES_PER_BLOCK + i;
			if (bIndex == garbagePage->ppn) {
				dd_write(fIndex, pagebuf);
				addressMappingTable[lsn] = fIndex;
				continue;
			}
			dd_read(bIndex, buffer);
			dd_write(fIndex, buffer);
			memcpy(&spareData, buffer + SECTOR_SIZE, sizeof(spareData));
			if (addressMappingTable[spareData.lpn] == bIndex) {
				addressMappingTable[spareData.lpn] = fIndex;
			}
		}
		dd_erase(block);

		delete(garbagePage);
		freeBlock = block;
		return;
	}

	// overwrite 하는 경우
	// 기존 ppn을 garbage 로 등록
	if (addressMappingTable[lsn] != -1) {
		insert(&garbagePageList, addressMappingTable[lsn]);
	}

	// 가용 ppn 을 찾아서
	dd_write(freePage->ppn, pagebuf);
	addressMappingTable[lsn] = freePage->ppn;
	delete(freePage);
	return;
}

void ftl_print()
{
	printf("lpn\tppn\n");
	for (int i = 0; i < PAGES_PER_BLOCK * DATABLKS_PER_DEVICE; i++)
		printf("%d\t%d\n", i, addressMappingTable[i]);
	printf("free block's pbn = %d\n", freeBlock);
	return;
}
