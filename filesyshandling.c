#include "filesyshandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

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

void copy_file_or_hard_link(char *path, char *new_path, i_node_node** table)
{
   if(new_path == NULL) return;
   struct stat stat1;

   CALL_OR_DIE(stat(path, &stat1), "stat error", int, -1);
   //check the file has more than one link
   if(stat1.st_nlink > 1)
   {
      //check if the i node is inside the table
      char *original_new_path = get_path(stat1.st_ino, *table);
      //if it is not put it there and copy the file
      if(original_new_path == NULL)
      {
         insert(stat1.st_ino, new_path, table);
         copy_file(path, new_path);
      }
      //if it is not create a hard link to the file that was inserted inside the table
      else
      {
         CALL_OR_DIE(link(original_new_path, new_path), "link error", int, -1);
      }
   }
   //if it only has 1 has link just copy the file
   else
   {
      copy_file(path, new_path);
   }
}

typedef struct
{
   int start;
   int end;
}Points;

//find the string inside the other string and return the index where it starts and where it ends
Points find_string_in_string(const char* s, const char* string_to_find)
{
   int len = strlen(string_to_find);
   Points p;
   for(int i = 0 ; s[i] != '\0' ; i++)
   {
      //if you find the first character check if the the next characters are the same
      if(s[i] == string_to_find[0])
      {
         bool found = true;
         p.start = i;
         int j = i + 1;
         for(j = i + 1 ; s[j] != '\0' && (j - i) < len; j++)
         {
            if(s[j] != string_to_find[j - i])
            {
               found = false;
            }
         }
         if(found == true)
         {
            p.end = j - 1;
            return p;
         }
      }
   }
   //if the string is not found return -1
   return (Points){.start = -1, .end = -1};
}

