#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define RECORD_SIZE 	100
#define SUFFLE_NUM	1000	// �� ���� ������� ������ �� ����

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

//
// argv[1]: ���ڵ� ���ϸ�
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

	// �Ʒ� �Լ��� �����ϸ� 'read_order_list' �迭�� ���� �����ϰ� �о�� �� ���ڵ� ��ȣ���� ������� �����Ǿ� �����
            // 'num_of_records'�� ���ڵ� ���Ͽ� ����Ǿ� �ִ� ��ü ���ڵ��� ���� �ǹ���
	GenRecordSequence(read_order_list, num_of_records);

	gettimeofday(&start_time, NULL);

	// 'read_order_list'�� �̿��Ͽ� ǥ�� �Է����� ���� ���ڵ� ���Ϸκ��� ���ڵ带 random �ϰ� �о���̰�,
            // �̶� �ɸ��� �ð��� �����ϴ� �ڵ� ������
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
