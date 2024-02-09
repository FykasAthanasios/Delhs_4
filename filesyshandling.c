#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char *add_to_path(char *current_path, char *new, int *current_size, char *result)
{
   int size = strlen(current_path) + 1 + strlen(new) + 1; // current_path + / + new + \0

   if(current_size == NULL)
   {
      result = malloc(sizeof(char) * (size));
   }
   else
   {
      if((*current_size) < size)
      {
         (*current_size) = size;
         if(result != NULL) free(result);
         result = malloc(sizeof(char) * (size));
      }
   }

   if(result == NULL)
   {
      (*current_size) = size;
      result = malloc(sizeof(char) * (size));
   }
   
   strcpy(result, current_path);
   strcat(result, "/");
   strcat(result, new);

   return result;
}

bool same_dir(char* name1, char* name2)
{
   if(strcmp(name1, name2) != 0)
   {
      return false;
   }

   return true;
}

bool same_file(char* name1, char* name2, char* path1, char* path2)
{
   if(strcmp(name1, name2) != 0)
   {
      return false;
   }

   char *real_path1 = add_to_path(path1, name1, NULL, NULL);
   char *real_path2 = add_to_path(path2, name2, NULL, NULL); 

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

   while(read(fid1, &byte1, 1) != 0 && read(fid2, &byte2, 1) != 0)
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
