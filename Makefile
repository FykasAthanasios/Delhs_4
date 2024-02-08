CC = gcc

targe: .obj_dir .main

clear:
	rm -rf obj
	rm -f *.o
	rm -f cmpcat

.obj_dir:
	mkdir -p obj

.main: .filesyshandling.o
	$(CC) obj/filesyshandling.o main.c -o cmpcat

.filesyshandling.o: filesyshandling.h filesyshandling.c
	$(CC) -c filesyshandling.c -o obj/filesyshandling.o

.filesyscomp.o: filesyscomp.h filesyscomp.c
	$(CC) -c filesyscomp.c -o obj/filesyscomp.o