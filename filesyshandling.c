#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char *add_to_path(char *current_path, char *new, int *current_size, char *result)
{
   if(current_path == NULL)
   {
      return NULL;
   }
   int size = strlen(current_path) + 1 + strlen(new) + 1; // current_path + / + new + \0

   if(current_size == NULL)
   {
      result = CALL_OR_DIE(malloc(sizeof(char) * (size)), "malloc error", void *, NULL);
   }
   else
   {
      if((*current_size) < size)
      {
         (*current_size) = size;
         if(result != NULL) free(result);
         result = CALL_OR_DIE(malloc(sizeof(char) * (size)), "malloc error", void *, NULL);
      }
   }

   if(result == NULL)
   {
      (*current_size) = size;
      result = CALL_OR_DIE(malloc(sizeof(char) * (size)), "malloc error", void *, NULL);
   }
   
   strcpy(result, current_path);
   strcat(result, "/");
   strcat(result, new);

   return result;
}

void free_path(char *path)
{
   if(path != NULL)
      free(path);
}

int my_mkdir(char *path, mode_t mode)
{
   if(path != NULL)
   {
      return mkdir(path, mode);
   }
   return 0;
}

int my_creat(char *path, mode_t mode)
{
   if(path != NULL)
   {
      return creat(path, mode);
   }
   return 0;
}

void copy_file(char *path, char *new_path)
{
   if(new_path == NULL)
   {
      return;
   }

   int fid2 = CALL_OR_DIE(my_creat(new_path, S_IRWXU), "create error", int, -1);

   int fid1 = CALL_OR_DIE(open(path, O_RDONLY | O_NONBLOCK), "open error", int, -1);

   char byte;

   while(CALL_OR_DIE(read(fid1, &byte, 1), "read error", int, -1) != 0)
   {
      write(fid2, &byte, 1);
   }

   if(close(fid1) == -1)
   {
      perror("close error");
   }

   if(close(fid2) == -1)
   {
      perror("close error");
   }
}

void copy_file_or_hard_link(char *path, char *new_path)
{
   copy_file(path, new_path);
}

bool same_dir(char* name1, char* name2)
{
   if(strcmp(name1, name2) != 0)
   {
      return false;
   }

   return true;
}

bool same_file(char* name, char* path1, char* path2)
{
   char *real_path1 = add_to_path(path1, name, NULL, NULL);
   char *real_path2 = add_to_path(path2, name, NULL, NULL); 

   struct stat stat1, stat2;

   CALL_OR_DIE(stat(real_path1, &stat1), "stat error", int, -1);
   CALL_OR_DIE(stat(real_path2, &stat2), "stat error", int, -1);

   if(stat1.st_size != stat2.st_size)
   {
      free(real_path1);
      free(real_path2);
      return false;
   }

   int fid1 = CALL_OR_DIE(open(real_path1, O_RDONLY | O_NONBLOCK), "open error", int, -1);

   int fid2 = CALL_OR_DIE(open(real_path2, O_RDONLY | O_NONBLOCK), "open error", int, -1);

   free(real_path1);
   free(real_path2);

   char byte1, byte2;

   while(CALL_OR_DIE(read(fid1, &byte1, 1), "read error", int, -1) != 0 && CALL_OR_DIE(read(fid2, &byte2, 1), "read error", int, -1) != 0)
   {
      if(byte1 != byte2)
      {
         if(close(fid1) == -1)
         {
            perror("close error");
         }

         if(close(fid2) == -1)
         {
            perror("close error");
         }

         return false;
      }
   }

   if(close(fid1) == -1)
   {
      perror("close error");
   }

   if(close(fid2) == -1)
   {
      perror("close error");
   }

   return true;
}

bool file1_modif_bigger_file2_modif(char *name1, char *name2, char *path1, char *path2)
{
   char *real_path1 = add_to_path(path1, name1, NULL, NULL);
   char *real_path2 = add_to_path(path2, name2, NULL, NULL);

   struct stat stat1, stat2;

   CALL_OR_DIE(stat(real_path1, &stat1), "stat error", int, -1);
   CALL_OR_DIE(stat(real_path2, &stat2), "stat error", int, -1);

   bool result = !(stat1.st_mtime >= stat2.st_mtime);

   free(real_path1);
   free(real_path2);

   return result;
}