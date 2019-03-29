#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <iostream>
#include <thread>
#include <sys/time.h>
#include "../CommonLib/UO_MemoryPool.h"
#include "../CommonLib/UO_Tree.h"
#include "../CommonLib/UO_Queue.h"
#include "../CommonLib/avlLib.h"
#include "../CommonLib/UO_Alarm.h"
#include "../CommonLib/UO_TableMap.h"
#include "../Net/UO_Net.h"

void memorypooltest()
{
	struct test
	{
		int a;
		int b;
		int c;
		test()
		{
			a = 111;
			b = 222;
			c = 121;
		}
		~test()
		{
			a = 11;
			b = 33;
			c = 55;
		}
	};
	MemoryPool<test> pool;
	test *ptest = pool.newElement();
	pool.deleteElement(ptest);
	ptest = pool.newElement();
	ptest->b = 100;
	test *ptest3 = pool.newElement();
	test *ptest4 = pool.newElement();
	std::cout << ptest3->b << std::endl;
	std::cout << ptest4->b << std::endl;
	std::cout << ptest->b << std::endl;
	
	test* ptest2 = pool.allocate();
	pool.construct(ptest2);
	std::cout << "ptest2:" << ptest2->c << std::endl;
	pool.destroy(ptest2);
	std::cout << "ptest2:" << ptest2->c << std::endl;
	pool.deallocate(ptest2);
	
	pool.freeMemoryPool();
}

