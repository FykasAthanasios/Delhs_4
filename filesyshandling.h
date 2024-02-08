#include <stdbool.h>

static char *parent1_dir;
static char *parent2_dir;

void initilization(char *parent1_directory, char *parent2_directory);
bool compare_dir(char* path1, char* path2);
bool compare_file(char* path1, char* path2);