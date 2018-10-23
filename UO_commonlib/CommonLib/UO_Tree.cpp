#include "UO_Tree.h"
#include <stdlib.h>
#include <string.h>
#include <queue>

UO_AVLTree::UO_AVLTree(std::function<int(void*,void*)> const &pcomparefun):m_proot(NULL),m_pcomparefun(pcomparefun)
{
}

UO_AVLTree::~UO_AVLTree()
{
	delete_avltree();
}

inline int UO_AVLTree::lchildhigh(AVLNODE *node)
{
	return node->lchild ? node->lchild->high : 0;
}

inline int UO_AVLTree::rchildhigh(AVLNODE *node)
{
	return node->rchild ? node->rchild->high : 0;
}

inline int UO_AVLTree::abs(int num)
{
	int temp = (uint32_t)num >> 31;
	return (num ^ temp) - temp;
}

inline void UO_AVLTree::Set_high(AVLNODE *node)
{
	if(!node)
		return;
	int lh = lchildhigh(node);
	int rh = rchildhigh(node);
	node->high = (lh > rh ? lh : rh) + 1;
}

inline void UO_AVLTree::Swap_root(AVLNODE *parent,AVLNODE *oldnode,AVLNODE *newnode)
{
	if(parent)
	{
		if(parent->lchild == oldnode)
			parent->lchild = newnode;
		else
			parent->rchild = newnode;
	}
	else
		m_proot = newnode;
}

inline void UO_AVLTree::Rotate_Right(AVLNODE *node)
{
	AVLNODE *parent = node->parent;
	AVLNODE *lchild = node->lchild;
	AVLNODE *lchildrchild = lchild->rchild;
	if(lchildrchild)
		lchildrchild->parent = node;
	node->lchild = lchildrchild;
	node->parent = lchild;
	lchild->rchild = node;
	lchild->parent = parent;
	Swap_root(parent, node, lchild);
	Set_high(node);
	Set_high(node->parent);
}

inline void UO_AVLTree::Rotate_Left(AVLNODE *node)
{
	AVLNODE *parent = node->parent;
	AVLNODE *rchild = node->rchild;
	AVLNODE *rchildlchild = rchild->lchild;
	if(rchildlchild)
		rchildlchild->parent = node;
	node->rchild = rchildlchild;
	node->parent = rchild;
	rchild->lchild = node;
	rchild->parent = parent;	
	Swap_root(parent, node, rchild);
	Set_high(node);
	Set_high(node->parent);
}

inline void UO_AVLTree::Rotate_Left_Higher(AVLNODE *node)
{
	AVLNODE *lchild = node->lchild;
	int lh = lchildhigh(lchild);
	int rh = rchildhigh(lchild);
	if(lh < rh)
		Rotate_Left(lchild);
	Rotate_Right(node);
}

inline void UO_AVLTree::Rotate_Right_Higher(AVLNODE *node)
{
	AVLNODE *rchild = node->rchild;
	int lh = lchildhigh(rchild);
	int rh = rchildhigh(rchild);
	if(rh < lh)
		Rotate_Right(rchild);
	Rotate_Left(node);
}

