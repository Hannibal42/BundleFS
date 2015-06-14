.PHONY : all

SRC = src/*.c
SRC_TEST = unity/src/*.c test/src/*.c
INC = -Iinclude/ -Iunity/include/ -Itest/include

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
	gcc $(SRC) -Wall $(INC) -ggdb -o build/start

memprof:
	valgrind --massif-out-file=build/memory.out --tool=massif build/start
