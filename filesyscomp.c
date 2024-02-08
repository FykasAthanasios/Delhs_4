#include "filesyscomp.h"
#include "filesyshandling.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

static char *parent1_dir;
static char *parent2_dir;

#define CALL_OR_DIE(f_call, error_message, Type, error_value)      \
({                                                                 \
   Type value =  f_call;                                           \
   if(value == error_value){                                       \
      perror(error_message); exit(EXIT_FAILURE);                   \
   }                                                               \
   value;                                                          \
})


void filesys_initilization(char *parent1_directory, char *parent2_directory)
{
   int size1 = strlen(parent1_directory) + 2;
   int size2 = strlen(parent2_directory) + 2;

   parent1_dir = malloc(size1 * sizeof(char));
   parent2_dir = malloc(size2 * sizeof(char));

   strcpy(parent1_dir, parent1_directory);
   strcpy(parent2_dir, parent2_directory);

   parent1_dir[size1 - 1] = '\0';
   parent2_dir[size2 - 1] = '\0';
   parent1_dir[size1 - 2] = '/';
   parent2_dir[size2 - 2] = '/';

   initilization(parent1_dir, parent2_dir);
}

void print_differences_and_merge_rec(char *parent1, char *parent2, char *result_parent)
{
}

void print_differences_and_merge(char *result_parent)
{
   if(result_parent != NULL)
   {
      CALL_OR_DIE(mkdir(result_parent, S_IRWXU), "mkdir error", int, -1);
   }

   DIR *dir1 = CALL_OR_DIE(opendir(parent1_dir), "opendir error", DIR*, NULL);
   DIR *dir2 = CALL_OR_DIE(opendir(parent2_dir), "opendir error", DIR*, NULL);

   struct dirent *d1;

   printf("In %s :\n", parent1_dir);

   while((d1 = readdir(dir1)) != NULL)
   {
      //NOTE add file to tha result file system
      bool found = false;
      struct dirent *d2;
      while((d2 = readdir(dir2)) != NULL)
      {
         //NOTE MAYBE ADD A LIST OF THE FILES THAT ARE THE SAME SO WE DONT SEARCH IT AGAIN FOR THE SECOND DIRECTORY
         if(d1->d_type == d2->d_type)
         {
            if(d1->d_type == DT_REG && compare_file(d1->d_name, d2->d_name))
            {
               found = true;
               break;
            }
            else if(d1->d_type == DT_DIR && compare_dir(d1->d_name, d2->d_name))
            {
               //NOTE ADD RECURSIVE CALL FOR THIS DIRECTORY
               found = true;
               break;
            }
         }
      }
      if(found == false)
      {
         printf("%s\n", d1->d_name);
      }

      found = false;
      rewinddir(dir2);
   }

   rewinddir(dir1);
   struct dirent *d2;

   while((d2 = readdir(dir2)) != NULL)
   {
      bool found = false;
      struct dirent *d1;
      while((d1 = readdir(dir1)) != NULL)
      {
         if(d1->d_type == d2->d_type)
         {
            if(d2->d_type == DT_REG && compare_file(d1->d_name, d2->d_name))
            {
               found = true;
               break;
            }
            else if(d2->d_type == DT_DIR && compare_dir(d1->d_name, d2->d_name))
            {
               //NOTE ADD RECURSIVE CALL FOR THIS DIRECTORY
               found = true;
               break;
            }
         }
      }
      if(found == false)
      {
         //NOTE add file to tha result file system
         printf("%s\n", d1->d_name);
      }

      found = false;
      rewinddir(dir2);
   }
}