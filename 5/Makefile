compile: clear
	gcc person.c
	./a.out i person.dat "111" "Dongkyoo Lee" "23" "Seoul" "111" "aaa@naver.com"
	hexdump person.dat -C
	./a.out i person.dat "222" "Dongkyoo Kim" "23" "Anyang" "222" "bbb@naver.com"
	hexdump person.dat -C
	./a.out i person.dat "333" "Dongkyoo Park" "23" "Busan" "333" "ccc@naver.com"
	hexdump person.dat -C
	./a.out d person.dat "222"
	hexdump person.dat -C

clear:
	rm -f person.dat
