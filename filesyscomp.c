#include "filesyscomp.h"
#include "filesyshandling.h"
#include <string.h>
#include <stdlib.h>

void filesys_initilization(char *parent1_directory, char *parent2_directory)
{
   int size1 = strlen(parent1_directory) + 2;
   int size2 = strlen(parent2_directory) + 2;

   parent1_dir = malloc(size1 * sizeof(char));
   parent2_dir = malloc(size2 * sizeof(char));

   strcpy(parent1_dir, parent1_directory);
   strcpy(parent2_dir, parent2_directory);

   parent1_dir[size1 - 1] = '\0';
   parent2_dir[size2 - 1] = '\0';
   parent1_dir[size1 - 2] = '/';
   parent2_dir[size2 - 2] = '/';

   initilization(parent1_dir, parent2_dir);
}