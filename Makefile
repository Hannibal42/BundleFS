.PHONY : all

SRC = src/*.c unity/src/*.c
INC = -Iinclude/ -Iunity/include/

all: 
	gcc $(SRC) $(INC) -o build/start

debug: 
	gcc $(SRC) -Wall $(INC) -ggdb -o build/start

gdb:
	gcc $(SRC) -Wall -ggdb $(INC) -o build/start
	gdb build/start

clean: 
	rm build/*
	rm disks/*
