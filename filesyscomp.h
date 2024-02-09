#ifndef FILESYSCOMP_H
#define FILESYSCOMP_H

/*
 * if result_parent == NULL then it will only print the differences betwen the 2 file systems
 * if the result parent contains a string the it will print the difference and at the same time it
 * will create the new filesystem inside the result parent directory
 */
void print_differences_and_merge(char *parent_dir1, char *parent_dir2, char *result_parent);

#endif