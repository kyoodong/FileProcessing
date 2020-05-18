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



// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE * pagenum, SEEK_SET); 
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE * pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
// 
void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf, 0, RECORD_SIZE);
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{
	sscanf(recordbuf, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#%[^#]#", p->sn, p->name, p->age, p->addr, p->phone, p->email);
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
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

	// 삭제 레코드가 있는지 확인
	memset(headerPagebuf, 0, sizeof(headerPagebuf));
	readPage(fp, headerPagebuf, 0);
	header = (Header *) headerPagebuf;

	// 최초로 파일이 생성된 경우
	if (header->pageNum == 0) {
		// 헤더 갱신
		header->pageNum = 1;
		header->recordNum = 1;
		header->deletedPage = -1;
		header->deletedRecord = -1;
		pageWrite(fp, headerPagebuf, 0);

		// 레코드 추가
		memset(pagebuf, 0, sizeof(pagebuf));
		pack(pagebuf, p);
		pageWrite(fp, pagebuf, 1);
	}

	// 삭제 레코드가 없는 경우
	if (header->deletedPage == -1 && header->deletedRecord == -1) {
		recordPerPage = PAGE_SIZE / RECORD_SIZE;

		// 마지막 페이지에 남은 자리가 있는 경우
		if (header->recordNum < header->pagenum * recordPerPage) {
			pageRead(fp, pagebuf, header->pageNum);
			recordNum = header->recordNum % recordPerPage;
			cp = pagebuf + RECORD_SIZE * recordNum;

			pack(cp, p);
			pageWrite(fp, pagebuf, header->pageNum);

			// 헤더 갱신
			header->recordNum++;
			pageWrite(fp, headerPagebuf, 0);
		}

		// 페이지에 남은 자리가 없어서 새로운 페이지를 만들어야되는 경우
		else {
			memset(pagebuf, 0, sizeof(pagebuf));
			pack(pagebuf, p);
			pageWrite(fp, pagebuf, recordPage + 1);

			// 헤더 갱신
			header->pageNum++;
			header->recordNum++;
			pageWrite(fp, headerPagebuf, 0);
		}
	}

	// 삭제 레코드가 있는 경우
	else {
		readPage(fp, pagebuf, header->deletedPage);
		cp = pagebuf;
		cp += RECORD_SIZE * header->deletedRecord;

		if (*cp != '*') {
			printf("error\n");
			return;
		}

		cp++;
		
		// header 에 갱신될 새로운 데이터
		sscanf(cp, "%d%d", &header->deletedPage, &header->deletedRecord);

		cp--;

		// pagebuf의 deletedRecord번째 데이터에 패킹 후 저장
		pack(cp, p);
		writePage(pagebuf);

		// 해더 갱신
		writePage(headerPagebuf);
	}
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{

}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	fp = fopen(argv[1], "a+");
	return 1;
}
