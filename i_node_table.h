#ifndef I_NODE_TABLE_H
#define I_NODE_TABLE_H

#include <stdbool.h>

typedef struct st
{
   int i_node;
   char *path;
   struct st *next;
}i_node_node;

//return NULL if does not exists
char *get_path(int i_node, i_node_node* head);
void insert(int i_node, char* path, i_node_node** head);
i_node_node* create_table();
void delete_table(i_node_node* head);

#endif