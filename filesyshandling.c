#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CALL_OR_DIE(f_call, error_message, Type, error_value)      \
({                                                                 \
   Type value =  f_call;                                           \
   if(value == error_value){                                       \
      perror(error_message); exit(EXIT_FAILURE);                   \
   }                                                               \
   value;                                                          \
})

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

   struct stat stat1, stat2;

   CALL_OR_DIE(stat(path1, &stat1), "stat error", int, -1);
   CALL_OR_DIE(stat(path2, &stat2), "stat error", int, -1);

   if(stat1.st_size != stat2.st_size)
   {
      return false;
   }

   int fid1 = CALL_OR_DIE(open(path1, O_RDONLY | O_NONBLOCK), "open error", int, -1);

   int fid2 = CALL_OR_DIE(open(path2, O_RDONLY | O_NONBLOCK), "open error", int, -1);

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

   return true;
}
