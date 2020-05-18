#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
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
		pageWrite(fp, headerPagebuf, 0);

		// ���ڵ� �߰�
		memset(pagebuf, 0, sizeof(pagebuf));
		pack(pagebuf, p);
		pageWrite(fp, pagebuf, 1);
	}

	// ���� ���ڵ尡 ���� ���
	if (header->deletedPage == -1 && header->deletedRecord == -1) {
		recordPerPage = PAGE_SIZE / RECORD_SIZE;

		// ������ �������� ���� �ڸ��� �ִ� ���
		if (header->recordNum < header->pagenum * recordPerPage) {
			pageRead(fp, pagebuf, header->pageNum);
			recordNum = header->recordNum % recordPerPage;
			cp = pagebuf + RECORD_SIZE * recordNum;

			pack(cp, p);
			pageWrite(fp, pagebuf, header->pageNum);

			// ��� ����
			header->recordNum++;
			pageWrite(fp, headerPagebuf, 0);
		}

		// �������� ���� �ڸ��� ��� ���ο� �������� �����ߵǴ� ���
		else {
			memset(pagebuf, 0, sizeof(pagebuf));
			pack(pagebuf, p);
			pageWrite(fp, pagebuf, recordPage + 1);

			// ��� ����
			header->pageNum++;
			header->recordNum++;
			pageWrite(fp, headerPagebuf, 0);
		}
	}

	// ���� ���ڵ尡 �ִ� ���
	else {
		readPage(fp, pagebuf, header->deletedPage);
		cp = pagebuf;
		cp += RECORD_SIZE * header->deletedRecord;

		if (*cp != '*') {
			printf("error\n");
			return;
		}

		cp++;
		
		// header �� ���ŵ� ���ο� ������
		sscanf(cp, "%d%d", &header->deletedPage, &header->deletedRecord);

		cp--;

		// pagebuf�� deletedRecord��° �����Ϳ� ��ŷ �� ����
		pack(cp, p);
		writePage(pagebuf);

		// �ش� ����
		writePage(headerPagebuf);
	}
}

//
// �ֹι�ȣ�� ��ġ�ϴ� ���ڵ带 ã�Ƽ� �����ϴ� ����� �����Ѵ�.
//
void delete(FILE *fp, const char *sn)
{

}

int main(int argc, char *argv[])
{
	FILE *fp;  // ���ڵ� ������ ���� ������
	fp = fopen(argv[1], "a+");
	return 1;
}