inline void UO_AVLTree::Rotate(AVLNODE *node)
{
	while(node)
	{
		int lh = lchildhigh(node);
		int rh = rchildhigh(node);
		int high = (lh > rh ? lh : rh) + 1;
		if(node->high == high && abs(lh-rh) < 2)
			break;
		else
		{
			node->high = high;
			if(lh - rh > 1)
				Rotate_Left_Higher(node);
			else  if(rh - lh > 1)
				Rotate_Right_Higher(node);
		}
		node = node->parent;
	}
}
/*
*		///////////////////////////////////////////////////////////
*		//RightRotate
*		//									4
*		//					2								6
*		//			1				3
*		//		0.5
*		///////////////////////////////////////////////////////////
*		//									4
*		//					2								6
*		//			1				3
*		//				1.5
*		///////////////////////////////////////////////////////////
*		//LeftRotate and RightRotate
*		//									4
*		//					2								6
*		//			1				3
*		//						2.5
*		///////////////////////////////////////////////////////////
*		//									4
*		//					2								6
*		//			1				3
*		//								3.5
*		///////////////////////////////////////////////////////////
*		//LeftRotate
*		//									4
*		//					2								6
*		//											5				7
*		//														6.5
*		///////////////////////////////////////////////////////////
*		//									4
*		//					2								6
*		//											5				7
*		//																7.5				
*		///////////////////////////////////////////////////////////
*		//RightRotate and LeftRotate
*		//									4
*		//					2								6
*		//											5				7
*		//										4.5
*		///////////////////////////////////////////////////////////
*		//									4
*		//					2								6
*		//											5				7
*		//												5.5
*/
bool UO_AVLTree::insert_item_to_avltree(void *value)
{
	Lock();
	AVLNODE **pnodechild = &m_proot;
	AVLNODE *parent = nullptr;
	while(*pnodechild)
	{
		parent = *pnodechild;
		int delta = m_pcomparefun((*pnodechild)->value,value);
		if(delta == 0)
			return false;
		else if(delta > 0)
			pnodechild = &(*pnodechild)->lchild;
		else
			pnodechild = &(*pnodechild)->rchild;
	}
	*pnodechild = AVLTreePool.newElement();
	(*pnodechild)->value = value;
	(*pnodechild)->parent = parent;
	Rotate(parent);
	Unlock();
	return true;
}

void *UO_AVLTree::get_nextitem_from_child(AVLNODE *node)
{
	Lock();
	AVLNODE *lchild = nullptr;
	node = node->rchild;
	while((lchild = node->lchild) != nullptr)
		node = lchild;
	Unlock();
	return node;
}

void *UO_AVLTree::get_nextitem(void *value)
{
	Lock();
	AVLNODE *pnode = nullptr;
	AVLNODE *ptemp = m_proot;
	while(ptemp)
	{
		int delta = m_pcomparefun(ptemp->value,value);
		if(delta <= 0)
			ptemp = ptemp->rchild;
		else
		{
			pnode = ptemp;
			ptemp = ptemp->lchild;
		}
	}
	Unlock();
	return pnode?pnode->value:nullptr;
}

void *UO_AVLTree::get_preitem(void *value)
{
	Lock();
	AVLNODE *pnode = nullptr;
	AVLNODE *ptemp = m_proot;
	while(ptemp)
	{
		int delta = m_pcomparefun(ptemp->value,value);
		if(delta >= 0)
			ptemp = ptemp->lchild;
		else
		{
			pnode = ptemp;
			ptemp = ptemp->rchild;
		}
	}
	Unlock();
	return pnode?pnode->value:nullptr;
}

bool UO_AVLTree::delete_item_from_avltree(void *value)
{
	bool flag = false;
	AVLNODE *pdeletenode = static_cast<AVLNODE *>(search_AVLitem_from_avltree(value));
	if(pdeletenode)
	{
		AVLNODE *parent = nullptr;
		if(pdeletenode->lchild && pdeletenode->rchild)
		{
			AVLNODE *nextnode = static_cast<AVLNODE *>(get_nextitem_from_child(pdeletenode));
			if(nextnode->parent == pdeletenode)
			{
				pdeletenode->lchild->parent = nextnode;
				nextnode->lchild = pdeletenode->lchild;
				nextnode->parent = pdeletenode->parent;
				Swap_root(pdeletenode->parent, pdeletenode, nextnode);
				parent = nextnode;
			}
			else
			{
				parent = nextnode->parent;
				if(nextnode->rchild)
					nextnode->rchild->parent = nextnode->parent;
				nextnode->parent->lchild = nextnode->rchild;
				//Swap_root(nextnode->parent, nextnode, nextnode->rchild);
				pdeletenode->lchild->parent = nextnode;
				pdeletenode->rchild->parent = nextnode;
				nextnode->lchild = pdeletenode->lchild;
				nextnode->rchild = pdeletenode->rchild;
				nextnode->parent = pdeletenode->parent;
				Swap_root(pdeletenode->parent, pdeletenode, nextnode);
			}
		}
		else
		{
			AVLNODE *pchild = nullptr;
			if(pdeletenode->lchild)
				pchild = pdeletenode->lchild;
			else
				pchild = pdeletenode->rchild;
			if(pchild)
				pchild->parent = pdeletenode->parent;
			Swap_root(pdeletenode->parent, pdeletenode, pchild);
			parent = pdeletenode->parent;
		}
		AVLTreePool.deleteElement(pdeletenode);
		if(parent)
			Rotate(parent);
		flag = true;
	}
	return flag;
}

