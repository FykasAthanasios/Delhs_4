CC = gcc
Flags = -std=c11 -D_DEFAULT_SOURCE -Wall

targe: .obj_dir .main

clear:
	rm -rf obj
	rm -f *.o
	rm -f cmpcat

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