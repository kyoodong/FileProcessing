#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include "person.h"

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
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 ���ų� ���� ���ڵ带 ������ ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. �� ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE * pagenum, SEEK_SET); 
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE * pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// ���ο� ���ڵ带 ������ �� �͹̳ηκ��� �Է¹��� ������ Person ����ü�� ���� �����ϰ�, pack() �Լ��� ����Ͽ�
// ���ڵ� ���Ͽ� ������ ���ڵ� ���¸� recordbuf�� �����. �׷� �� �� ���ڵ带 ������ �������� readPage()�� ���� ���α׷� ��
// �о� �� �� pagebuf�� recordbuf�� ����Ǿ� �ִ� ���ڵ带 �����Ѵ�. �� ���� writePage() ȣ���Ͽ� pagebuf�� �ش� ������ ��ȣ��
// �����Ѵ�. pack() �Լ����� readPage()�� writePage()�� ȣ���ϴ� ���� �ƴ϶� pack()�� ȣ���ϴ� ������ pack() �Լ� ȣ�� ��
// readPage()�� writePage()�� ���ʷ� ȣ���Ͽ� ���ڵ� ���⸦ �ϼ��Ѵٴ� �ǹ��̴�.
// 
void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf, 0, RECORD_SIZE);
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}

// 
// �Ʒ��� unpack() �Լ��� recordbuf�� ����Ǿ� �ִ� ���ڵ带 ����ü�� ��ȯ�� �� ����Ѵ�. �� �Լ��� ���� ȣ��Ǵ�����
// ������ ������ pack()�� �ó������� �����ϸ� �ȴ�.
//
void unpack(const char *recordbuf, Person *p)
{
	sscanf(recordbuf, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}

//
// ���ο� ���ڵ带 �����ϴ� ����� �����ϸ�, �͹̳ηκ��� �Է¹��� �ʵ尪�� ����ü�� ������ �� �Ʒ��� insert() �Լ��� ȣ���Ѵ�.
//
void insert(FILE *fp, const Person *p)
{
	char recordbuf[RECORD_SIZE];
	char headerPagebuf[PAGE_SIZE];
	char pagebuf[PAGE_SIZE];
	char *cp;
	int *meta;
	Header *header;
	int recordPerPage;
	int recordNum;

	// ���� ���ڵ尡 �ִ��� Ȯ��
	memset(headerPagebuf, 0, sizeof(headerPagebuf));
	readPage(fp, headerPagebuf, 0);
	header = (Header *) headerPagebuf;

	// ���ʷ� ������ ������ ���
	if (header->pageNum == 0) {
		// ��� ����
		header->pageNum = 1;
		header->recordNum = 1;
		header->deletedPage = -1;
		header->deletedRecord = -1;
		writePage(fp, headerPagebuf, 0);

		// ���ڵ� �߰�
		memset(pagebuf, 0, sizeof(pagebuf));
		pack(pagebuf, p);
		writePage(fp, pagebuf, 1);
		return;
	}

	// ���� ���ڵ尡 ���� ���
	if (header->deletedPage == -1 && header->deletedRecord == -1) {
		recordPerPage = PAGE_SIZE / RECORD_SIZE;

		// ������ �������� ���� �ڸ��� �ִ� ���
		if (header->recordNum < header->pageNum * recordPerPage) {
			readPage(fp, pagebuf, header->pageNum);
			recordNum = header->recordNum % recordPerPage;
			cp = pagebuf + RECORD_SIZE * recordNum;

			pack(cp, p);

			writePage(fp, pagebuf, header->pageNum);

			// ��� ����
			header->recordNum++;
			writePage(fp, headerPagebuf, 0);
		}

		// �������� ���� �ڸ��� ��� ���ο� �������� �����ߵǴ� ���
		else {
			memset(pagebuf, 0, sizeof(pagebuf));
			pack(pagebuf, p);
			writePage(fp, pagebuf, header->pageNum + 1);

			// ��� ����
			header->pageNum++;
			header->recordNum++;
			writePage(fp, headerPagebuf, 0);
		}
	}

	// ���� ���ڵ尡 �ִ� ���
	else {
		readPage(fp, pagebuf, header->deletedPage);
		int page = header->deletedPage;

		cp = pagebuf;
		cp += RECORD_SIZE * header->deletedRecord;

		if (*cp != '*') {
			printf("error\n");
			return;
		}

		cp++;
		meta = (int *) cp;
		
		// header �� ���ŵ� ���ο� ������
		header->deletedPage = *meta++;
		header->deletedRecord = *meta;

		// ������ ������ �������� cp �� ������ ��ŷ
		cp--;

		// pagebuf�� deletedRecord��° �����Ϳ� ��ŷ �� ����
		pack(cp, p);
		writePage(fp, pagebuf, page);

		// �ش� ����
		writePage(fp, headerPagebuf, 0);
	}
}

//
// �ֹι�ȣ�� ��ġ�ϴ� ���ڵ带 ã�Ƽ� �����ϴ� ����� �����Ѵ�.
//
void delete(FILE *fp, const char *sn)
{
	char headerPagebuf[PAGE_SIZE];
	char pagebuf[PAGE_SIZE];
	Header *header;
	int recordPerPage;
	char *record;
	int *meta;
	Person person;

	readPage(fp, headerPagebuf, 0);
	header = (Header *) headerPagebuf;

	recordPerPage = PAGE_SIZE / RECORD_SIZE;
	for (int i = 1; i <= header->pageNum; i++) {
		readPage(fp, pagebuf, i);
		for (int j = 0; j < recordPerPage; j++) {
			record = pagebuf + j * RECORD_SIZE;
			unpack(record, &person);

			// ���� �ֹι�ȣ�� �߰�
			//printf("i = %d\nj = %d\n%s\n%s\n", i, j, person.sn, sn);
			if (!strcmp(person.sn, sn)) {
				*record++ = '*';
				meta = (int *) record;

				// ���� ����
				if (header->deletedPage == -1 && header->deletedRecord == -1) {
					*meta++ = -1;
					*meta = -1;
				}

				// ������ ������ ���ڵ尡 �ִ� ���
				else {
					*meta++ = header->deletedPage;
					*meta = header->deletedRecord;
				}

				writePage(fp, pagebuf, i);

				// ��� ����
				header->deletedPage = i;
				header->deletedRecord = j;
				writePage(fp, headerPagebuf, 0);
				return;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;  // ���ڵ� ������ ���� ������
	Person person;
	fp = fopen(argv[2], "r+");
	if (fp == NULL) {
		fp = fopen(argv[2], "w");
	}

	// ����
	if (argv[1][0] == 'i') {
		if (argc < 9) {
			printf("usage : a.out i filename sn name age addr phone email\n");
			return 1;
		}

		strcpy(person.sn, argv[3]);
		strcpy(person.name, argv[4]);
		strcpy(person.age, argv[5]);
		strcpy(person.addr, argv[6]);
		strcpy(person.phone, argv[7]);
		strcpy(person.email, argv[8]);
		insert(fp, &person);
	}

	// ����
	else if (argv[1][0] == 'd') {
		if (argc < 4) {
			printf("usage : a.out d filename sn\n");
			return 1;
		}

		delete(fp, argv[3]);
	}

	else {
		printf("�� �� ���� ����Դϴ�.\n");
		return 1;
	}

	
	return 0;
}
