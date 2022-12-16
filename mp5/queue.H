/*
 
 File name: queue.H
 Author: Amit Patil
 Description: A simple queue of threads
 
 */

#define NULL 0
#include "thread.H"
//#include "console.H"

struct node
{
      Thread* thread;
      node *next;
};

class queue
{
      node *rear,*front;
public:
      queue()
      { rear=NULL;
	front=NULL;
//	Console::puts("thread queue initialized\n");
      }



void enqueue(Thread * _thread)
{
      node *temp;
      temp=new node;
      temp->next=NULL;
      temp->thread = _thread;
      if(rear==NULL)
      {
            rear=temp;
            front=temp;
      }
      else
      {
            rear->next=temp;
            rear=temp;
      }
  //        Console::puts("thread queued\n");
}

Thread* dequeue()
{
      if(front!=NULL)
      {
            node *temp=front;
            front=front->next;
            //delete temp;
            if(front==NULL)
                  rear=NULL;
      
    //        Console::puts("thread removed\n");
            return temp->thread;
      } else {
      //	Console::puts("queue is empty!nothing to remove\n");
	return NULL;
      } 
}

};
