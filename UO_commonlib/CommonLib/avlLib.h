#ifndef __INCavlLibh
#define __INCavlLibh
#include <functional>
/* typedefs */

typedef struct
{
	void * left;
	void * right;
	int height;
}AVL_NODE;

typedef AVL_NODE * AVL_TREE;

typedef union
{
	int i;
	unsigned int u;
	void * p;
}GENERIC_ARGUMENT;

/* function declarations */

void avlRebalance(AVL_NODE *** ancestors, int count);

void * avlSearch1(AVL_TREE root, GENERIC_ARGUMENT key,
          int compare(void *, GENERIC_ARGUMENT));

void * avlSearch2(AVL_TREE root, GENERIC_ARGUMENT key,
          std::function<int(void*,GENERIC_ARGUMENT)>  const &compare);

void * avlSuccessorGet(AVL_TREE root, GENERIC_ARGUMENT key,
         int compare(void *, GENERIC_ARGUMENT));

void * avlPredecessorGet (AVL_TREE root, GENERIC_ARGUMENT key,
         int compare(void *, GENERIC_ARGUMENT));

void * avlMinimumGet(AVL_TREE root);

void * avlMaximumGet(AVL_TREE root);

int avlInsert(AVL_TREE * root, void * newNode, GENERIC_ARGUMENT key,
          int compare (void *, GENERIC_ARGUMENT));

int avlInsertInform(AVL_TREE * pRoot, void * pNewNode, GENERIC_ARGUMENT key,
          void ** ppKeyHolder,
          int compare(void *, GENERIC_ARGUMENT));

void * avlRemoveInsert(AVL_TREE * pRoot, void * pNewNode, GENERIC_ARGUMENT key,
          int compare (void *, GENERIC_ARGUMENT));

void * avlDelete(AVL_TREE * root, GENERIC_ARGUMENT key,
          int compare (void *, GENERIC_ARGUMENT));

int avlTreeWalk(AVL_TREE * pRoot, void walkExec(AVL_TREE * nodepp));

int avlTreePrint(AVL_TREE * pRoot, void printNode(void * nodep));

int avlTreeErase(AVL_TREE * pRoot);

int avlTreePrintErase(AVL_TREE * pRoot, void printNode(void * nodep));

/* specialized implementation functions */

void * avlSearchUnsigned(AVL_TREE root, unsigned int key);

int avlInsertUnsigned(AVL_TREE * root, void * newNode);

void * avlDeleteUnsigned(AVL_TREE * root, unsigned int key);

#endif
