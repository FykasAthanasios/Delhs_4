#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <dirent.h>

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

bool file1_modif_less_file2_modif(char *name1, char *name2, char *path1, char *path2)
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

bool same_link(char* name1, char* name2, char* path1, char* path2)
{
   if(strcmp(name1, name2) != 0)
   {
      return false;
   }
   DIR* dirptr=CALL_OR_DIE(opendir("./"), "opendir error", DIR*, NULL);
   int dirfid=CALL_OR_DIE(dirfd(dirptr),"dirfd error", int , -1);

   char * target1=NULL;
   char * target2=NULL;
   ssize_t size1 = 0, size2=0;
   ssize_t len1=0;

   do
   {
      free(target1);
      size1 +=1024;
      target1=CALL_OR_DIE(malloc(size1*sizeof(char)), "malloc error", char*, NULL);
      len1=CALL_OR_DIE(readlinkat(dirfid, path1, target1, size1-1), "readlink error", ssize_t, -1);
   }while((len1 == size2 -1));

   target1[len1] = '\0';

   target2=CALL_OR_DIE(malloc(sizeof(char)*(len1+1)), "malloc error" , void*, NULL);
   if(CALL_OR_DIE(readlinkat(dirfid, path2, target2, len1+1), "readlink error" , ssize_t , -1)== len1+1)
   {    
      target2[len1] = '\0';
      free(target1);
      free(target2);
      if(closedir(dirptr) == -1)
      {
         perror("closedir error");
      }
      return false;
   }
   target2[len1] = '\0';
   if(closedir(dirptr) == -1)
   {
      perror("closedir error");
   }

   //Check if both links , look to a link, and call the same function rec with the new paths
   if( is_link_target_a_link(target1) == true , is_link_target_a_link(target2) == true) 
   {
      char* tempname1=malloc(sizeof(char)*20);
      char* tempname2=malloc(sizeof(char)*20);
      getLastPathComponent(target1, tempname1, 20);
      getLastPathComponent(target2, tempname2, 20);
      if(same_link( tempname1 , tempname2, target1, target2) == true) 
      {
         free(tempname1);
         free(tempname2);
         free(target1);
         free(target2);
         return true;
      }
      free(tempname1);
      free(tempname2);
   }
    
   char* tempname1=CALL_OR_DIE(malloc(sizeof(char)*20), "malloc error", void* , NULL);
   char* tempname2=CALL_OR_DIE(malloc(sizeof(char)*20), "malloc error", void* , NULL);
   getLastPathComponent(target1, tempname1, 20);
   getLastPathComponent(target2, tempname2, 20);
   if (strcmp(tempname1, tempname2) == 0) {     
      bool result= same_file(tempname1, target1, target2);
      free(tempname1);
      free(tempname2);
      if ( result == false)
      {
         return false;
      }
      return true; // The links point to the same target
   } else {
      free(target1);
      free(target2);
      free(tempname1);
      free(tempname2);
      return false; // The links do not point to the same target
   }
} 

bool is_link_target_a_link(const char* path)
{
   struct stat statbuf;
   printf("%s\n", path);
   CALL_OR_DIE(lstat(path, &statbuf), "lstat error", int, -1);

   if (S_ISLNK(statbuf.st_mode)) {
      return true; // The path is a symbolic link
   } else {
      return false; // The path is not a symbolic link
   }
}

void getLastPathComponent(const char* path, char* lastComponent, int bufSize) {
   if (path == NULL || lastComponent == NULL || bufSize == 0) {
      return;
   }
   int length = strlen(path);
   if (length == 0) {
      strncpy(lastComponent, "", bufSize);
      return;
   }
   //move the pointer to the last character
   const char* p = path + length - 1;

   //Find the start of the last component of the path
   const char* end = p;
   while (p > path && *(p - 1) != '/') {
      --p;
   }

   int lastLen = end - p + 1;

   if (lastLen < bufSize) {
      memcpy(lastComponent, p, lastLen);
      lastComponent[lastLen] = '\0';
   }
}
