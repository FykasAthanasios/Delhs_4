#include "filesyscomp.h"
#include "filesyshandling.h"
#include "i_node_table.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
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

/*
 *function call called when we are in a directory that exists only in one of the two file system tree we examine
 *it prints all the object inside the file recursivly and adds them in the result dirctery if it is not NULL
*/
void traverse_differences(char *path, char *result_parent, i_node_node** table)
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
   //read every object in the directory
   while((d = readdir(direct)) != NULL)
   {
      new_path = add_to_path(path, d->d_name, &path_len, new_path);
      //if the object is a directory and its not . or ..
      if(d->d_type == DT_DIR && !check_to_ignore_dir(d->d_name))
      {
         new_result_path = add_to_path(result_parent, d->d_name, &result_path_len, new_result_path);
         CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);

         printf("\t%s\n", new_path);
         traverse_differences(new_path, new_result_path, table);
      }
      //if it is a file or a hard link
      else if(d->d_type == DT_REG)
      {
         new_result_path = add_to_path(result_parent, d->d_name, &result_path_len, new_result_path);
         copy_file_or_hard_link(new_path, new_result_path, table);
         printf("\t%s\n", new_path);
      }
      //if it s a soft link
      else if(d->d_type == DT_LNK)
      {
         new_result_path = add_to_path(result_parent, d->d_name, &result_path_len, new_result_path);
         copy_link(new_path, new_result_path);
         printf("\t%s\n", new_path);
      }
   }

   if(closedir(direct) == -1)
   {
      perror("closedir error");
   }

   free_path(new_path);
   free_path(new_result_path);
}

/*
 *if index is 0 it means we are traversing the first directory
 *if index is 1 , the second
 *traverse the first or the second file system tree and find and print the differences 
 *if result_parent is not NULL then merge the trees
 *if we need to merge we add the object that exists in both trees only when we traverse the first tree (index = 0) so we dont add it 2 times
 */
void print_differences_and_merge_rec(char **parent_dir, int index, char *result_parent, i_node_node** table)
{
   int compare_index = 0;
   if(index == 0)
      compare_index = 1;

   DIR *dir[2];

   dir[0] = CALL_OR_DIE(opendir(parent_dir[0]), "opendir error", DIR*, NULL);
   dir[1] = CALL_OR_DIE(opendir(parent_dir[1]), "opendir error", DIR*, NULL);

   struct dirent *d1;
   //check every object in the directory if it exists in the same relative path in the other directory
   while((d1 = readdir(dir[index])) != NULL)
   {
      if(check_to_ignore_dir(d1->d_name)) continue;

      bool found = false;
      bool copy = true;

      struct dirent *d2;
      while((d2 = readdir(dir[compare_index])) != NULL)
      {
         //if it is the same type (dir or link or file)
         if(d1->d_type == d2->d_type)
         {
            //if it is a file or a hard link
            if(d1->d_type == DT_REG)
            {
               //if tehy have the same name
               if(strcmp(d1->d_name, d2->d_name) == 0)
               {
                  //if it is the same file or hard link
                  if(same_file(d1->d_name, parent_dir[index], parent_dir[compare_index]))
                  {
                     //if we are not traversing the first directory then dont copy the file in the result (it arleady exists)
                     if(index != 0)
                     {
                        copy = false;
                     }
                     //mark it as found
                     found = true;
                     break;
                  }
                  //if they have the same name but they are not the same file we need when it was last modified
                  else
                  {
                     int result = compare_file1_modif_with_file2_modif(d1->d_name, d2->d_name, parent_dir[index], parent_dir[compare_index]);
                     if(result == 1 && index == 1)
                     {
                        copy = false;
                     }
                     else if(result == 2)
                     {
                        copy = false;
                     }                 
                  }
               }
            }
            else if(d1->d_type == DT_DIR)
            {
               //ignore . and .. directories
               if(check_to_ignore_dir(d2->d_name)) continue;

               if(same_dir(d1->d_name, d2->d_name))
               {
                  copy = false; //dont copy the directory bellow ether way as we do it in this section
                  //mark the directory as found
                  found = true;

                  char *new_parents_dir[2];
                  char *new_result_path = add_to_path(result_parent, d1->d_name, NULL, NULL);
                  new_parents_dir[index] = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
                  new_parents_dir[compare_index] = add_to_path(parent_dir[compare_index], d2->d_name, NULL, NULL);

                  //if we are traversing the first directory it copy it if we are not dont copy it as it arleady exists
                  if(index == 0)
                  {
                     CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);
                  }
                  //call this functyion recursivly to check the directory content
                  print_differences_and_merge_rec(new_parents_dir ,index, new_result_path, table);

                  free_path(new_parents_dir[index]);
                  free_path(new_parents_dir[compare_index]);
                  free_path(new_result_path);
                  break;
               }
            }
            //if they are soft links
            else if(d1->d_type == DT_LNK)
            {
               //do the same as the file but we different compare functions
               if(strcmp(d1->d_name, d2->d_name) == 0)
               {
                  if(same_link(d1->d_name, parent_dir[index], parent_dir[compare_index]))
                  {
                     if(index != 0)
                     {
                        copy = false;
                     }
                     found = true;
                     break;
                  }
                  else
                  {
                     int result = compare_link1_modif_with_link2_modif(d1->d_name, d2->d_name, parent_dir[index], parent_dir[compare_index]);
                     if(result == 1 && index == 1)
                     {
                        copy = false;
                     }
                     else if(result == 2)
                     {
                        copy = false;
                     }   
                  }
               }
            }
         }
      }
      char* path_to_file = NULL;
      char *new_result_path = NULL;
      //if this object is present in both directories we examine or if we set above this object to be copied
      if(found == false || copy == true)
      {
         path_to_file = add_to_path(parent_dir[index], d1->d_name, NULL, NULL);
         new_result_path = add_to_path(result_parent, d1->d_name, NULL, NULL);
      }
      //if we set above this object to be copied
      if(copy == true)
      {
         if(d1->d_type == DT_REG)
         {
            copy_file_or_hard_link(path_to_file, new_result_path, &table[index]);
         }
         else if(d1->d_type == DT_DIR)
         {
            CALL_OR_DIE(my_mkdir(new_result_path, S_IRWXU), new_result_path, int, -1);
         }
         else if(d1->d_type == DT_LNK)
         {
            copy_link(path_to_file, new_result_path);
         }
      }
      //if this object is not present in both directories print it as different and call the traverse_differences
      if(found == false)
      {
         printf("\t%s\n", path_to_file);
         if(d1->d_type == DT_DIR)
         {
            traverse_differences(path_to_file, new_result_path, &table[index]);
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
   //create a table with hard links for both directories
   i_node_node* table[2] = {create_table(), create_table()};
   //create the result directory if it is not null
   CALL_OR_DIE(my_mkdir(result_parent, S_IRWXU), "mkdir error", int, -1);

   char *parents_dir[2] = {parent1_dir, parent2_dir};
   //travese the first file sys tree
   printf("In %s :\n", parents_dir[0]);
   print_differences_and_merge_rec(parents_dir, 0, result_parent, table);
   //traverse the second file sys tree
   printf("In %s :\n", parents_dir[1]);
   print_differences_and_merge_rec(parents_dir, 1, result_parent, table);
   //free i node table
   delete_table(table[0]);
   delete_table(table[1]);
}
