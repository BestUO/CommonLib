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

//thread safe
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
			tail = m_ptail;
		}while(!__sync_bool_compare_and_swap(&tail->nextnode,NULL,newnode));
		__sync_bool_compare_and_swap(&m_ptail,tail,newnode);
		return true;
	}
	void *popMSG()
	{
		Node *head = NULL;
		do
		{
			head = m_phead;
			if(!head->nextnode)
				return nullptr;
		}while(!__sync_bool_compare_and_swap(&m_phead,head,head->nextnode));
		void *value = head->nextnode->value;
		QueuePool.deleteElement(head);
		return value;
	}
private:
	struct Node
	{
		Node *nextnode;
		void *value;
		Node()
		{
			nextnode = NULL;
			value = NULL;
		}
	};
	Node *m_phead;
	Node *m_ptail;
	MemoryPool<Node,4096> QueuePool;
	void InitQueue()
	{
		m_phead = QueuePool.newElement();
	}
	void FreeQueue()
	{
		QueuePool.deleteElement(m_phead);
		QueuePool.freeMemoryPool();
	}
};


#define QUEUESIZE 0x10000// releate to uint16_t head in Struct Queue
#define QUEUEMask (QUEUESIZE - 1)
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

	bool pushnMSGs(void **p,uint16_t num,bool single_pro)
	{
		uint16_t head = 0;
		uint16_t tail = 0;
		uint16_t free_entries = 0;
		bool success = false;
		do
		{
			head = Freelock_Queue->head;
			tail = Freelock_Queue->tail;
			free_entries = tail + QUEUEMask - head;
			if(free_entries < num)
				return false;
			if(single_pro)
				Freelock_Queue->head = head+num , success = true;
			else
				success = __sync_bool_compare_and_swap(&Freelock_Queue->head, head, head+num);
		}while(!unlikely(success));
		for(int i = 0; i < num; i++)
			Freelock_Queue->q[head+i] = p[i];
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

	uint16_t popnMSGs(void **p,uint16_t num,bool single_con)
	{
		uint16_t head = 0;
		uint16_t tail = 0;
		uint16_t entries = 0;
		bool success = false;
		do
		{
			head = Freelock_Queue->head;
			tail = Freelock_Queue->tail;
			entries = head - tail > num ? num : head - tail;
			if(entries == 0)
				return entries;
			if(single_con)
				Freelock_Queue->tail = tail+entries , success = true;
			else
				success = __sync_bool_compare_and_swap(&Freelock_Queue->tail, tail, tail+entries);
		}while(!unlikely(success));
		for(int i = 0; i < entries; i++)
		{
			p[i] = Freelock_Queue->q[tail+i];
			Freelock_Queue->q[tail+i] = NULL;
		}
		return entries;
	}

private:
	struct Queue
	{
		volatile uint16_t head;
		volatile uint16_t tail;
		void *q[QUEUESIZE];
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
