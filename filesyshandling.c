#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static char *parent1_dir;
static char *parent2_dir;

void initilization(char *parent1_directory, char *parent2_directory)
{
   int size1 = strlen(parent1_directory) + 1;
   int size2 = strlen(parent2_directory) + 1;

   parent1_dir = malloc(size1 * sizeof(char));
   parent2_dir = malloc(size2 * sizeof(char));

   strcpy(parent1_dir, parent1_directory);
   strcpy(parent2_dir, parent2_directory);
}

bool compare_dir(char* path1, char* path2)
{
   if(strcmp(path1, path2) != 0)
   {
      return false;
   }

   return true;
}

bool compare_file(char* path1, char* path2)
{
   if(strcmp(path1, path2) != 0)
   {
      return false;
   }

   char real_path1[strlen(path1) + strlen(parent1_dir) + 1], real_path2[strlen(path2) + strlen(parent2_dir) + 1];

   strcpy(real_path1, parent1_dir);
   strcat(real_path1, path1);
   strcpy(real_path2, parent2_dir);
   strcat(real_path2, path2);

   struct stat stat1, stat2;

   if(stat(real_path1, &stat1) == -1)
   {
      perror("stat error");
      exit(EXIT_FAILURE);
   }

   if(stat(real_path2, &stat2) == -1)
   {
      perror("stat error");
      exit(EXIT_FAILURE);
   }

   if(stat1.st_size != stat2.st_size)
   {
      return false;
   }

   int fid1 = open(real_path1, O_RDONLY | O_NONBLOCK);

   if(fid1 == -1)
   {
      perror("open failed");
      exit(EXIT_FAILURE);
   }

   int fid2 = open(real_path2, O_RDONLY | O_NONBLOCK);

   if(fid2 == -1)
   {
      perror("open failed");
      exit(EXIT_FAILURE);
   }

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