inline void *UO_AVLTree::search_AVLitem_from_avltree(void *value)
{
	Lock();
	AVLNODE *pnode = m_proot;
	while(pnode)
	{
		int delta = m_pcomparefun(pnode->value,value);
		if(delta == 0)
		{
			Unlock();
			return pnode;
		}
		else if(delta > 0)
			pnode = pnode->lchild;
		else
			pnode = pnode->rchild;
	}
	Unlock();
	return nullptr;
}

void *UO_AVLTree::search_item_from_avltree(void *value)
{
	void *rc;
	AVLNODE *pnode = static_cast<AVLNODE*>(search_AVLitem_from_avltree(value));
	if(pnode)
		rc = pnode->value;
	else
		rc = nullptr;
	return rc;
}

void *UO_AVLTree::get_max_item()
{
	Lock();
	AVLNODE *pnode = m_proot;
	AVLNODE *prenode = NULL;
	while(pnode)
	{
		prenode = pnode;
		pnode = pnode->rchild;
	}
	Unlock();
	return prenode?prenode->value:nullptr;
}

void *UO_AVLTree::get_min_item()
{
	Lock();
	AVLNODE *pnode = m_proot;
	AVLNODE *prenode = NULL;
	while(pnode)
	{
		prenode = pnode;
		pnode = pnode->lchild;
	}
	Unlock();
	return prenode?prenode->value:nullptr;
}

void UO_AVLTree::Depth_First_Search_avltree(std::function<void(void *)> const &traversefun,AVLNODE *pnode)
{
	if(pnode)
	{
		Depth_First_Search_avltree(traversefun,pnode->lchild);
		Depth_First_Search_avltree(traversefun,pnode->rchild);
		traversefun(pnode->value);
	}
}

void UO_AVLTree::Breadth_First_Search_avltree(std::function<void(void *)> const &traversefun,AVLNODE *root)
{
	std::queue<AVLNODE *> queue;
	queue.push(root);
	while(!queue.empty())
	{
		AVLNODE *pnode = queue.front();
		queue.pop();
		traversefun(pnode->value);
		if(pnode->lchild)
			queue.push(pnode->lchild);
		if(pnode->rchild)
			queue.push(pnode->rchild);
	}
}

void UO_AVLTree::traverse_avltree(std::function<void(void *)> const &traversefun,FIRST_SEARCH_TYPE type)
{
	Lock();
	switch(type)
	{
		case DEPTH:
			Depth_First_Search_avltree(traversefun,m_proot);
			break;
		case BREADTH:
			Breadth_First_Search_avltree(traversefun,m_proot);
			break;
		default:
			break;
	}
	Unlock();
}

void UO_AVLTree::delete_avltree()
{
	if(m_proot)
		AVLTreePool.freeMemoryPool();
}

UO_SegmentTree::UO_SegmentTree(std::function<int(void*,void*)> const &p):m_conflicate(nullptr),m_comparefun(p)
{
	m_avlcomparefun = std::bind(&UO_SegmentTree::Segcompare,this,std::placeholders::_1,std::placeholders::_2);
	m_pavltree = m_SegTreePool.newElement(m_avlcomparefun);
	m_allocated = new UO_AVLTree(std::bind(&UO_SegmentTree::_Allocatedcompare,this,std::placeholders::_1,std::placeholders::_2));
	m_allocated->insert_item_to_avltree(m_pavltree);
}

