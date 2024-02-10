CC = gcc
Flags = -std=c11 -Wall

targe: .obj_dir .main

clear:
	rm -rf obj
	rm -f *.o
	rm -f cmpcat
case1:
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
	ln dirA/C.txt dirA/hardlinkA
	ln dirB/C.txt dirB/hardlinkA
 
case2:
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
	echo "This is a new file" | cat >> dirA/C.txt
	echo "This is a different file" | cat >> dirB/C.txt
	ln dirA/C.txt dirA/hardlinkA
	ln dirB/C.txt dirB/hardlinkA

clear_test_dir:
	rm -rf dirA
	rm -rf dirB

clear_created_dir:
	rm -rf dirC

.obj_dir:
	mkdir -p obj

.main: .filesyscomp.o
	$(CC) $(Flags) obj/*.o main.c -o cmpcat

.filesyshandling.o: filesyshandling.h filesyshandling.c
	$(CC) $(Flags) -c filesyshandling.c -o obj/filesyshandling.o

.filesyscomp.o: filesyscomp.h filesyscomp.c .filesyshandling.o
	$(CC) $(Flags) -c filesyscomp.c -o obj/filesyscomp.o