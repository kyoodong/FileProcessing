compile: clear
	gcc person.c
	./a.out i dat "111" "Dongkyoo Lee" "23" "Seoul" "111" "aaa@naver.com"
	./a.out i dat "222" "Dongkyoo Kim" "23" "Anyang" "222" "bbb@naver.com"
	./a.out i dat "333" "Dongkyoo Park" "23" "Busan" "333" "ccc@naver.com"
	./a.out d dat "222"
	./a.out i dat "444" "New One" "25" "Hongkong" "444" "ddd@naver.com"
	./a.out i dat "555" "New One" "26" "Hongkong" "555" "eee@naver.com"
	hexdump dat -C
	./a.out d dat "666"
	hexdump dat -C
	./a.out d dat "111"
	hexdump dat -C
	./a.out d dat "333"
	./a.out i dat "666" "New One" "26" "Hongkong" "555" "eee@naver.com"
	./a.out i dat "777" "New One" "26" "Hongkong" "555" "eee@naver.com"
	./a.out i dat "888" "New One" "26" "888" "888" "888@naver.com"
	./a.out i dat "999" "New One" "26" "999" "999" "999@naver.com"


clear:
	rm -f dat a.out