UO_SegmentTree::~UO_SegmentTree()
{
	m_allocated->traverse_avltree(std::bind(&UO_SegmentTree::_delete_allocated_node,this,std::placeholders::_1), DEPTH);
	delete m_allocated;
}

void UO_SegmentTree::_delete_allocated_node(void *node)
{
	m_SegTreePool.deleteElement(static_cast<UO_AVLTree*>(node));
}

int UO_SegmentTree::_Allocatedcompare(void *a,void *b)
{
	return (uint64_t)a - (uint64_t)b;
}

int UO_SegmentTree::_compare(SEGNODE *node1,SEGNODE *node2,COMPARETYPE comparetype)
{
	int temp = 0;
	void *min1 = node1->min;
	void *max1 = node1->max;
	void *min2 = node2->min;
	void *max2 = node2->max;
	switch(comparetype)
	{
		case STARTSTART:
			temp = m_comparefun(min1,min2);
			break;
		case STARTEND:
			temp = m_comparefun(min1,max2);
			break;
		case ENDSTART:
			temp = m_comparefun(max1,min2);
			break;
		case ENDEND:
			temp = m_comparefun(max1,max2);
			break;
	}
	return temp;
}

bool UO_SegmentTree::insert_item_to_segtree(SEGNODE *nodeadd)
{
	m_conflicate = nullptr;
	SEGNODE *nodesearch = static_cast<SEGNODE*>(m_pavltree->search_item_from_avltree(nodeadd));
	SEGNODE *upnode = nullptr;
	while(nodesearch)
	{
		if(m_conflicate)
		{
			int ss = _compare(m_conflicate,nodeadd,STARTSTART);
			int ee = _compare(m_conflicate,nodeadd,ENDEND);
			m_conflicate = nullptr;
			if(ss == 0 && ee == 0)
				return false;
			else if(ss <= 0 && ee >= 0)//contain
			{
				upnode = nodesearch;
				nodesearch = static_cast<SEGNODE*>(nodesearch->avltree->search_item_from_avltree(nodeadd));
				if(!nodesearch)
					return upnode->avltree->insert_item_to_avltree(nodeadd);
				else
					continue;
			}
			else if(ss >= 0 && ee <= 0)//becontained
			{
				std::queue<SEGNODE*> queue;
				queue.push(nodesearch);
				UO_AVLTree *ptree = upnode?upnode->avltree:m_pavltree;
				SEGNODE *prenode = static_cast<SEGNODE*>(ptree->get_preitem(nodesearch));
				while(prenode)
				{
					if(_compare(prenode,nodeadd,ENDSTART) < 0)
						break;
					if(_compare(prenode,nodeadd,STARTSTART) >= 0 && _compare(prenode,nodeadd,ENDEND) <= 0)
					{
						queue.push(prenode);
						prenode = static_cast<SEGNODE*>(ptree->get_preitem(prenode));
					}
					else
						return false;
				}
				SEGNODE *nextnode = static_cast<SEGNODE*>(ptree->get_nextitem(nodesearch));
				while(nextnode)
				{
					if(_compare(nextnode,nodeadd,STARTEND) > 0)
						break;
					if(_compare(nextnode,nodeadd,STARTSTART) >= 0 && _compare(nextnode,nodeadd,ENDEND) <= 0)
					{
						queue.push(nextnode);
						nextnode = static_cast<SEGNODE*>(ptree->get_nextitem(nextnode));
					}
					else
						return false;
				}
				while(!queue.empty())
				{
					nodesearch = queue.front();
					queue.pop();
					ptree->delete_item_from_avltree(nodesearch);
					if(!nodeadd->avltree)
					{
						nodeadd->avltree = m_SegTreePool.newElement(m_avlcomparefun);
						m_allocated->insert_item_to_avltree(nodeadd->avltree);
					}
					nodeadd->avltree->insert_item_to_avltree(nodesearch);
				}
				return ptree->insert_item_to_avltree(nodeadd);
			}
		}
		else
			return false;
	}
	return m_pavltree->insert_item_to_avltree(nodeadd);
}

