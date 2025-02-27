#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define RECORD_SIZE 	100
#define SUFFLE_NUM	1000	// 이 값은 마음대로 수정할 수 있음

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

//
// argv[1]: 레코드 파일명
//
int main(int argc, char **argv)
{
	int *read_order_list;
	int num_of_records;
	int fd;
	char buffer[RECORD_SIZE];
	struct timeval start_time, end_time;

	if (argc != 2) {
		fprintf(stderr, "Usage %s <file>\n", argv[0]);
		return 1;
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "%s open error\n", argv[1]);
		return 1;
	}

	num_of_records = lseek(fd, 0, SEEK_END) / 100;
	read_order_list = malloc(num_of_records * sizeof(int));

	// 아래 함수를 실행하면 'read_order_list' 배열에 추후 랜덤하게 읽어야 할 레코드 번호들이 순서대로 나열되어 저장됨
            // 'num_of_records'는 레코드 파일에 저장되어 있는 전체 레코드의 수를 의미함
	GenRecordSequence(read_order_list, num_of_records);

	gettimeofday(&start_time, NULL);

	// 'read_order_list'를 이용하여 표준 입력으로 받은 레코드 파일로부터 레코드를 random 하게 읽어들이고,
            // 이때 걸리는 시간을 측정하는 코드 구현함
	for (int i = 0; i < num_of_records; i++) {
		lseek(fd, read_order_list[i] * RECORD_SIZE, SEEK_SET);
		read(fd, buffer, RECORD_SIZE);
	}

	gettimeofday(&end_time, NULL);
	end_time.tv_sec -= start_time.tv_sec;
	end_time.tv_usec += end_time.tv_sec * 1000000;
	end_time.tv_usec -= start_time.tv_usec;
	printf("#records: %d timecost: %ld us\n", num_of_records, end_time.tv_usec);
	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM * n; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
