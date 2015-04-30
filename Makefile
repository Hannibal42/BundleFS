makeall: src/disk_interface.c src/disk.c
	gcc src/disk_interface.c src/disk.c -o build/start