SEGNODE *UO_SegmentTree::search_item_from_segtree(SEGNODE *nodesearch)
{
	m_conflicate = nullptr;
	SEGNODE *node = static_cast<SEGNODE*>(m_pavltree->search_item_from_avltree(nodesearch));
	SEGNODE *prenode = nullptr;
	while(node && m_conflicate)
	{
		prenode = node;
		m_conflicate = nullptr;
		int ss = _compare(node,nodesearch,STARTSTART);
		int ee = _compare(node,nodesearch,ENDEND);
		if(ss == 0 && ee == 0)
			return node;
		else if(ss >= 0 && ee <= 0)
			return nullptr;
		if(node->avltree)
			node = static_cast<SEGNODE*>(node->avltree->search_item_from_avltree(nodesearch));
		else
			return prenode;
	}
	return prenode;
}

bool UO_SegmentTree::delete_item_from_segtree(SEGNODE *nodedel)
{
	m_conflicate = nullptr;
	SEGNODE *node = static_cast<SEGNODE*>(m_pavltree->search_item_from_avltree(nodedel));
	UO_AVLTree *tree = m_pavltree;
	while(node && m_conflicate)
	{
		m_conflicate = nullptr;
		int ss = _compare(node,nodedel,STARTSTART);
		int ee = _compare(node,nodedel,ENDEND);
		if(ss == 0 && ee == 0)
		{
			tree->delete_item_from_avltree(node);
			UO_AVLTree *delnodetree = nullptr;
			if((delnodetree = node->avltree) != nullptr)
			{
				std::function<void(void*)> pfun = [&tree](void *nodechild)
				{
					tree->insert_item_to_avltree(nodechild);
				};
				node->avltree->traverse_avltree(pfun,DEPTH);
		
			}
			m_allocated->delete_item_from_avltree(node);
			return true;
		}
		else if(ss <= 0 && ee >= 0)
		{
			tree = node->avltree;
			node = static_cast<SEGNODE*>(node->avltree->search_item_from_avltree(nodedel));
		}
		else
			return false;
	}
	return false;
}

void UO_SegmentTree::_show_itemtree_from_segtree(std::function<void(SEGNODE*)> const &showfun,UO_AVLTree *tree,int level)
{
	SEGNODE *minnode = static_cast<SEGNODE*>(tree->get_min_item());
	SEGNODE *minnodenext = nullptr;
	while(minnode)
	{
		minnodenext = static_cast<SEGNODE*>(tree->get_nextitem(minnode));
		for(int i = 0;i<level;i++)
			printf("\t");
		showfun(minnode);
		if(minnode->avltree)
			_show_itemtree_from_segtree(showfun,minnode->avltree,level + 1);
		minnode = minnodenext;
	}
}

void UO_SegmentTree::show_item_from_segtree(std::function<void(SEGNODE*)> const &showfun)
{
	_show_itemtree_from_segtree(showfun,m_pavltree,0);
}

int UO_SegmentTree::Segcompare(void *node1,void *node2)
{
	int ss = _compare(static_cast<SEGNODE*>(node1),static_cast<SEGNODE*>(node2),STARTSTART);
	int se = _compare(static_cast<SEGNODE*>(node1),static_cast<SEGNODE*>(node2),STARTEND);
	int es = _compare(static_cast<SEGNODE*>(node1),static_cast<SEGNODE*>(node2),ENDSTART);
	int ee = _compare(static_cast<SEGNODE*>(node1),static_cast<SEGNODE*>(node2),ENDEND);
	if(es < 0)
		return -1;
	else if(se > 0)
		return 1;
	else if((ss <= 0 && ee >= 0) || (ss >= 0 && ee <= 0))
	{
		m_conflicate = static_cast<SEGNODE*>(node1);
		return 0;
	}
	else
		return 0;
}

UO_CompressTrietree::UO_CompressTrietree()
{}

