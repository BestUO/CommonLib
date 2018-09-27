#ifndef QUEUE_H_UO
#define QUEUE_H_UO

#include <stdint.h>
#include <cstring>
#include <iostream>
#include "UO_MemoryPool.h"

class Stack
{
public:
	Stack() {}
	~Stack() {}
};

class UO_Queue
{
public:
	UO_Queue() {InitQueue();}
	~UO_Queue() {FreeQueue();}
	bool pushMSG(void *p)
	{
		Node *newnode = QueuePool.newElement();
		newnode->value = p;
		Node *tail = NULL;
		do
		{
			tail = root->prenode;
		}while(!__sync_bool_compare_and_swap(&tail->nextnode,NULL,newnode));
		__sync_bool_compare_and_swap(&root->prenode,tail,newnode);
		return true;
	}
	void *popMSG()
	{
		Node *head = NULL;
		do
		{
			head = root->nextnode;
			if(!head)
				return nullptr;
		}while(!__sync_bool_compare_and_swap(&root->nextnode,head,head->nextnode));
		void *value = head->value;
		QueuePool.deleteElement(head);
		return value;
	}
private:
	struct Node
	{
		Node *prenode;
		Node *nextnode;
		void *value;
		Node()
		{
			prenode = NULL;
			nextnode = NULL;
			value = NULL;
		}
	};
	Node *root;
	MemoryPool<Node,4096> QueuePool;
	void InitQueue()
	{
		root = QueuePool.newElement();
		root->prenode = root;
	}
	void FreeQueue()
	{
		QueuePool.deleteElement(root);
		QueuePool.freeMemoryPool();
	}
};


#define QueueSize 0x10000// releate to uint16_t head in Struct Queue
#define Mask (QueueSize - 1)
class UO_RingQueue
{
public:
	UO_RingQueue() {InitRingQueue();}
	~UO_RingQueue() {FreeRingQueue();};
	
	bool pushMSG(void *p)
	{
		uint16_t head = 0;
		do
		{
			head = Freelock_Queue->head;
			if(Freelock_Queue->q[head])
				return false;
		}while(!__sync_bool_compare_and_swap(&Freelock_Queue->head, head, head+1));
		Freelock_Queue->q[head] = p;
		return true;
	}

	void *popMSG()
	{
		uint16_t tail = 0;		
		void *p = NULL;
		do
		{
			tail = Freelock_Queue->tail;
			if(!Freelock_Queue->q[tail])
				return NULL;
		}while(!__sync_bool_compare_and_swap(&Freelock_Queue->tail, tail, tail+1));
		p = Freelock_Queue->q[tail];
		Freelock_Queue->q[tail] = NULL;
		return p;
	}

private:
	struct Queue
	{
		volatile uint16_t head;
		volatile uint16_t tail;
		void *q[QueueSize];
	};
	Queue *Freelock_Queue;

	void InitRingQueue()
	{
		Freelock_Queue = new Queue;
		memset(Freelock_Queue,0,sizeof(Queue));
	}
	void FreeRingQueue()
	{
		if(Freelock_Queue)
			delete Freelock_Queue;
	}
};

#endif