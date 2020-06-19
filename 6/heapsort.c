#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

int page_num;
int record_num;
int heap_size;

typedef struct Header {
     int pageNum;
     int recordNum;
     int deletedPage;
     int deletedRecord;
} Header;

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, SEEK_SET, pagenum * PAGE_SIZE);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, SEEK_SET, pagenum * PAGE_SIZE);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

void unpack(Person *p, char *record) {
	char buf[RECORD_SIZE];
	char *cp;

	memcpy(buf, record, RECORD_SIZE);
	memset(p, 0, sizeof(Person));
	
	cp = strtok(buf, "#");
	if (cp == NULL) {
		p->sn[0] = '*';
		return;
	}

	strcpy(p->sn, cp);
	strcpy(p->name, strtok(NULL, "#"));
	strcpy(p->age, strtok(NULL, "#"));
	strcpy(p->addr, strtok(NULL, "#"));
	strcpy(p->phone, strtok(NULL, "#"));
	strcpy(p->email, strtok(NULL, "#"));
}

void pack(Person *p, char *record) {
	sprintf(record, "%s#%s#%s#%s#%s#%s", p->sn, p->name, p->age, p->addr, p->phone, p->email);
	record[strlen(record)] = '#';
}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	char buf[PAGE_SIZE];
	char record_buf[RECORD_SIZE];
	int record_per_page = PAGE_SIZE / RECORD_SIZE;
	Person p, p2;
	int curSize = 1;
	int parent, current;
	int count = 0;
	heap_size = 0;

	for (int index = 1; index < page_num; index++) {
		readPage(inputfp, buf, index);

		for (int i = 0; i < record_per_page; i++) {
			unpack(&p, buf + i * RECORD_SIZE);
			count++;
			if (p.sn[0] == '*')
				continue;

			heap_size++;
			if (count > record_num)
				break;

			heaparray[curSize] = malloc(RECORD_SIZE);
			pack(&p, heaparray[curSize]);

			current = curSize;
			parent = current / 2;
			while (parent > 0) {
				unpack(&p, heaparray[current]);
				unpack(&p2, heaparray[parent]);

				if (strcmp(p.sn, p2.sn) < 0) {
					memcpy(record_buf, heaparray[current], RECORD_SIZE);
					memcpy(heaparray[current], heaparray[parent], RECORD_SIZE);
					memcpy(heaparray[parent], record_buf, RECORD_SIZE);

					current = parent;
					parent = current / 2;
				} else {
					break;
				}
			}
			curSize++;
		}
	}
}

void pop(char **heaparray) {
	int current, left_child, right_child, min_child;
	Person p1, p2;
	char buf[RECORD_SIZE];

	current = 1;
	memcpy(heaparray[1], heaparray[heap_size], RECORD_SIZE);
	heap_size--;

	while (current <= heap_size) {
		left_child = current * 2;
		right_child = left_child + 1;

		if (left_child > heap_size)
			break;

		if (right_child <= heap_size) {
			unpack(&p1, heaparray[left_child]);
			unpack(&p2, heaparray[right_child]);

			// left가 더 작음
			if (strcmp(p1.sn, p2.sn) < 0) {
				min_child = left_child;
			} else {
				min_child = right_child;
			}
		} else {
			min_child = left_child;
		}

		unpack(&p1, heaparray[current]);
		unpack(&p2, heaparray[min_child]);

		if (strcmp(p1.sn, p2.sn) < 0)
			break;

		memcpy(buf, heaparray[current], RECORD_SIZE);
		memcpy(heaparray[current], heaparray[min_child], RECORD_SIZE);
		memcpy(heaparray[min_child], buf, RECORD_SIZE);
	}
}

//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	int record_per_page = PAGE_SIZE / RECORD_SIZE;
	char buf[PAGE_SIZE];
	int num = 1;
	char *cp;

	// TODO: header 
	Header header;
	header.pageNum = (heap_size - 1) / record_per_page + 2;
	header.recordNum = heap_size;
	header.deletedPage = -1;
	header.deletedRecord = -1;

	memset(buf, 0xff, sizeof(buf));
	memcpy(buf, &header, sizeof(Header));
	writePage(outputfp, buf, 0);

	while (heap_size) {
		cp = buf;

		for (int i = 0; i < record_per_page && heap_size; i++) {
			cp = stpcpy(cp, heaparray[1]);
			pop(heaparray);
		}

		while (cp < buf + PAGE_SIZE) {
			*cp = 0xff;
			cp++;
		}

		writePage(outputfp, buf, num);
		num++;
	}
}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	char buf[PAGE_SIZE];

	if (argc != 4)
		return -1;

	if (argv[1][0] != 's')
		return -1;

	inputfp = fopen(argv[2], "r");
	outputfp = fopen(argv[3], "w");

	memset(buf, 0xff, sizeof(buf));
	readPage(inputfp, buf, 0);

	int *ip = (int *) buf;
	page_num = *ip;

	ip++;
	record_num = *ip;
	char **heaparray = calloc(page_num, sizeof(char *));

	buildHeap(inputfp, heaparray);
	Person *p = malloc(sizeof(Person));
	for (int i = 1; i <= heap_size; i++) {
		if (heaparray[i] == NULL)
			break;

		unpack(p, heaparray[i]); 
	}

	makeSortedFile(outputfp, heaparray);

	return 0;
}
