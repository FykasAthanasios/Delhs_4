#ifndef FILESYHANDLING_H
#define FILESYHANDLING_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdbool.h>
#include <sys/types.h>
#include "i_node_table.h"

#define CALL_OR_DIE(f_call, error_message, Type, error_value)      \
({                                                                 \
   Type value =  f_call;                                           \
   if(value == error_value){                                       \
      perror(error_message); exit(EXIT_FAILURE);                   \
   }                                                               \
   value;                                                          \
})

/*
 *checks if the current size is large enough to hold the new path , if it is it changes the string to the new path,
 *if it is not its frees the current space and create a new one which is large enough to hold the new path.
 *if current_size == NULL then it just creates a new dynbamic string
 *if current_size != NULL but result == NULL it still creates a new space for the result and set the current_size to the correct value
 */
char *add_to_path(char *current_path, char *new, int *current_size, char *result);
void free_path(char *path);

int my_mkdir(char *path, mode_t mode);

int my_creat(char *path, mode_t mode);

void copy_file_or_hard_link(char *path, char *new_path, i_node_node** table);
void copy_link(char *path, char *new_path);

bool same_dir(char* name1, char* name2);
//compare files with the same name
bool same_file(char* name, char* path1, char* path2);
bool file1_modif_less_file2_modif(char *name1, char *name2, char *path1, char *path2);
bool same_link(char* name1, char* path1, char* path2);


#endif