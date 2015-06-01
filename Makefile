.PHONY : all

all: src/*.c
	gcc src/*.c -Ih/ -o build/start

debug: src/*.c
	gcc src/*.c -Wall -Ih/ -ggdb -o build/start

gdb:
	gcc src/*.c -Wall -ggdb -Ih/ -o build/start
	gdb build/start

clean: 
	rm build/*
	rm disks/*
