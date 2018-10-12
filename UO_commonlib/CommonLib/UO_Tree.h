
#ifndef TREE_H_UO
#define TREE_H_UO
#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <vector>
#include "UO_MemoryPool.h"
#include "UO_Public.h"
#include "UO_Queue.h"

enum FIRST_SEARCH_TYPE{DEPTH,BREADTH};

//safe
class UO_AVLTree:UO_SpinLock
{
private:
	struct AVLNODE
	{
		AVLNODE *lchild;
		AVLNODE *rchild;
		AVLNODE *parent;
		uint16_t high;
		void *value;
		AVLNODE()
		{
			lchild = nullptr;
			rchild = nullptr;
			parent = nullptr;
			value = nullptr;
			high = 1;
		}
	};
	MemoryPool<AVLNODE,1024> AVLTreePool;
	AVLNODE *m_proot;
	std::function<int(void*,void*)> m_pcomparefun;
	void delete_avltree();
	void Depth_First_Search_avltree(std::function<void(void *)> const &traversefun,AVLNODE *node);
	void Breadth_First_Search_avltree(std::function<void(void *)> const &traversefun,AVLNODE *node);
	void Swap_root(AVLNODE *parent,AVLNODE *oldnode,AVLNODE *newnode);
	void Set_high(AVLNODE *node);
	void Rotate_Right(AVLNODE *node);
	void Rotate_Left(AVLNODE *node);
	void Rotate_Left_Higher(AVLNODE *node);
	void Rotate_Right_Higher(AVLNODE *node);
	void Rotate(AVLNODE *node);
	void *get_nextitem_from_child(AVLNODE *node);
	void *search_AVLitem_from_avltree(void *value);
	int lchildhigh(AVLNODE *node);
	int rchildhigh(AVLNODE *node);
	int abs(int num);
public:
	explicit UO_AVLTree(std::function<int(void*,void*)> const &pcomparefun);
	~UO_AVLTree();
	bool insert_item_to_avltree(void *value);
	bool delete_item_from_avltree(void *value);
	void *search_item_from_avltree(void *value);
	void *get_nextitem(void *value);
	void *get_preitem(void *value);
	void *get_max_item();
	void *get_min_item();
	void traverse_avltree(std::function<void(void *)> const &traversefun,FIRST_SEARCH_TYPE type);
};

//not suport multithread
struct SEGNODE
{
	UO_AVLTree *avltree;
	void *min;
	void *max;
	void *value;
};
class UO_SegmentTree
{
private:
	enum COMPARETYPE{STARTSTART,STARTEND,ENDSTART,ENDEND};
	UO_AVLTree *m_pavltree;
	MemoryPool<UO_AVLTree,1024> m_SegTreePool;
	UO_AVLTree *m_allocated;
	SEGNODE *m_conflicate;
	std::function<int(void*,void*)> m_comparefun;
	std::function<int(void*,void*)> m_avlcomparefun;
	int _compare(SEGNODE *node1,SEGNODE *node2,COMPARETYPE comparetype);
	bool _check_item(UO_AVLTree *avltree,SEGNODE *node,SEGNODE *nodeadd);
	bool _insert_item_to_segtree(UO_AVLTree *avltree,SEGNODE *nodeadd);
	int Segcompare(void *node1,void *node2);
	int _Allocatedcompare(void *node1,void *node2);
	void _delete_allocated_node(void *node);
	void _show_itemtree_from_segtree(std::function<void(SEGNODE*)> const &showfun,UO_AVLTree *tree,int level);

public:
	explicit UO_SegmentTree(std::function<int(void*,void*)> const &p);
	~UO_SegmentTree();
	bool insert_item_to_segtree(SEGNODE *nodeadd);
	bool delete_item_from_segtree(SEGNODE *nodedel);
	SEGNODE *search_item_from_segtree(SEGNODE *nodesearch);
	void show_item_from_segtree(std::function<void(SEGNODE*)> const &showfun);
};

//asc2:
//!:21
//~:7E
class UO_CompressTrietree
{
private:
	struct TRIENODE
	{
		char str[128];
		TRIENODE *node[93];
		bool isentry;
		TRIENODE():isentry(false)
		{
			str[0] = '\0';
			memset(node,0,sizeof(void*) * 93);
		}
	};
	TRIENODE m_trieroot;
	MemoryPool<TRIENODE,20480> m_mempool;
	int _addstr_in_trietree(const char *str,TRIENODE *parentnode);
	int comparestr(const char *str1,const char *str2);
	void *_newnode(const char *str,const int len,const bool isentry);
	int _getindex(const char *ch);
public:
	UO_CompressTrietree();
	~UO_CompressTrietree();
	void addstr_in_trietree(const char *str);
	bool findstr_in_trietree(const char *str);
	bool deletestr_in_trietree(const char *str);
};

#endif