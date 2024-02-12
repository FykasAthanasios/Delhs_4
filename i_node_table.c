#include "i_node_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CALL_OR_DIE(f_call, error_message, Type, error_value)      \
({                                                                 \
   Type value =  f_call;                                           \
   if(value == error_value){                                       \
      perror(error_message); exit(EXIT_FAILURE);                   \
   }                                                               \
   value;                                                          \
})

static i_node_node *head;

void create_table()
{
   head = NULL;
}

void insert(int i_node, char* path)
{
   i_node_node *temp = CALL_OR_DIE(malloc(sizeof(i_node_node)), "malloc error", void *, NULL);
   temp->i_node = i_node;
   temp->path = CALL_OR_DIE(malloc(sizeof(char) * (strlen(path) + 1)), "malloc error", void *, NULL);
   strcpy(temp->path, path);
   temp->next = head;
   head = temp;
}

void delete_table()
{
   i_node_node *temp = NULL;
   if(head != NULL)
   {
      temp = head->next;
      free(head->path);
      free(head);
   }
   while(temp != NULL)
   {
      head = temp;
      temp = temp->next;
      free(head->path);
      free(head);
   }
}

char *get_path(int i_node)
{
   i_node_node *temp = head;

   while(temp != NULL)
   {
      if(temp->i_node == i_node)
      {
         return temp->path;
      }
      temp = temp->next;
   }

   return NULL;
}