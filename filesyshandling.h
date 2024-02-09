#ifndef FILESYHANDLING_H
#define FILESYHANDLING_H

#include <stdbool.h>

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

bool same_dir(char* name1, char* name2);
bool same_file(char* name1, char* name2, char* path1, char* path2);

#endif