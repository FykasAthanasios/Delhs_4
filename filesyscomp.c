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
#include <fcntl.h>

bool check_to_ignore_dir(char *name)
{
   if(name[0] == '.'  || strcmp(name, "..") == 0)
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
   char *new_path = CALL_OR_DIE(malloc(path_len * sizeof(char)), "malloc error", void *, NULL);
   char *new_result_path = NULL;
   int result_path_len;

   if(result_parent != NULL)
   {
      result_path_len = strlen(result_parent) + 1 + 32;
      new_result_path = CALL_OR_DIE(malloc(result_path_len * sizeof(char)), "malloc error", void *, NULL);
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
         copy_file_or_hard_link(new_path, new_result_path);
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
   //printf("%s %s\n", parent_dir[0], parent_dir[1]);
   dir[0] = CALL_OR_DIE(opendir(parent_dir[0]), "opendir error", DIR*, NULL);
   dir[1] = CALL_OR_DIE(opendir(parent_dir[1]), "opendir error", DIR*, NULL);

   struct dirent *d1;

   while((d1 = readdir(dir[index])) != NULL)
   {
      if(check_to_ignore_dir(d1->d_name)) continue;

      bool found = false;
      bool copy = true;

      struct dirent *d2;
      while((d2 = readdir(dir[compare_index])) != NULL)
      {
         if(check_to_ignore_dir(d2->d_name)) continue;
         //NOTE MAYBE ADD A LIST OF THE FILES THAT ARE THE SAME SO WE DONT SEARCH IT AGAIN FOR THE SECOND DIRECTORY
         if(d1->d_type == d2->d_type)
         {
            if(d1->d_type == DT_REG)
            {
               if(strcmp(d1->d_name, d2->d_name) == 0)
               {
                  if(same_file(d1->d_name, parent_dir[index], parent_dir[compare_index]))
                  {
                     //if we are searching in the second directory it is the second time we encounter the duplicate file so we dont need to copy it to the merged dir
                     if(index != 0)
                     {
                        copy = false;
                     }
                     found = true;
                     break;
                  }
                  else if(file1_modif_less_file2_modif(d1->d_name, d2->d_name, parent_dir[index], parent_dir[compare_index]))
                  {
                     copy = false;
                  }
               }
            }
            else if(d1->d_type == DT_DIR)
            {
               if(same_dir(d1->d_name, d2->d_name))
               {
                  copy = false;
                  found = true;
                  char *new_parents_dir[2];
                  char *new_result_path = add_to_path(result_parent, d1->d_name, NULL, NULL);
                  new_parents_dir[index] = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
                  new_parents_dir[compare_index] = add_to_path(parent_dir[compare_index], d2->d_name, NULL, NULL);

                  //if we are searching in the first directory it is the first time we encounter the duplicate dir so we need to copy it to the result directory
                  if(index == 0)
                  {
                     CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);
                  }

                  print_differences_and_merge_rec(new_parents_dir ,index, new_result_path);

                  free_path(new_parents_dir[index]);
                  free_path(new_parents_dir[compare_index]);
                  free_path(new_result_path);
                  break;
               }
            }
         }
      }
      char* path_to_file = NULL;
      char *new_result_path = NULL;

      if(found == false || copy == true)
      {
         path_to_file = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
         new_result_path = add_to_path(result_parent, d1->d_name, NULL, NULL);
      }

      if(copy == true)
      {
         if(d1->d_type == DT_REG)
         {
            copy_file_or_hard_link(path_to_file, new_result_path);
         }
         else if(d1->d_type == DT_DIR)
         {
            CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);
         }
      }

      if(found == false)
      {
         printf("\t%s\n", path_to_file);
         if(d1->d_type == DT_DIR)
         {
            traverse_differences(path_to_file, new_result_path);
         }
      }

      free_path(path_to_file);
      free_path(new_result_path);

      found = false;
      copy = true;
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
   CALL_OR_DIE(my_mkdir(result_parent, S_IRWXU), "mkdir error", int, -1);
   char *parents_dir[2] = {parent1_dir, parent2_dir};
   printf("In %s :\n", parents_dir[0]);
   print_differences_and_merge_rec(parents_dir, 0, result_parent);
   printf("In %s :\n", parents_dir[1]);
   print_differences_and_merge_rec(parents_dir, 1, result_parent);
}