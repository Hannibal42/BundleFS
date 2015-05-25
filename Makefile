makeall: src/*.c
	gcc src/*.c -o build/start

make debug: src/*.c
	gcc src/*.c -Wall -ggdb -o build/start

make gdb:
	gcc src/*.c -Wall -ggdb -o build/start
	gdb build/start

make clean: 
	rm build/*
	rm disks/*
