#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

typedef struct node
{
  void* data;
  struct node* next;
}node, *node_t;

struct queue 
{
  node_t front;
  node_t rear;
  int length;
}queue;

node_t node_create(void *data)
{
  node_t temp = (node_t)malloc(sizeof(node));
  temp->data = data;
  temp->next = NULL;
  return temp;
}

queue_t queue_create(void)
{
  queue_t qt =(queue_t)malloc(sizeof(queue));
  if(!qt) return NULL; //failure when allocating the new queue.
  qt->front=qt->rear =NULL;
  qt ->length = 0;
  return qt;
}

int queue_destroy(queue_t queue)
{
  // Return -1 if queue is NULL or if queue is not empty.
  if(!queue || (queue->front!=NULL)) return -1;  
  free(queue);
  return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
  if(queue == NULL ||data==NULL) return -1;
  
  // Create a new node
  node_t temp = node_create(data);
  if(!temp) return -1;
  queue->length++;
  
  // If queue is empty, then new node is front and rear both 
  if (queue->rear == NULL) 
  { 
    queue->front = queue->rear = temp;
    return 0; 
  } 
    
  // Add the new node at the end of queue and change rear 
  queue->rear->next = temp; 
  queue->rear = temp;
  return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
  // If queue or data is null or if queue is empty, return -1. 
  if (!queue || !data || queue->front == NULL) 
    return -1; 
  queue->length--;
    
  // Store previous front and move front one node ahead 
  node_t temp = queue->front;
  *data = temp->data;
  queue->front = queue->front->next; 
  free(temp); // frees the node
  
  // If front becomes NULL, then change rear also as NULL 
  if (queue->front == NULL)
    queue->rear = NULL; 
  return 0;
}

int queue_delete(queue_t queue, void *data)
{
  // Return -1 if queue or data are NULL or data was not found in the *queue
  if (!queue || !data|| queue->front == NULL)
    return -1; 
  
  node_t temp = queue->front;
  node_t last;

  if(temp->data==data){
    queue->front = temp->next;
    if (queue->front == NULL)
    queue->rear = NULL;
    return 0;    
  }
  
  while(temp)
  {
    if((temp->data)==data)
    {
      last->next=temp->next;
      queue->length--;
      return 0;
    }
    
    last = temp;
    temp=temp->next;
  }

  // If front becomes NULL, then change rear also as NULL 
  if (queue->front == NULL)
    queue->rear = NULL; 
  return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
  if (queue == NULL || func == NULL)
    return -1;

  node_t temp = queue->front;
  int value = 0;
  while(temp != NULL)
  {
    // call func on node
    value = func(temp->data, arg);

    if (value == 1)
      break;

    temp = temp->next;
  }

  if (value==1 && data != NULL)
    *data = temp->data;

  return 0;
}

int queue_length(queue_t queue)
{
  if(!queue) return-1;
    return queue->length;
}