void Queue_test()
{

	UO_RingQueue aa;
	int *array1 = new int[10000000];
	//int array1[1000000] = {0};
	for(int a = 0;a < 10000000;a++)
		array1[a] = a + 10000000;

	struct timeval start,end;
	int stop = 0;
	unsigned long diff;
	std::thread t1,t2,t3,t4;
	/*
	pthread_mutex_t mutex;
	gettimeofday(&start, NULL);
	t1 = std::thread( [ &array1,&aa,&mutex ]{for(int a = 0;a < 10000000;a++){pthread_mutex_lock(&mutex);aa.pushMSG(&array1[a]);pthread_mutex_unlock(&mutex);} });
	t2 = std::thread( [ &aa,&mutex ]{ int *MSG = NULL; while(true){pthread_mutex_lock(&mutex);MSG = (int*)aa.popMSG();pthread_mutex_unlock(&mutex);if(MSG == NULL){continue;}else{if(0 == *MSG)break;else{std::cout << *MSG << std::endl;}}} });
	t1.join();
	aa.pushMSG(&stop);
	t2.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
 	std::cout << "use mutex,one push,one pop:" << diff << std::endl;
 	
	gettimeofday(&start, NULL);
	t1 = std::thread( [ &array1,&aa,&mutex ]{for(int a = 0;a < 10000000;a++){pthread_mutex_lock(&mutex);aa.pushMSG(&array1[a]);pthread_mutex_unlock(&mutex);} });
	t2 = std::thread( [ &aa,&mutex ]{ int *MSG = NULL; while(true){pthread_mutex_lock(&mutex);MSG = (int*)aa.popMSG();pthread_mutex_unlock(&mutex);if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t3 = std::thread( [ &aa,&mutex ]{ int *MSG = NULL; while(true){pthread_mutex_lock(&mutex);MSG = (int*)aa.popMSG();pthread_mutex_unlock(&mutex);if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t1.join();
	aa.pushMSG(&stop);
	aa.pushMSG(&stop);
	t2.join();
	t3.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
 	std::cout << "use mutex,one push,two pop:" << diff << std::endl;
 	*/
	gettimeofday(&start, NULL);
	t1 = std::thread( [ &array1,&aa ]{for(int a = 0;a < 10000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t2 = std::thread( [ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t1.join();
	aa.pushMSG(&stop);
	t2.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
    std::cout << "use UORQ,one push,one pop:" << diff << std::endl;
 
	gettimeofday(&start, NULL);
	t1 = std::thread([ &array1,&aa ]{for(int a = 0;a < 5000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t2 = std::thread([ &array1,&aa ]{for(int a = 5000000;a < 10000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t3 = std::thread([ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t1.join();
	t2.join();
	aa.pushMSG(&stop);
	t3.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "use UORQ,two push,one pop:" << diff << std::endl;

	gettimeofday(&start, NULL);
	t1 = std::thread([ &array1,&aa ]{for(int a = 0;a < 5000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t2 = std::thread([ &array1,&aa ]{for(int a = 5000000;a < 10000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t3 = std::thread([ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t4 = std::thread([ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });	
	t1.join();
	t2.join();
	aa.pushMSG(&stop);
	aa.pushMSG(&stop);
	t3.join();
	t4.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "use UORQ,two push,two pop:" << diff << std::endl;

	gettimeofday(&start, NULL);
	t1 = std::thread([ &array1,&aa ]{for(int a = 0;a < 5000000;a++) while(!aa.pushMSG(&array1[a])){}});
	t3 = std::thread([ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });
	t4 = std::thread([ &aa ]{ int *MSG = NULL; while(true){MSG = (int*)aa.popMSG();if(MSG == NULL){continue;}else{if(!*MSG)break;}} });	
	t1.join();
	aa.pushMSG(&stop);
	aa.pushMSG(&stop);
	t3.join();
	t4.join();
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "use UORQ,one push,two pop:" << diff << std::endl;

	delete []array1;
}

int compare(void *node1,void *node2)
{
	int value1 = *(int*)node1;
	int value2 = *(int*)node2;
	return value1 - value2;
}

void avltreetest()
{
	// UO_AVLTree avltree(
	// 	[](void *node1,void *node2)->int 
	// 	{
	// 		int value1 = *(int*)node1;
	// 		int value2 = *(int*)node2;
	// 		return value1 - value2;
	// 	});
	
	UO_AVLTree avltree(compare);
	int size = 100;
	int values[100];
	//insert
	for(int i = 0; i < size; i++)
	{
		values[i] = i+100;
		if(!avltree.insert_item_to_avltree(&values[i]))
			std::cout << i << std::endl;
	}
	//search
	for(int i = 0; i < size; i++)
	{
		int* searchnode = (int*)avltree.search_item_from_avltree(&values[i]);
		if(!searchnode)
		{
			std::cout << i << std::endl;
			break;
		}
		if(*searchnode != values[i])
			std::cout << i << std::endl;
	}
	//delete
	
	for(int i = 0; i < size; i++)
	{
		if(!avltree.delete_item_from_avltree(&values[i]))
			std::cout << "delete fail:" << i << std::endl;

		std::cout << "delete " << i << std::endl;
		avltree.traverse_avltree([](void *node)
		{
			int *value1 = (int*)node;
			std::cout << *value1 << std::endl;
		},BREADTH);
		
		int* searchnode = (int*)avltree.search_item_from_avltree(&values[i]);
		if(searchnode)
			std::cout << "delete fail search_item_from_avltree" << *searchnode <<std::endl;
		avltree.insert_item_to_avltree(&values[i]);
		searchnode = (int*)avltree.search_item_from_avltree(&values[i]);
		if(!searchnode)
			std::cout << "delete fail insert_item_to_avltree" << i << std::endl;
		std::cout << "insert " << i << std::endl;
		avltree.traverse_avltree([](void *node)
		{
			int *value1 = (int*)node;
			std::cout << *value1 << std::endl;
		},BREADTH);
	}
	//preitem,nextitem
	int test = 10;
	std::cout << *(int*)avltree.get_nextitem(&test) << std::endl;
	std::cout << *(int*)avltree.get_max_item() << std::endl;
	std::cout << *(int*)avltree.get_min_item() << std::endl;
}

void segmenttreetest()
{
	UO_SegmentTree test([](void *node1,void *node2)->int 
		{
			int value1 = *(int*)node1;
			int value2 = *(int*)node2;
			return value1 - value2;
		});
	int size = 10;
	SEGNODE *node = new SEGNODE[size];
	memset(node,0,sizeof(SEGNODE) * size);
	int *minmax = new int[size*2];
	for(int i = 0;i < size * 2;i+=2)
	{
		minmax[i] = (i * 10) + 1;
		minmax[i+1] = (i + 2) * 10;
	}
	for(int i = 0;i < size;i++)
	{
		node[i].min = &minmax[i*2];
		node[i].max = &minmax[i*2+1];
		node[i].value = &minmax[i*2];
	}
	//insert
	for(int i = 0;i < size; i++)
		test.insert_item_to_segtree(&node[i]);

	SEGNODE node21_100 = {0};
	int min21_100 = 21;
	int max21_100 = 100;
	node21_100.min = &min21_100;
	node21_100.max = &max21_100;
	test.insert_item_to_segtree(&node21_100);

	SEGNODE node41_80 = {0};
	int min41_80 = 41;
	int max41_80 = 80;
	node41_80.min = &min41_80;
	node41_80.max = &max41_80;
	test.insert_item_to_segtree(&node41_80);
	//show
	test.show_item_from_segtree([](SEGNODE *node){printf("min:%d----max:%d\n",*(int*)node->min,*(int*)node->max);});
	//search
	SEGNODE nodesearch = {0};
	int nodesearchmin = 21;
	int nodesearchmax = 100;
	nodesearch.min = &nodesearchmin;
	nodesearch.max = &nodesearchmax;
	SEGNODE *searchnode = test.search_item_from_segtree(&nodesearch);
	if(searchnode)
		printf("min:%d----max:%d\n",*(int*)searchnode->min,*(int*)searchnode->max);
	else
		printf("nullptr");
	//delete
	printf("deleteSegnode: \n");
	test.delete_item_from_segtree(&node41_80);
	test.show_item_from_segtree([](SEGNODE *node){printf("min:%d----max:%d\n",*(int*)node->min,*(int*)node->max);});
	delete []node;
	delete []minmax;
}

int com1(void * node1, void * node2)
{
	int value1 = *(int*)node1;
	int value2 = *(int*)node2;
	return value1 - value2;
}
struct AVLNODE
{
	AVL_NODE node;
	void *a;
};
int com(void * node, GENERIC_ARGUMENT key)
{

	int value1 = *(int*)((AVLNODE*)node)->a;
	int value2 = *(int*)((AVLNODE*)key.p)->a;
	return value1 - value2;
}

void UOavl_vs_asiaavl_vs_linuxrbtree()
{
	/////////////////////////////////UOavl
	UO_AVLTree avltree(
	[](void *node1,void *node2)->int 
	{
		int value1 = *(int*)node1;
		int value2 = *(int*)node2;
		return value1 - value2;
	});
	//UO_AVLTree avltree(com1);
	struct timeval start,end;
	unsigned long diff;

	int size = 1000000;
	int *values = new int[size];
	////////////////UOavl insert

	for(int i = 0; i < size; i++)
	{
		values[i] = i;
		if(!avltree.insert_item_to_avltree(&values[i]))
			std::cout << i << std::endl;
	}

	///////////////////asiaavl insert
	AVL_TREE asiaavl = nullptr;
	GENERIC_ARGUMENT    key;
	AVLNODE *asiaavlnodes = new AVLNODE[size];
	memset(asiaavlnodes,0,sizeof(AVLNODE) * size);

	for(int i = 0; i < size; i++)
	{
		asiaavlnodes[i].a = &values[i];
		key.p = &asiaavlnodes[i];
		if(-1 == avlInsert(&asiaavl, &asiaavlnodes[i], key, com))
			std::cout << "a";
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////asiasearch
	gettimeofday(&start, NULL);
	for(int i = 0; i < size; i++)
	{
		key.p = &asiaavlnodes[i];
		avlSearch1(asiaavl,key,com);
	}
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "asiaavl search time:" << diff << std::endl;
	//////////////UOavlsearch

	gettimeofday(&start, NULL);
	for(int i = 0; i < size; i++)
		avltree.search_item_from_avltree(&values[i]);
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "UOavl search time:" << diff << std::endl;

	delete []asiaavlnodes;
	delete []values;
}

int fun1(int a,int b)
{
	return a - b;
}

void functiontest()
{
	struct timeval start,end;
	int size = 10000000;
	unsigned long diff;
	std::function<int(int,int)> tt = fun1;
	gettimeofday(&start, NULL);
	for(int i = 0;i < size;i++)
		tt(i,i+1);
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "std::function time:" << diff << std::endl;

	int (*pfun)(int,int);
	pfun = fun1;
	gettimeofday(&start, NULL);
	for(int i = 0;i < size;i++)
		pfun(i,i+1);
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "function pointer time:" << diff << std::endl;
}

void testalarm()
{
	UO_Alarm UOalarm([](void *param){std::cout << *(int*)param << std::endl;});
	int a = 2;
	int b = 4;
	UOalarm.setalarmtime(a, &a);
	UOalarm.setalarmtime(b, &b);
	char c;
	std::cin >> c;
}

void trietree()
{
	UO_CompressTrietree test;
	test.addstr_in_trietree("abcdddd");
	test.addstr_in_trietree("bbb");
	test.addstr_in_trietree("abcd");
	test.addstr_in_trietree("abcdedd");
	test.addstr_in_trietree("ab");

	std::cout << test.findstr_in_trietree("ab") << std::endl;
	std::cout << test.findstr_in_trietree("abcdedd") << std::endl;
	std::cout << test.findstr_in_trietree("abcd") << std::endl;
	std::cout << test.findstr_in_trietree("abcdddd") << std::endl;
	std::cout << test.findstr_in_trietree("bbb") << std::endl;

	test.deletestr_in_trietree("ab");
	std::cout << test.findstr_in_trietree("ab") << std::endl;
	test.addstr_in_trietree("ab");
	test.addstr_in_trietree("abc");
	std::cout << test.findstr_in_trietree("ab") << std::endl;
	std::cout << test.findstr_in_trietree("abd") << std::endl;
	std::cout << test.findstr_in_trietree("abc") << std::endl;
}

void ASIOtest()
{
	UO_RingQueue udprecvring;
	MemoryPool<SocketPacket> mempool;
	UO_Asio_UDP AsioUDPServer(&udprecvring,&mempool,3);
	std::string ip = "192.168.10.51";
	int port = 12345;
	AsioUDPServer.startudp_server(ip, port);
}

void epolltest()
{
	UO_RingQueue udprecvring;
	MemoryPool<SocketPacket> mempool;
	UO_Epoll epollServer(&udprecvring,&mempool,2);
	epollServer.startudp_server("192.168.10.51", 12345);
}

void asiothreadtooltest()
{
	UO_ThreadPool threads(4);
	threads.post([](){std::cout<<"test 1"<<std::endl;});
	threads.post([](){sleep(2);std::cout<<"test 2"<<std::endl;});
 
	sleep(4);
	threads.stop();
}

void testbitmap()
{
	UO_BitMap bitmap;
	std::cout << bitmap.add_to_bitmap(100) << std::endl;
	std::cout << bitmap.add_to_bitmap(100) << std::endl;
	std::cout << bitmap.if_in_bitmap(100) << std::endl;
	std::cout << bitmap.delete_from_bitmap(100) << std::endl;
	std::cout << bitmap.if_in_bitmap(100) << std::endl;
}

void testhashtable()
{
	UO_Hash_Table hashtable(128 * 1024);
	for(int i = 0;i< 1000;i++)
		hashtable.Add_To_HashTable(to_string(i).data(),&i);
	//std::cout << hashtable.Conflict << std::endl;
}

void testnextandarray()
{
	struct NODEu
	{
		NODEu *next;
	};
	NODEu *p = new NODEu[10000000];
	for(int i = 0;i< 9999999;i++)
		p[i].next = &p[i+1];
	NODEu *t;
	struct timeval start,end;
	unsigned long diff;
	
	gettimeofday(&start, NULL);
	for(int i = 0;i < 9999999;i++)
		t = &p[i];
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "std::function time:" << diff << std::endl;

	gettimeofday(&start, NULL);
	for(;p;p=p->next)
		t = p;
	gettimeofday(&end, NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;
	std::cout << "std::function time:" << diff << std::endl;
}

static char u8ToChars[256][8];
static char u16ToChars[65536][8];

void init_u8toChars()
{
	int i;
	for (i = 0; i <= 0xff; i++)
		snprintf(u8ToChars[i], 5, "%d.", i);
	for (i = 0; i <= 0xffff; i++)
		snprintf(u16ToChars[i], 6, "%d", i);
}

size_t tune_ntop4(uint8_t *src, char *dst)
{
	uint8_t *tmp = (uint8_t *)dst;
	memcpy(tmp, u8ToChars[src[0]], 4);
	if (src[0] >= 100)
		tmp += 2;
	else if (src[0] >= 10)
		tmp += 1;
	tmp += 2;
	memcpy(tmp, u8ToChars[src[1]], 4);
	if (src[1] >= 100)
		tmp += 2;
	else if (src[1] >= 10)
		tmp += 1;
	tmp += 2;
	memcpy(tmp, u8ToChars[src[2]], 4);
	if (src[2] >= 100)
		tmp += 2;
	else if (src[2] >= 10)
		tmp += 1;
	tmp += 2;
	memcpy(tmp, u8ToChars[src[3]], 4);
	if (src[3] >= 100)
		tmp += 2;
	else if (src[3] >= 10)
		tmp += 1;
	tmp += 1;
	*tmp = '\0';
	return tmp - (uint8_t *)dst;
}

void testiptos()
{
	init_u8toChars();
	char ip[16] = { 0 };
	uint8_t nip[16] = { 0 };
	inet_pton(AF_INET,"192.168.1.1",nip);
	int i = 0;
	clock_t start, finish;

	start = clock();
	for (; i < 10000000; i++)
		inet_ntop(AF_INET, (struct in_addr *)nip, ip, 16);
	finish = clock();
	std::cout << finish - start << std::endl;

	i = 0;
	start = clock();
	for (; i<100000000; i++)
		tune_ntop4(nip, ip);
	finish = clock();
	std::cout << finish - start << std::endl;
}

void readfromfile()
{
	char buf[1024] = {0};
	UO_Hash_Table hashtable(100000);
	int value[110000] = {0};
	int i = 0;
	while(true)
	{
		std::cin >> buf;
		if(!(strcmp(buf,"end")))
			break;
		hashtable.Add_To_HashTable(buf,&value[i]);
		memset(buf,0,1024);
		value[i] = i;
		i++;
	}
	//std::cout << hashtable.Conflict << std::endl;
}

int main()
{
	//memorypooltest();
	//Queue_test();
	//avltreetest();
	//segmenttreetest();
	//UOavl_vs_asiaavl_vs_linuxrbtree();
	//functiontest();
	//testalarm();
	//trietree();
	//ASIOtest();
	//epolltest();
	//asiothreadtooltest();
	//testnextandarray();
	//testbitmap();
	//testhashtable();
	//testiptos();
	//readfromfile();
	return 0;
}
