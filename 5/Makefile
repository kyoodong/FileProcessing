compile: clear
	gcc person.c
	./a.out i person.dat "111" "Dongkyoo Lee" "23" "Seoul" "111" "aaa@naver.com"
	./a.out i person.dat "222" "Dongkyoo Kim" "23" "Anyang" "222" "bbb@naver.com"
	./a.out i person.dat "333" "Dongkyoo Park" "23" "Busan" "333" "ccc@naver.com"
	./a.out d person.dat "222"
	./a.out i person.dat "444" "New One" "25" "Hongkong" "444" "ddd@naver.com"
	./a.out i person.dat "555" "New One" "26" "Hongkong" "555" "eee@naver.com"
	./a.out d person.dat "666"
	./a.out d person.dat "111"
	./a.out d person.dat "333"
	hexdump person.dat -C
	./a.out i person.dat "666" "New One" "26" "Hongkong" "555" "eee@naver.com"
	hexdump person.dat -C
	./a.out i person.dat "777" "New One" "26" "Hongkong" "555" "eee@naver.com"
	hexdump person.dat -C


clear:
	rm -f person.dat
