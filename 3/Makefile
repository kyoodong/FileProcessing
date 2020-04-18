a.out: fdevicedriver.o ftlmgr.o
	gcc fdevicedriver.o ftlmgr.c -o a.out

fdevicedriver.o: fdevicedriver.c
	gcc -c fdevicedriver.c -o fdevicedriver.o

ftlmgr.o: ftlmgr.c
	gcc -c ftlmgr.c -o ftlmgr.o

clear:
	rm *.o a.out