UO_CompressTrietree::~UO_CompressTrietree()
{
	m_mempool.freeMemoryPool();
}

void UO_CompressTrietree::addstr_in_trietree(const char *str)
{
	TRIENODE *parentnode = &m_trieroot;
	char *addstr = const_cast<char*>(str);
	int res = _addstr_in_trietree(addstr,parentnode);
	int index = 0;
	while(res != -1)
	{
		index = _getindex(addstr);
		parentnode = parentnode->node[index];
		addstr+=res;
		res = _addstr_in_trietree(addstr,parentnode);
	}
}

//return offset of str
int UO_CompressTrietree::_addstr_in_trietree(const char *str,TRIENODE *parentnode)
{
	int index = _getindex(str);
	TRIENODE *pnode = parentnode->node[index];
	char *addstr = const_cast<char*>(str);
	char *pnodestr = pnode->str;
	if(!pnode)
	{
		parentnode->node[index] = (TRIENODE*)_newnode(str, strlen(str), true);
		return -1;
	}
	else
	{
		int same = comparestr(addstr,pnodestr);
		addstr+=same;
		pnodestr+=same;
		if(*addstr == '\0' && *pnodestr == '\0')
		{
			pnode->isentry = true;
			return -1;
		}
		if(*addstr == '\0')
		{
			TRIENODE *newnode = (TRIENODE*)_newnode(str, strlen(str), true);
			parentnode->node[index] = newnode;

			strcpy(pnode->str,pnodestr);
			index = _getindex(pnode->str);
			newnode->node[index] = pnode;
			return -1;
		}
		else if(*pnodestr == '\0')
		{
			if(!parentnode->node[index])
			{
				parentnode->node[index] = (TRIENODE*)_newnode(addstr, strlen(addstr), true);
				return -1;
			}
			else
				return same;
		}
		else
		{
			TRIENODE *newnode = (TRIENODE*)_newnode(str, same, false);
			parentnode->node[index] = newnode;

			strcpy(pnode->str,pnodestr);
			index = _getindex(pnodestr);
			newnode->node[index] = pnode;
			
			index = _getindex(addstr);
			newnode->node[index] = (TRIENODE*)_newnode(addstr, strlen(addstr), true);;
			return -1;
		}
	}
}

void *UO_CompressTrietree::_newnode(const char *str,const int len,bool isentry)
{
	TRIENODE *newnode = m_mempool.newElement();
	strncpy(newnode->str,str,len);
	newnode->isentry = isentry;
	return newnode;
}

int UO_CompressTrietree::comparestr(const char *str1,const char *str2)
{
	int same = 0;
	while(*str1 != '\0' && *str1++ == *str2++)
		same++;
	return same;
}

int UO_CompressTrietree::_getindex(const char *ch)
{
	return *ch - 33;
}

bool UO_CompressTrietree::findstr_in_trietree(const char *str)
{
	char *pfind = const_cast<char*>(str);
	uint16_t index = _getindex(pfind);
	TRIENODE *node = m_trieroot.node[index];
	int offset = 0;
	while(node && (offset = comparestr(pfind,node->str)))
	{
		pfind+=offset;
		if(*pfind != '\0')
		{
			index = _getindex(pfind);
			if(node->node[index])
				node = node->node[index];
			else
				return false;
		}
		else
			return node->isentry && *(node->str + offset) == '\0'?true:false;
	}
	return false;
}

bool UO_CompressTrietree::deletestr_in_trietree(const char *str)
{
	char *pfind = const_cast<char*>(str);
	uint16_t index = _getindex(pfind);
	TRIENODE *node = m_trieroot.node[index];
	int offset = 0;
	while(node && (offset = comparestr(pfind,node->str)))
	{
		pfind+=offset;
		if(*pfind != '\0')
		{
			index = _getindex(pfind);
			if(node->node[index])
				node = node->node[index];
			else
				return false;
		}
		else
		{
			if(node->isentry && *(node->str + offset) == '\0')
			{
				node->isentry = false;
				return true;
			}
			else
				return false;
		}
	}
	return false;
}