void copy_link(char *path, char *new_path)
{
   if(new_path == NULL) return;

   char result_parent_dir[256];
   char original_parent[256];

   //find the parents of the old path and the new path 
   int len1, len2;
   for(len1 = 0, len2 = 0; new_path[len1] != '/' &&  path[len2] != '/' ; len1++, len2++)
   {
      if(new_path[len1] == '/')
      {
         len1--;
      }
      else
      {
         result_parent_dir[len1] = new_path[len1];
      }
      
      if(path[len2] == '/')
      {
         len2--;
      }
      else
      {
         original_parent[len2] = path[len2];
      }
   }

   result_parent_dir[len1] = '\0';
   original_parent[len2] = '\0';

   char * target = NULL;
   ssize_t size = 0;
   ssize_t len = 0;
   //find where the link is pointing
   do
   {
      free_path(target);
      size += 64;
      target = CALL_OR_DIE(malloc(size * sizeof(char)), "malloc error", char*, NULL);
      len = CALL_OR_DIE(readlink(path, target, size - 1), "readlink error", ssize_t, -1);
   }while((len == size -1));
   target[len] = '\0';

   //create a new path path for the link to point to by changing the path for example /home/dirA/dirRandom/link -> /home/dirC/dirRandom/link
   char *new_path_to_file = CALL_OR_DIE(malloc(((len + 1) - len2 + len1 + 1) * sizeof(char)), "malloc error", void *, NULL);
   Points p = find_string_in_string(target, original_parent);

   for(int i = 0 ; i < p.start ; i++)
   {
      new_path_to_file[i] = target[i];
   }
   new_path_to_file[p.start] = '\0';
   
   strcat(new_path_to_file, result_parent_dir);
   strcat(new_path_to_file, (target + p.end + 1));

   free_path(target);
   //create the soft link in the new path
   CALL_OR_DIE(symlink(new_path_to_file, new_path), "symlink error", int, -1);
   free_path(new_path_to_file);
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

int compare_file1_modif_with_file2_modif(char *name1, char *name2, char *path1, char *path2)
{
   char *real_path1 = add_to_path(path1, name1, NULL, NULL);
   char *real_path2 = add_to_path(path2, name2, NULL, NULL);

   struct stat stat1, stat2;

   CALL_OR_DIE(stat(real_path1, &stat1), "stat error", int, -1);
   CALL_OR_DIE(stat(real_path2, &stat2), "stat error", int, -1);

   int result = 0;
   if(stat1.st_mtime == stat2.st_mtime)
   {
      result = 1;
   }
   else if(stat1.st_mtime < stat2.st_mtime)
   {
      result = 2;
   }

   free(real_path1);
   free(real_path2);

   return result;
}

bool is_link_target_a_link(const char* path)
{
   struct stat statbuf;
   
   CALL_OR_DIE(lstat(path, &statbuf), "lstat error", int, -1);

   if (S_ISLNK(statbuf.st_mode)) {
      return true; // The path is a symbolic link
   } else {
      return false; // The path is not a symbolic link
   }
}

void getLastPathComponent(const char* path, int length, char* lastComponent, int bufSize) {
   if (bufSize == 0) {
      return;
   }

   if (length == 0) {
      strcpy(lastComponent, "");
      return;
   }
   
   int slash_index = 0;
   for(int i = length - 1; i >= 0 ; i--)
   {
      if(path[i] == '/')
      {
         slash_index = i;
         break;
      }
   }

   for(int i = slash_index + 1, j = 0 ; i < length ; i++, j++)
   {
      if(j >= bufSize)
      {
         printf("out of bounds error\n");
         exit(EXIT_FAILURE);
      }
      lastComponent[j] = path[i];
   }
}

void removeLastComponent(char *path) {
   int len = strlen(path);

   for (int i = len - 1; i >= 0; i--) {
      if (path[i] == '/' || path[i] == '\\') { 
         path[i] = '\0';
         break;
      }
   }
}

//same as same link function
bool same_link_rec(char* name1, char* name2, char* path1, char* path2)
{
   if(strcmp(name1, name2) != 0)
   {
      return false;
   }

   char * target1=NULL;
   char * target2=NULL;
   ssize_t size1 = 0;
   ssize_t len1=0;

   //do the following loop until size1 is large enough to fit the string, 
   //target will contain the target of link 1
   do
   {
      free(target1);
      size1 +=64;
      target1=CALL_OR_DIE(calloc(size1, sizeof(char)), "malloc error", char*, NULL);
      len1=CALL_OR_DIE(readlink(path1, target1, size1-1), "readlink error", ssize_t, -1);
   }while((len1 == size1 -1));
   target1[len1] = '\0';

   //If target 2 doesnt fit withing lenght1 ,then its not the same
   target2=CALL_OR_DIE(calloc(len1 + 1, sizeof(char)), "malloc error", char*, NULL);
   if(CALL_OR_DIE(readlink(path2, target2, len1+1), "readlink error" , ssize_t , -1)== len1+1)
   {    
      target2[len1] = '\0';
      free(target1);
      free(target2);
      return false;
   }
   target2[len1] = '\0';

   //Check if both links , look to a link, and call the same function recursively with the new paths
   if( (is_link_target_a_link(target1) == true) && (is_link_target_a_link(target2) == true)) 
   {
      char* tempname1=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
      char* tempname2=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
      getLastPathComponent(target1, len1 + 1, tempname1, 256);
      getLastPathComponent(target2, len1 + 1, tempname2, 256);
      if(same_link_rec( tempname1 , tempname2, target1, target2) == true) 
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
   
   char* tempname1=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
   char* tempname2=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
   getLastPathComponent(target1, len1 + 1, tempname1, 256);
   getLastPathComponent(target2, len1 + 1, tempname2, 256);
   //Check if the name of files that the links look to are the same, then call the 
   //same file function to isnure that they are the considered the "same"
   if (strcmp(tempname1, tempname2) == 0) {    
      removeLastComponent(target1);
      removeLastComponent(target2);
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

bool same_link(char* name1, char* path1, char* path2)
{
   char *real_path1 = add_to_path(path1, name1, NULL, NULL);
   char *real_path2 = add_to_path(path2, name1, NULL, NULL); 
   char * target1=NULL;
   char * target2=NULL;
   ssize_t size1 = 0;
   ssize_t len1=0;

   //do the following loop until size1 is large enough to fit the string, 
   //target will contain the target of link 1
   do
   {
      free(target1);
      size1 +=64;
      target1=CALL_OR_DIE(calloc(size1, sizeof(char)), "malloc error", char*, NULL);
      len1=CALL_OR_DIE(readlink(real_path1, target1, size1-1), "readlink error", ssize_t, -1);
   }while((len1 == size1 -1));

   target1[len1] = '\0';
   //If target 2 doesnt fit withing lenght1 ,then its not the same
   target2=CALL_OR_DIE(calloc(len1 + 1, sizeof(char)), "malloc error", char*, NULL);
   if(CALL_OR_DIE(readlink(real_path2, target2, len1+1), "readlink error" , ssize_t , -1)== len1+1)
   {    
      target2[len1] = '\0';
      free(target1);
      free(target2);
      free(real_path1);
      free(real_path2);
      return false;
   }
   target2[len1] = '\0';
   free(real_path1);
   free(real_path2);
   //Check if both links , look to a link, and call the same function recursively with the new paths
   if( (is_link_target_a_link(target1) == true) && (is_link_target_a_link(target2) == true)) 
   {
      char* tempname1=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
      char* tempname2=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
      getLastPathComponent(target1, len1 + 1, tempname1, 256);
      getLastPathComponent(target2, len1 + 1, tempname2, 256);
      if(same_link_rec( tempname1 , tempname2, target1, target2) == true) 
      {
         free(tempname1);
         free(tempname2);
         free(target1);
         free(target2);
         return true;
      }
      free(tempname1);
      free(tempname2);
      free(target1);
      free(target2);
      return false;

   }
   char* tempname1=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
   char* tempname2=CALL_OR_DIE(calloc(256, sizeof(char)), "malloc error", char*, NULL);
   getLastPathComponent(target1, len1 + 1, tempname1, 256);
   getLastPathComponent(target2, len1 + 1, tempname2, 256);
  
   //Check if the name of files that the links look to are the same, then call the 
   //same file function to isnure that they are the considered the "same"
   if (strcmp(tempname1, tempname2) == 0) {      
      
      //remove the last component of the path so same file function works as intended
      removeLastComponent(target1);
      removeLastComponent(target2);
      bool result= same_file(tempname1, target1, target2);
      free(tempname1);
      free(tempname2);
      free(target1);
      free(target2);
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

