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

class UO_RingQueue
{
public:
	UO_RingQueue(uint32_t size, bool is_single=false):m_is_single(is_single) {InitRingQueue(size);}
	~UO_RingQueue() {FreeRingQueue();};
	
	bool pushnMSGs(void **p,uint32_t num)
	{
		uint32_t oldhead = 0;
		uint32_t newhead = 0;
		bool success = false;
		do
		{
			oldhead = producer.first;
			uint32_t tail = consumer.second;
			uint32_t free_entries = tail + m_mask - oldhead;
			if(unlikely(free_entries < num) || free_entries > m_mask)
				return false;
			newhead = oldhead + num;
			if(m_is_single)
				producer.first = newhead, success = true;
			else
				success = __sync_bool_compare_and_swap(&producer.first, oldhead, newhead);
		}while(!unlikely(success));

		uint32_t idx = oldhead & m_mask;
		for(uint32_t i = 0; i < num; i++)
		{
			uint32_t idx2 = (idx + i) & m_mask;
			Freelock_Queue[idx2] = p[i];
		}

		do
		{	
			if(m_is_single)
				producer.second = newhead, success = true;
			else
				success = __sync_bool_compare_and_swap(&producer.second, oldhead, newhead);
		}while(!unlikely(success));

		return true;
	}
	
	uint32_t popnMSGs(void **p,uint32_t num)
	{
		uint32_t oldtail = 0;
		uint32_t newtail = 0;
		uint32_t entries = 0;
		bool success = false;
		do
		{
			uint32_t head = producer.second;
			oldtail = consumer.first;
			entries = head - oldtail > num ? num : head - oldtail;
			if(entries == 0)
				return entries;
			newtail = oldtail + entries;
			if(m_is_single)
				consumer.first = newtail , success = true;
			else
				success = __sync_bool_compare_and_swap(&consumer.first, oldtail, newtail);
		}while(!unlikely(success));

		uint32_t idx = oldtail & m_mask;
		for(uint32_t i = 0; i < entries; i++)
		{	
			uint32_t idx2 = (idx + i) & m_mask;
			p[i] = Freelock_Queue[idx2];
			Freelock_Queue[idx2] = nullptr;
		}

		do
		{	
			if(m_is_single)
				consumer.second = newtail, success = true;
			else
				success = __sync_bool_compare_and_swap(&consumer.second, oldtail, newtail);
		}
		while(!unlikely(success));

		return entries;
	}
	
	bool pushMSG(void *p)
	{
		uint32_t head = 0;
		uint32_t idx = 0;
		do
		{
			head = producer.first;
			idx = head & m_mask;
			if(Freelock_Queue[idx])
				return false;
		}while(!__sync_bool_compare_and_swap(&producer.first, head, head+1));
		Freelock_Queue[idx] = p;
		return true;
	}

	void *popMSG()
	{
		uint32_t tail = 0;
		uint32_t idx = 0;
		void *p = nullptr;
		do
		{
			tail = consumer.first;
			idx = tail & m_mask;
			if(!Freelock_Queue[idx])
				return nullptr;
		}while(!__sync_bool_compare_and_swap(&consumer.first, tail, tail+1));
		p = Freelock_Queue[idx];
		Freelock_Queue[idx] = nullptr;
		return p;
	}

private:
	uint32_t m_mask;
	uint32_t m_capacity;
	bool m_is_single;
	void **Freelock_Queue;
	struct HeadTail
	{
		volatile uint32_t first cache_aligned;
		volatile uint32_t second cache_aligned;
	};
	HeadTail consumer;
	HeadTail producer;

	void InitRingQueue(uint32_t size)
	{
		memset(&consumer,0,sizeof(HeadTail));
		memset(&producer,0,sizeof(HeadTail));
		m_capacity = public_align32pow2(size);
		Freelock_Queue = new void* [m_capacity]();
		m_mask = m_capacity - 1;
	}
	void FreeRingQueue()
	{
		if(Freelock_Queue)
			delete []Freelock_Queue;
	}
};

#endif