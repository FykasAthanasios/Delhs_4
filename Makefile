CC = gcc
Flags = -std=c11 -Wall
mypath = $(shell pwd)

target: .obj_dir .main

run :
	rm -rf dirC
	./cmpcat -d dirA dirB -s dirC

clear:
	rm -rf obj
	rm -f *.o
	rm -f cmpcat
	rm -rf dirC

test_dir:
	mkdir dirA
	touch dirA/A
	touch dirA/B
	mkdir dirA/dir1
	mkdir dirA/dir2
	mkdir dirA/dir3
	touch dirA/C.txt
	touch dirA/dir1/A1
	mkdir dirA/dir2/dir21
	mkdir dirA/dir2/dir22
	touch dirA/dir2/dir22/file.csv
	mkdir -p dirB/dir1/d11/d111/d1111
	touch dirB/B
	touch dirB/C.txt
	mkdir dirB/dir2
	mkdir dirB/dir2/d21
	mkdir dirB/dir2/d22
	touch dirB/dir2/d22/file.csv
	touch dirB/dir2/myfile.c
	touch dirB/dir1/A1
	touch dirB/dir1/d11/d111/d1111/test.txt
	ln -s $(mypath)/dirA/A dirA/dir1/link
	ln -s $(mypath)/dirB/dir1/d11/d111/d1111/test.txt dirB/dir1/link
	ln -s $(mypath)/dirA/dir1/A1 dirA/link2
	ln -s $(mypath)/dirB/dir1/A1 dirB/link2
	ln -s $(mypath)/dirA/B dirA/link3
	ln -s $(mypath)/dirB/B dirB/link3
	echo "hello" | cat >> dirA/B
	echo "hello2" | cat >> dirB/B
	echo "hello world" | cat >> dirA/C.txt
	echo "hello world" | cat >> dirB/C.txt
	echo "i am A" | cat >> dirA/A
	ln -s $(mypath)/dirA/link3 dirA/link4
	ln -s $(mypath)/dirB/link3 dirB/link4
	ln $(mypath)/dirA/C.txt dirA/common_hard_link
	ln $(mypath)/dirB/C.txt dirB/common_hard_link
	ln $(mypath)/dirA/A dirA/dir3/hard_link
	

clear_test_dir:
	rm -rf dirA
	rm -rf dirB

.obj_dir:
	mkdir -p obj

.main: .filesyscomp.o main.c
	$(CC) $(Flags) obj/*.o main.c -o cmpcat

.filesyshandling.o: filesyshandling.h filesyshandling.c .i_node_table.o
	$(CC) $(Flags) -c filesyshandling.c -o obj/filesyshandling.o

.filesyscomp.o: filesyscomp.h filesyscomp.c .filesyshandling.o 
	$(CC) $(Flags) -c filesyscomp.c -o obj/filesyscomp.o

.i_node_table.o: i_node_table.h i_node_table.c
	$(CC) $(Flags) -c i_node_table.c -o obj/i_node_table.o