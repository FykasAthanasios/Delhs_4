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

char *add_to_path(char *current_path, char *new, int *current_size, char *result);
bool same_dir(char* name1, char* name2);
bool same_file(char* name1, char* name2, char* path1, char* path2);

#endif