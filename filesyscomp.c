#include "filesyscomp.h"
#include "filesyshandling.h"

#define _DEFAULT_SOURCE

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

bool check_to_ignore_dir(char *name)
{
   if(strcmp(name, ".") == 0  || strcmp(name, "..") == 0)
   {
      return true;
   }
   return false;
}


void traverse_differences(char *path, char *result_parent)
{
   DIR* direct = CALL_OR_DIE(opendir(path), "opendir error", DIR*, NULL);

   struct dirent *d;
   int path_len = strlen(path) + 1 + 32;
   char *new_path = malloc(path_len * sizeof(char));
   char *new_result_path = NULL;
   int result_path_len;

   if(result_parent != NULL)
   {
      result_path_len = strlen(result_parent) + 1 + 32;
      new_result_path = malloc(result_path_len * sizeof(char));
   }
   
   while((d = readdir(direct)) != NULL)
   {
      new_path = add_to_path(path, d->d_name, &path_len, new_path);
      
      if(d->d_type == DT_DIR && !check_to_ignore_dir(d->d_name))
      {
         new_result_path = add_to_path(result_parent, d->d_name, &result_path_len, new_result_path);
         CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);

         printf("\t%s\n", new_path);
         traverse_differences(new_path, new_result_path);
      }
      else if(d->d_type == DT_REG)
      {
         new_result_path = add_to_path(result_parent, d->d_name, &result_path_len, new_result_path);
         copy_file(new_path, new_result_path);
         printf("\t%s\n", new_path);
      }
   }

   free_path(new_path);
   free_path(new_result_path);
}

//if index is 0 it means we are traversing the first directory
//if index is 1 , the second
void print_differences_and_merge_rec(char **parent_dir, int index, char *result_parent)
{
   int compare_index = 0;
   if(index == 0)
      compare_index = 1;

   DIR *dir[2];
   dir[0] = CALL_OR_DIE(opendir(parent_dir[0]), "opendir error", DIR*, NULL);
   dir[1] = CALL_OR_DIE(opendir(parent_dir[1]), "opendir error", DIR*, NULL);

   struct dirent *d1;

   while((d1 = readdir(dir[index])) != NULL)
   {
      //NOTE add file to tha result file system
      bool found = false;
      struct dirent *d2;
      while((d2 = readdir(dir[compare_index])) != NULL)
      {
         //NOTE MAYBE ADD A LIST OF THE FILES THAT ARE THE SAME SO WE DONT SEARCH IT AGAIN FOR THE SECOND DIRECTORY
         if(d1->d_type == d2->d_type)
         {
            if(d1->d_type == DT_REG)
            {
               if(same_file(d1->d_name, d2->d_name, parent_dir[index], parent_dir[compare_index]))
               {
                  found = true;
                  break;
               }
               else if(strcmp(d1->d_name, d2->d_name) == 0)
               {

               }
            }
            else if(d1->d_type == DT_DIR && same_dir(d1->d_name, d2->d_name))
            {
               found = true;
               char *new_parents_dir[2];
               new_parents_dir[index] = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
               new_parents_dir[compare_index] = add_to_path(parent_dir[compare_index], d2->d_name, NULL, NULL);
               print_differences_and_merge_rec(new_parents_dir ,index);
               free_path(new_parents_dir[index]);
               free_path(new_parents_dir[compare_index]);
               break;
            }
         }
      }
      if(found == false)
      {
         if(d1->d_type == DT_REG)
         {
            char* path_to_file = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
            printf("\t%s\n", path_to_file);
            free_path(path_to_file);
         }
         else if(d1->d_type == DT_DIR)
         {
            char* path_to_dir = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
            char *new_result_path = add_to_path(result_parent, d1->d_name, NULL, NULL);
            printf("\t%s\n", path_to_dir);
            traverse_differences(path_to_dir, new_result_path);
            free_path(path_to_dir);
            free_path(new_result_path);
         }
      }

      found = false;
      rewinddir(dir[compare_index]);
   }

   if(closedir(dir[0]) == -1)
   {
      perror("close error");
   }
   
   if(closedir(dir[1]) == -1)
   {
      perror("close error");
   }
}

void print_differences_and_merge(char *parent1_dir, char *parent2_dir, char *result_parent)
{
   if(result_parent != NULL)
   {
      CALL_OR_DIE(mkdir(result_parent, S_IRWXU), "wtf", int, -1);
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
            if(d1->d_type == DT_REG && same_file(d1->d_name, d2->d_name, parent1_dir, parent2_dir))
            {
               found = true;
               break;
            }
            else if(d1->d_type == DT_DIR && same_dir(d1->d_name, d2->d_name))
            {
               //NOTE ADD RECURSIVE CALL FOR THIS DIRECTORY
               found = true;
               break;
            }
         }
      }
      if(found == false)
      {
         
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
            if(d2->d_type == DT_REG && same_file(d1->d_name, d2->d_name, parent1_dir, parent2_dir))
            {
               found = true;
               break;
            }
            else if(d2->d_type == DT_DIR && same_dir(d1->d_name, d2->d_name))
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
         
      }

      found = false;
      rewinddir(dir2);
   }
}