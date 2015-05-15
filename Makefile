makeall: src/*.c
	gcc src/*.c -o build/start

make debug: src/*.c
	gcc src/*.c -Wall -ggdb -o build/start
	
