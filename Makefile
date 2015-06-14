.PHONY : all

SRC = src/*.c
SRC_TEST = test/src/*.c unity/src/*.c 
SRC_MEM = memprof/*.c
INC = -Iinclude/ -Itest/include/ -Iunity/include/

all: 
	gcc $(SRC) $(SRC_TEST) $(INC) -o build/test

debug: 
	gcc $(SRC) $(SRC_TEST) -Wall $(INC) -ggdb -o build/test 

gdb:
	gcc $(SRC) $(SRC_TEST) -Wall -ggdb $(INC) -o build/test
	gdb build/test

clean: 
	rm build/*
	rm disks/*

memory:
	gcc $(SRC) $(SRC_MEM) -Wall $(INC) -ggdb -o build/memory

heap:
	valgrind --massif-out-file=build/memory.out --max-snapshots=1000 --tool=massif build/memory

stack:
	valgrind --massif-out-file=build/memory.out --stacks=yes --max-snapshots=1000 --tool=massif build/memory
