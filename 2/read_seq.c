#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define RECORD_SIZE 100

//
// argv[1]: ���ڵ� ���ϸ�
//
int main(int argc, char **argv)
{
	int fd;
	struct timeval start_time, end_time;
	int recordCount;
	char buffer[RECORD_SIZE];

	// ǥ���Է����� ���� ���ڵ� ���Ͽ� ����Ǿ� �ִ� ��ü ���ڵ带 "������"���� �о���̰�, �̶�
	// �ɸ��� �ð��� �����ϴ� �ڵ� ������
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <record file>\n", argv[0]);
		return 1;
	}

	char* record_file = argv[1];

	if ((fd = open(record_file, O_RDONLY)) < 0) {
		fprintf(stderr, "%s open error\n", record_file);
		return 1;
	}

	recordCount = lseek(fd, 0, SEEK_END) / RECORD_SIZE;

	gettimeofday(&start_time, NULL);

	for (int i = 0; i < recordCount; i++) {
		read(fd, buffer, RECORD_SIZE);
	}
	gettimeofday(&end_time, NULL);
	end_time.tv_sec -= start_time.tv_sec;
	end_time.tv_usec += end_time.tv_sec * 1000000;
	end_time.tv_usec -= start_time.tv_usec;
	printf("#records: %d timecost: %ld us\n", recordCount, end_time.tv_usec);

	return 0;
}
