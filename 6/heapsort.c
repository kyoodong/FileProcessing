#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

int page_num;
int record_num;
int heap_size;

typedef struct Header {
     int pageNum;
     int recordNum;
     int deletedPage;
     int deletedRecord;
} Header;

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 �� ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. ��, ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, SEEK_SET, pagenum * PAGE_SIZE);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// �־��� ���ڵ� ���Ͽ��� ���ڵ带 �о� heap�� ����� ������. Heap�� �迭�� �̿��Ͽ� ����Ǹ�, 
// heap�� ������ Chap9���� ������ �˰����� ������. ���ڵ带 ���� �� ������ ������ ����Ѵٴ� �Ϳ� �����ؾ� �Ѵ�.
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

			// left�� �� ����
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
// �ϼ��� heap�� �̿��Ͽ� �ֹι�ȣ�� �������� ������������ ���ڵ带 �����Ͽ� ���ο� ���ڵ� ���Ͽ� �����Ѵ�.
// Heap�� �̿��� ������ Chap9���� ������ �˰����� �̿��Ѵ�.
// ���ڵ带 ������� ������ ���� ������ ������ ����Ѵ�.
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
	FILE *inputfp;	// �Է� ���ڵ� ������ ���� ������
	FILE *outputfp;	// ���ĵ� ���ڵ� ������ ���� ������
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
