.PHONY : all

all: src/*.c
	gcc src/*.c -o build/start

debug: src/*.c
	gcc src/*.c -Wall -ggdb -o build/start

gdb:
	gcc src/*.c -Wall -ggdb -o build/start
	gdb build/start

clean: 
	rm build/*
	rm disks/*
