#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
   char *dirA = NULL, *dirB = NULL,  *dirC = NULL;
   
   if(argc != 4 && argc != 6)
   {
      printf("Wrong number of arguments\n");
      printf("Arguments : ./cmpcat -d dirA DirB -s DirC or  ./cmpcat -d dirA DirB\n");
      return EXIT_FAILURE;
   }

   if(argc == 4)
   {
      if(strcmp(argv[1], "-d") != 0)
      {
         printf("Wrong flag argument\n");
         printf("Arguments : ./cmpcat -d dirA DirB -s DirC or  ./cmpcat -d dirA DirB\n");
         return EXIT_FAILURE;
      }
      dirA = argv[2];
      dirB = argv[3];
   }
   else
   {
      int next_index;
      if(strcmp(argv[1], "-d") == 0)
      {
         dirA = argv[2];
         dirB = argv[3];
         next_index = 4;
      }
      else if(strcmp(argv[1], "-s") == 0)
      {
         dirC = argv[2];
         next_index = 3;
      }
      else
      {
         printf("Wrong flag argument\n");
         printf("Arguments : ./cmpcat -d dirA DirB -s DirC or  ./cmpcat -d dirA DirB\n");
         return EXIT_FAILURE;
      }

      if(strcmp(argv[next_index], "-d") == 0)
      {
         dirA = argv[next_index + 1];
         dirB = argv[next_index + 2];
      }
      else if(strcmp(argv[next_index], "-s") == 0)
      {
         dirC = argv[next_index + 1];
      }
      else
      {
         printf("Wrong flag argument\n");
         printf("Arguments : ./cmpcat -d dirA DirB -s DirC or  ./cmpcat -d dirA DirB\n");
         return EXIT_FAILURE;
      }
   }

   return 0;
}