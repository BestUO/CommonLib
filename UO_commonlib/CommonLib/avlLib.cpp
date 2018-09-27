#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "avlLib.h"
/* typedefs */

typedef struct
{
	AVL_NODE    avl;
	unsigned int        key;
}AVL_UNSIGNED_NODE;

/* defines */

#define AVL_MAX_HEIGHT 42

/*******************************************************************************
*
* avlRebalance - make sure the tree conforms to the AVL balancing rules, while
* preserving the ordering of the binary tree
*
* INTERNAL
* The AVL tree balancing rules are as follows :
* - the height of the left and right subtrees under a given node must never
*   differ by more than one
* - the height of a given subtree is defined as 1 plus the maximum height of
*   the subtrees under his root node
*
* The avlRebalance procedure must be called after a leaf node has been inserted
* or deleted from the tree. It checks that the AVL balancing rules are
* respected, makes local adjustments to the tree if necessary, recalculates
* the height field of the modified nodes, and repeats the process for every
* node up to the root node. This iteration is necessary because the balancing
* rules for a given node might have been broken by the modification we did on
* one of the subtrees under it.
*
* Because we need to iterate the process up to the root node, and the tree
* nodes does not contain pointers to their father node, we ask the caller of
* this procedure to keep a list of all the nodes traversed from the root node
* to the node just before the recently inserted or deleted node. This is the
* <ancestors> argument. Because each subtree might have to be re-rooted in the
* balancing operation, <ancestors> is actually a list pointers to the node
* pointers - thus if re-rooting occurs, the node pointers can be modified so
* that they keep pointing to the root of a given subtree.
*
* <count> is simply a count of elements in the <ancestors> list.
*
* RETURNS: N/A
*/

void avlRebalance
    (
    AVL_NODE ***    ancestors,  /* list of pointers to the ancestor
                       node pointers */
    int         count       /* number ancestors to rebalance */
    )
    {
    while (count > 0)
    {
    AVL_NODE ** nodepp; /* address of the pointer to the root node of
                   the current subtree */
    AVL_NODE *  nodep;  /* points to root node of current subtree */
    AVL_NODE *  leftp;  /* points to root node of left subtree */
    int     lefth;  /* height of the left subtree */
    AVL_NODE *  rightp; /* points to root node of right subtree */
    int     righth; /* height of the right subtree */

    /*
     * Find the current root node and its two subtrees. By construction,
     * we know that both of them conform to the AVL balancing rules.
     */

    nodepp = ancestors[--count];
    nodep = *nodepp;
    leftp = (AVL_NODE *)nodep->left;
    lefth = (leftp != NULL) ? leftp->height : 0;
    rightp = (AVL_NODE *)nodep->right;
    righth = (rightp != NULL) ? rightp->height : 0;

    if (righth - lefth < -1)
        {
        /*
         *         *
         *       /   \
         *    n+2      n
         *
         * The current subtree violates the balancing rules by beeing too
         * high on the left side. We must use one of two different
         * rebalancing methods depending on the configuration of the left
         * subtree.
         *
         * Note that leftp cannot be NULL or we would not pass there !
         */

        AVL_NODE *  leftleftp;  /* points to root of left left
                       subtree */
        AVL_NODE *  leftrightp; /* points to root of left right
                       subtree */
        int     leftrighth; /* height of left right subtree */

        leftleftp = (AVL_NODE *)leftp->left;
        leftrightp = (AVL_NODE *)leftp->right;
        leftrighth = (leftrightp != NULL) ? leftrightp->height : 0;

        if ((leftleftp != NULL) && (leftleftp->height >= leftrighth))
        {
        /*
         *            <D>                     <B>
         *             *                    n+2|n+3
         *           /   \                   /   \
         *        <B>     <E>    ---->    <A>     <D>
         *        n+2      n              n+1   n+1|n+2
         *       /   \                           /   \
         *    <A>     <C>                     <C>     <E>
         *    n+1    n|n+1                   n|n+1     n
         */

        nodep->left = leftrightp;   /* D.left = C */
        nodep->height = leftrighth + 1;
        leftp->right = nodep;       /* B.right = D */
        leftp->height = leftrighth + 2;
        *nodepp = leftp;        /* B becomes root */
        }
        else
        {
        /*
         *           <F>
         *            *
         *          /   \                        <D>
         *       <B>     <G>                     n+2
         *       n+2      n                     /   \
         *      /   \           ---->        <B>     <F>
         *   <A>     <D>                     n+1     n+1
         *    n      n+1                    /  \     /  \
         *          /   \                <A>   <C> <E>   <G>
         *       <C>     <E>              n  n|n-1 n|n-1  n
         *      n|n-1   n|n-1
         *
         * We can assume that leftrightp is not NULL because we expect
         * leftp and rightp to conform to the AVL balancing rules.
         * Note that if this assumption is wrong, the algorithm will
         * crash here.
         */

        leftp->right = leftrightp->left;    /* B.right = C */
        leftp->height = leftrighth;
        nodep->left = leftrightp->right;    /* F.left = E */
        nodep->height = leftrighth;
        leftrightp->left = leftp;       /* D.left = B */
        leftrightp->right = nodep;      /* D.right = F */
        leftrightp->height = leftrighth + 1;
        *nodepp = leftrightp;           /* D becomes root */
        }
        }
    else if (righth - lefth > 1)
        {
        /*
         *        *
         *      /   \
         *    n      n+2
         *
         * The current subtree violates the balancing rules by beeing too
         * high on the right side. This is exactly symmetric to the
         * previous case. We must use one of two different rebalancing
         * methods depending on the configuration of the right subtree.
         *
         * Note that rightp cannot be NULL or we would not pass there !
         */

        AVL_NODE *  rightleftp; /* points to the root of right left
                       subtree */
        int     rightlefth; /* height of right left subtree */
        AVL_NODE *  rightrightp;    /* points to the root of right right
                       subtree */

        rightleftp = (AVL_NODE *)rightp->left;
        rightlefth = (rightleftp != NULL) ? rightleftp->height : 0;
        rightrightp = (AVL_NODE *)rightp->right;

        if ((rightrightp != NULL) && (rightrightp->height >= rightlefth))
        {
        /*        <B>                             <D>
         *         *                            n+2|n+3
         *       /   \                           /   \
         *    <A>     <D>        ---->        <B>     <E>
         *     n      n+2                   n+1|n+2   n+1
         *           /   \                   /   \
         *        <C>     <E>             <A>     <C>
         *       n|n+1    n+1              n     n|n+1
         */

        nodep->right = rightleftp;  /* B.right = C */
        nodep->height = rightlefth + 1;
        rightp->left = nodep;       /* D.left = B */
        rightp->height = rightlefth + 2;
        *nodepp = rightp;       /* D becomes root */
        }
        else
        {
        /*        <B>
         *         *
         *       /   \                            <D>
         *    <A>     <F>                         n+2
         *     n      n+2                        /   \
         *           /   \       ---->        <B>     <F>
         *        <D>     <G>                 n+1     n+1
         *        n+1      n                 /  \     /  \
         *       /   \                    <A>   <C> <E>   <G>
         *    <C>     <E>                  n  n|n-1 n|n-1  n
         *   n|n-1   n|n-1
         *
         * We can assume that rightleftp is not NULL because we expect
         * leftp and rightp to conform to the AVL balancing rules.
         * Note that if this assumption is wrong, the algorithm will
         * crash here.
         */

        nodep->right = rightleftp->left;    /* B.right = C */
        nodep->height = rightlefth;
        rightp->left = rightleftp->right;   /* F.left = E */
        rightp->height = rightlefth;
        rightleftp->left = nodep;       /* D.left = B */
        rightleftp->right = rightp;     /* D.right = F */
        rightleftp->height = rightlefth + 1;
        *nodepp = rightleftp;           /* D becomes root */
        }
        }
    else
        {
        /*
         * No rebalancing, just set the tree height
         *
         * If the height of the current subtree has not changed, we can
         * stop here because we know that we have not broken the AVL
         * balancing rules for our ancestors.
         */

        int height;

        height = ((righth > lefth) ? righth : lefth) + 1;
        if (nodep->height == height)
        break;
        nodep->height = height;
        }
    }
    }

/*******************************************************************************
*
* avlSearch - search a node in an AVL tree
*
* At the time of the call, <root> is the root node pointer. <key> is the value
* we want to search, and <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* there is no ambiguity about which node will be found.
*
* Also note that the search procedure does not depend on the tree balancing
* rules, but because the tree is balanced, we know that the search procedure
* will always be efficient.
*
* RETURNS: pointer to the node whose key equals <key>, or NULL if there is
* no such node in the tree
*/
void * avlSearch1
    (
    AVL_TREE        root,           /* root node pointer */
    GENERIC_ARGUMENT    key,            /* search key */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE *  nodep;  /* pointer to the current node */

    nodep = root;
    while (1)
    {
    int delta;  /* result of the comparison operation */

    if (nodep == NULL)
        return NULL;    /* not found ! */

    delta = compare (nodep, key);
    if (0 == delta)
        return nodep;   /* found the node */
    else if (delta < 0)
        nodep = (AVL_NODE *)nodep->left;
    else
        nodep = (AVL_NODE *)nodep->right;
    }
    }
    void * avlSearch2
    (
    AVL_TREE        root,           /* root node pointer */
    GENERIC_ARGUMENT    key,            /* search key */
    std::function<int(void*,GENERIC_ARGUMENT)>  const &compare/* comparison function */
    )
    {
    AVL_NODE *  nodep;  /* pointer to the current node */

    nodep = root;
    while (1)
    {
    int delta;  /* result of the comparison operation */

    if (nodep == NULL)
        return NULL;    /* not found ! */

    delta = compare (nodep, key);
    if (0 == delta)
        return nodep;   /* found the node */
    else if (delta < 0)
        nodep = (AVL_NODE *)nodep->left;
    else
        nodep = (AVL_NODE *)nodep->right;
    }
    }

/*******************************************************************************
*
* avlSearchUnsigned - search a node in an AVL tree
*
* This is a specialized implementation of avlSearch for cases where the
* node to be searched is an AVL_UNSIGNED_NODE.
*
* RETURNS: pointer to the node whose key equals <key>, or NULL if there is
* no such node in the tree
*/

void * avlSearchUnsigned
    (
    AVL_TREE    root,   /* root node pointer */
    unsigned int    key /* search key */
    )
    {
    AVL_UNSIGNED_NODE * nodep;  /* pointer to the current node */

    nodep = (AVL_UNSIGNED_NODE *) root;
    while (1)
    {
    if (nodep == NULL)
        return NULL;    /* not found ! */

    if (key == nodep->key)
        return nodep;   /* found the node */
    else if (key < nodep->key)
        nodep = (AVL_UNSIGNED_NODE *)nodep->avl.left;
    else
        nodep = (AVL_UNSIGNED_NODE *)nodep->avl.right;
    }
    }

/*******************************************************************************
*
* avlInsert - insert a node in an AVL tree
*
* At the time of the call, <root> points to the root node pointer. This root
* node pointer is possibly NULL if the tree is empty. <newNode> points to the
* node we want to insert. His left, right and height fields need not be filled,
* but the user will probably have added his own data fields after those. <key>
* is newNode's key, that will be passed to the comparison function. This is
* redundant because it could really be derived from newNode, but the way to do
* this depends on the precise type of newNode so we cannot do this in a generic
* routine. <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* the insertion operation will fail if we try to insert a node that has a
* duplicate key.
*
* Also note that because we keep the tree balanced, the root node pointer that
* is pointed by the <root> argument can be modified in this function.
*
* INTERNAL
* The insertion routine works just like in a non-balanced binary tree : we
* walk down the tree like if we were searching a node, and when we reach a leaf
* node we insert newNode at this position.
*
* Because the balancing procedure needs to be able to walk back to the root
* node, we keep a list of pointers to the pointers we followed on our way down
* the tree.
*
* RETURNS: OK, or ERROR if the tree already contained a node with the same key
*/

int avlInsert
    (
    AVL_TREE *      root,       /* pointer to the root node ptr */
    void *      newNode,    /* ptr to the node we want to insert */
    GENERIC_ARGUMENT    key,        /* search key of newNode */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE ** nodepp;             /* ptr to current node ptr */
    AVL_NODE ** ancestor[AVL_MAX_HEIGHT];   /* list of pointers to all
                           our ancestor node ptrs */
    int     ancestorCount;          /* number of ancestors */

    nodepp = root;
    ancestorCount = 0;

    while (1)
    {
    AVL_NODE *  nodep;  /* pointer to the current node */
    int     delta;  /* result of the comparison operation */

    nodep = *nodepp;
    if (nodep == NULL)
        break;  /* we can insert a leaf node here ! */

    ancestor[ancestorCount++] = nodepp;

    delta = compare (nodep, key);
    if (0 == delta)
        return -1;
    else if (delta < 0)
        nodepp = (AVL_NODE **)&(nodep->left);
    else
        nodepp = (AVL_NODE **)&(nodep->right);
    }

    ((AVL_NODE *)newNode)->left = NULL;
    ((AVL_NODE *)newNode)->right = NULL;
    ((AVL_NODE *)newNode)->height = 1;
    *nodepp = (AVL_NODE *)newNode;

    avlRebalance (ancestor, ancestorCount);

    return 0;
    }

/*******************************************************************************
*
* avlInsertUnsigned - insert a node in an AVL tree
*
* This is a specialized implementation of avlInsert for cases where the
* node to be inserted is an AVL_UNSIGNED_NODE.
*
* RETURNS: OK, or ERROR if the tree already contained a node with the same key
*/

int avlInsertUnsigned
    (
    AVL_TREE *  root,   /* pointer to the root node ptr */
    void *  newNode /* ptr to the node we want to insert */
    )
    {
    AVL_UNSIGNED_NODE **    nodepp; /* ptr to current node ptr */
    AVL_UNSIGNED_NODE **    ancestor[AVL_MAX_HEIGHT];
            /* list of pointers to all our ancestor node ptrs */
    int     ancestorCount;      /* number of ancestors */
    unsigned int    key;

    key = ((AVL_UNSIGNED_NODE *)newNode)->key;
    nodepp = (AVL_UNSIGNED_NODE **) root;
    ancestorCount = 0;

    while (1)
    {
    AVL_UNSIGNED_NODE * nodep;  /* pointer to the current node */

    nodep = *nodepp;
    if (nodep == NULL)
        break;  /* we can insert a leaf node here ! */

    ancestor[ancestorCount++] = nodepp;

    if (key == nodep->key)
        return -1;
    else if (key < nodep->key)
        nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.left);
    else
        nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.right);
    }

    ((AVL_NODE *)newNode)->left = NULL;
    ((AVL_NODE *)newNode)->right = NULL;
    ((AVL_NODE *)newNode)->height = 1;
    *nodepp = (AVL_UNSIGNED_NODE *)newNode;

    avlRebalance ((AVL_NODE ***)ancestor, ancestorCount);

    return 0;
    }

/*******************************************************************************
*
* avlDelete - delete a node in an AVL tree
*
* At the time of the call, <root> points to the root node pointer and
* <key> is the key of the node we want to delete. <compare> is the
* user-provided comparison function.
*
* The deletion operation will of course fail if the desired node was not
* already in the tree.
*
* Also note that because we keep the tree balanced, the root node pointer that
* is pointed by the <root> argument can be modified in this function.
*
* On exit, the node is removed from the AVL tree but it is not free()'d.
*
* INTERNAL
* The deletion routine works just like in a non-balanced binary tree : we
* walk down the tree like searching the node we have to delete. When we find
* it, if it is not a leaf node, we have to replace it with a leaf node that
* has an adjacent key. Then the rebalancing operation will have to enforce the
* balancing rules by walking up the path from the leaf node that got deleted
* or moved.
*
* Because the balancing procedure needs to be able to walk back to the root
* node, we keep a list of pointers to the pointers we followed on our way down
* the tree.
*
* RETURNS: pointer to the node we deleted, or NULL if the tree does not
* contain any such node
*/

void * avlDelete
    (
    AVL_TREE *      root,   /* pointer to the root node pointer */
    GENERIC_ARGUMENT    key,    /* search key of node we want to delete */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE ** nodepp;             /* ptr to current node ptr */
    AVL_NODE *  nodep;              /* ptr to the current node */
    AVL_NODE ** ancestor[AVL_MAX_HEIGHT];   /* list of pointers to all our
                           ancestor node pointers */
    int ancestorCount;              /* number of ancestors */
    AVL_NODE *  deletep;            /* ptr to the node we have to
                           delete */

    nodepp = root;
    ancestorCount = 0;
    while (1)
    {
    int delta;      /* result of the comparison operation */

    nodep = *nodepp;
    if (nodep == NULL)
        return NULL;    /* node was not in the tree ! */

    ancestor[ancestorCount++] = nodepp;

    delta = compare (nodep, key);
    if (0 == delta)
        break;      /* we found the node we have to delete */
    else if (delta < 0)
        nodepp = (AVL_NODE **)&(nodep->left);
    else
        nodepp = (AVL_NODE **)&(nodep->right);
    }

    deletep = nodep;

    if (nodep->left == NULL)
    {
    /*
     * There is no node on the left subtree of delNode.
     * Either there is one (and only one, because of the balancing rules)
     * on its right subtree, and it replaces delNode, or it has no child
     * nodes at all and it just gets deleted
     */

    *nodepp = (AVL_NODE *)nodep->right;

    /*
     * we know that nodep->right was already balanced so we don't have to
     * check it again
     */

    ancestorCount--;
    }
    else
    {
    /*
     * We will find the node that is just before delNode in the ordering
     * of the tree and promote it to delNode's position in the tree.
     */

    AVL_NODE ** deletepp;       /* ptr to the ptr to the node
                           we have to delete */
    int     deleteAncestorCount;    /* place where the replacing
                           node will have to be
                           inserted in the ancestor
                           list */

    deleteAncestorCount = ancestorCount;
    deletepp = nodepp;
    deletep = nodep;

    /* search for node just before delNode in the tree ordering */

    nodepp = (AVL_NODE **)&(nodep->left);
    while (1)
        {
        nodep = *nodepp;
        if (nodep->right == NULL)
        break;
        ancestor[ancestorCount++] = nodepp;
        nodepp = (AVL_NODE **)&(nodep->right);
        }

    /*
     * this node gets replaced by its (unique, because of balancing rules)
     * left child, or deleted if it has no childs at all
     */

    *nodepp = (AVL_NODE *)nodep->left;

    /* now this node replaces delNode in the tree */

    nodep->left = deletep->left;
    nodep->right = deletep->right;
    nodep->height = deletep->height;
    *deletepp = nodep;

    /*
     * We have replaced delNode with nodep. Thus the pointer to the left
     * subtree of delNode was stored in delNode->left and it is now
     * stored in nodep->left. We have to adjust the ancestor list to
     * reflect this.
     */

    ancestor[deleteAncestorCount] = (AVL_NODE **)&(nodep->left);
    }

    avlRebalance (ancestor, ancestorCount);

    return deletep;
    }

/*******************************************************************************
*
* avlDeleteUnsigned - delete a node in an AVL tree
*
* This is a specialized implementation of avlDelete for cases where the
* node to be deleted is an AVL_UNSIGNED_NODE.
*
* RETURNS: pointer to the node we deleted, or NULL if the tree does not
* contain any such node
*/

void * avlDeleteUnsigned
    (
    AVL_TREE *  root,   /* pointer to the root node pointer */
    unsigned int    key /* search key of node we want to delete */
    )
    {
    AVL_UNSIGNED_NODE **    nodepp;     /* ptr to current node ptr */
    AVL_UNSIGNED_NODE *     nodep;      /* ptr to the current node */
    AVL_UNSIGNED_NODE **    ancestor[AVL_MAX_HEIGHT];
        /* list of pointers to all our ancestor node pointers */
    int             ancestorCount;  /* number of ancestors */
    AVL_UNSIGNED_NODE *     deletep;    /* ptr to the node we have to
                           delete */

    nodepp = (AVL_UNSIGNED_NODE **)root;
    ancestorCount = 0;
    while (1)
    {
    nodep = *nodepp;
    if (nodep == NULL)
        return NULL;    /* node was not in the tree ! */

    ancestor[ancestorCount++] = nodepp;

    if (key == nodep->key)
        break;      /* we found the node we have to delete */
    else if (key < nodep->key)
        nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.left);
    else
        nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.right);
    }

    deletep = nodep;

    if (nodep->avl.left == NULL)
    {
    /*
     * There is no node on the left subtree of delNode.
     * Either there is one (and only one, because of the balancing rules)
     * on its right subtree, and it replaces delNode, or it has no child
     * nodes at all and it just gets deleted
     */

    *nodepp = (AVL_UNSIGNED_NODE *)nodep->avl.right;

    /*
     * we know that nodep->right was already balanced so we don't have to
     * check it again
     */

    ancestorCount--;
    }
    else
    {
    /*
     * We will find the node that is just before delNode in the ordering
     * of the tree and promote it to delNode's position in the tree.
     */

    AVL_UNSIGNED_NODE **    deletepp;   /* ptr to the ptr to the node
                           we have to delete */
    int deleteAncestorCount;    /* place where the replacing node will
                       have to be inserted in the ancestor
                       list */

    deleteAncestorCount = ancestorCount;
    deletepp = nodepp;
    deletep = nodep;

    /* search for node just before delNode in the tree ordering */

    nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.left);
    while (1)
        {
        nodep = *nodepp;
        if (nodep->avl.right == NULL)
        break;
        ancestor[ancestorCount++] = nodepp;
        nodepp = (AVL_UNSIGNED_NODE **)&(nodep->avl.right);
        }

    /*
     * this node gets replaced by its (unique, because of balancing rules)
     * left child, or deleted if it has no childs at all
     */

    *nodepp = (AVL_UNSIGNED_NODE *)nodep->avl.left;

    /* now this node replaces delNode in the tree */

    nodep->avl.left = deletep->avl.left;
    nodep->avl.right = deletep->avl.right;
    nodep->avl.height = deletep->avl.height;
    *deletepp = nodep;

    /*
     * We have replaced delNode with nodep. Thus the pointer to the left
     * subtree of delNode was stored in delNode->left and it is now
     * stored in nodep->left. We have to adjust the ancestor list to
     * reflect this.
     */

    ancestor[deleteAncestorCount] = (AVL_UNSIGNED_NODE **)&(nodep->avl.left);
    }

    avlRebalance ((AVL_NODE ***)ancestor, ancestorCount);

    return deletep;
    }

/*******************************************************************************
*
* avlSuccessorGet - find node with key successor to input key on an AVL tree
*
* At the time of the call, <root> is the root node pointer. <key> is the value
* we want to search, and <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* there is no ambiguity about which node will be found.
*
* Also note that the search procedure does not depend on the tree balancing
* rules, but because the tree is balanced, we know that the search procedure
* will always be efficient.
*
* RETURNS: pointer to the node whose key is the immediate successor of <key>,
* or NULL if there is no such node in the tree
*/

void * avlSuccessorGet
    (
    AVL_TREE        root,           /* root node pointer */
    GENERIC_ARGUMENT    key,            /* search key */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE *  nodep;  /* pointer to the current node */
    AVL_NODE *  superiorp;  /* pointer to the current superior*/

    nodep = root;
    superiorp = NULL;
    while (1)
    {
    int delta;  /* result of the comparison operation */

    if (nodep == NULL)
        return superiorp;

    delta = compare (nodep, key);
    if (delta < 0)
        {
        superiorp = nodep; /* update superiorp */
        nodep = (AVL_NODE *)nodep->left;
        }
    else
        nodep = (AVL_NODE *)nodep->right;
    }
    }

/*******************************************************************************
*
* avlPredecessorGet - find node with key predecessor to input key on an AVL tree
*
* At the time of the call, <root> is the root node pointer. <key> is the value
* we want to search, and <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* there is no ambiguity about which node will be found.
*
* Also note that the search procedure does not depend on the tree balancing
* rules, but because the tree is balanced, we know that the search procedure
* will always be efficient.
*
* RETURNS: pointer to the node whose key is the immediate predecessor of <key>,
* or NULL if there is no such node in the tree
*/

void * avlPredecessorGet
    (
    AVL_TREE        root,           /* root node pointer */
    GENERIC_ARGUMENT    key,            /* search key */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE *  nodep;  /* pointer to the current node */
    AVL_NODE *  inferiorp;  /* pointer to the current inferior*/

    nodep = root;
    inferiorp = NULL;
    while (1)
    {
    int delta;  /* result of the comparison operation */

    if (nodep == NULL)
        return inferiorp;

    delta = compare (nodep, key);
    if (delta > 0)
        {
        inferiorp = nodep; /* update inferiorp */
        nodep = (AVL_NODE *)nodep->right;
        }
    else
        nodep = (AVL_NODE *)nodep->left;
    }
    }

/*******************************************************************************
*
* avlMinimumGet - find node with minimum key
*
* At the time of the call, <root> is the root node pointer. <key> is the value
* we want to search, and <compare> is the user-provided comparison function.
*
* RETURNS: pointer to the node with minimum key; NULL if the tree is empty
*/

void * avlMinimumGet
    (
    AVL_TREE        root           /* root node pointer */
    )
    {
    if  (NULL == root)
        return NULL;

    while (root->left != NULL)
        {
        root = (AVL_NODE *)root->left;
        }

    return root;
    }

/*******************************************************************************
*
* avlMaximumGet - find node with maximum key
*
* At the time of the call, <root> is the root node pointer. <key> is the value
* we want to search, and <compare> is the user-provided comparison function.
*
* RETURNS: pointer to the node with maximum key; NULL if the tree is empty
*/

void * avlMaximumGet
    (
    AVL_TREE        root           /* root node pointer */
    )
    {
    if  (NULL == root)
        return NULL;

    while (root->right != NULL)
        {
        root = (AVL_NODE *)root->right;
        }

    return root;
    }

/*******************************************************************************
*
* avlInsertInform - insert a node in an AVL tree and report key holder
*
* At the time of the call, <pRoot> points to the root node pointer. This root
* node pointer is possibly NULL if the tree is empty. <pNewNode> points to the
* node we want to insert. His left, right and height fields need not be filled,
* but the user will probably have added his own data fields after those. <key>
* is newNode's key, that will be passed to the comparison function. This is
* redundant because it could really be derived from newNode, but the way to do
* this depends on the precise type of newNode so we cannot do this in a generic
* routine. <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* the insertion operation will fail if we try to insert a node that has a
* duplicate key. However, if the <replace> boolean is set to true then in
* case of conflict we will remove the old node, we will put in its position the
* new one, and we will return the old node pointer in the postion pointed by
* <ppReplacedNode>.
*
* Also note that because we keep the tree balanced, the root node pointer that
* is pointed by the <pRoot> argument can be modified in this function.
*
* INTERNAL
* The insertion routine works just like in a non-balanced binary tree : we
* walk down the tree like if we were searching a node, and when we reach a leaf
* node we insert newNode at this position.
*
* Because the balancing procedure needs to be able to walk back to the root
* node, we keep a list of pointers to the pointers we followed on our way down
* the tree.
*
* RETURNS: OK, or ERROR if the tree already contained a node with the same key
* and replacement was not allowed. In both cases it will place a pointer to
* the key holder in the position pointed by <ppKeyHolder>.
*/

int avlInsertInform
    (
    AVL_TREE *          pRoot,              /* ptr to the root node pointer */
    void *              pNewNode,           /* pointer to the candidate node */
    GENERIC_ARGUMENT    key,                /* unique key of new node */
    void **             ppKeyHolder,        /* ptr to final key holder */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE ** nodepp;             /* ptr to current node ptr */
    AVL_NODE ** ancestor[AVL_MAX_HEIGHT];   /* list of pointers to all
                           our ancestor node ptrs */
    int     ancestorCount;          /* number of ancestors */

    if  (NULL == ppKeyHolder)
        {
        printf("invalid input data were passed to avlInsertInform\n");
        return -1;
        };

    nodepp = pRoot;
    ancestorCount = 0;

    while (1)
    {
    AVL_NODE *  nodep;  /* pointer to the current node */
    int     delta;  /* result of the comparison operation */

    nodep = *nodepp;
    if (nodep == NULL)
        break;  /* we can insert a leaf node here ! */

    ancestor[ancestorCount++] = nodepp;

    delta = compare (nodep, key);
    if  (0 == delta)
        {
        /* we inform the caller of the key holder node and return ERROR */
        *ppKeyHolder = nodep;
        return -1;
        }
    else if (delta < 0)
        nodepp = (AVL_NODE **)&(nodep->left);
    else
        nodepp = (AVL_NODE **)&(nodep->right);
    }

    ((AVL_NODE *)pNewNode)->left = NULL;
    ((AVL_NODE *)pNewNode)->right = NULL;
    ((AVL_NODE *)pNewNode)->height = 1;
    *nodepp = (AVL_NODE *)pNewNode;

    *ppKeyHolder = pNewNode;

    avlRebalance (ancestor, ancestorCount);

    return 0;
    }

/*******************************************************************************
*
* avlRemoveInsert - forcefully insert a node in an AVL tree
*
* At the time of the call, <pRoot> points to the root node pointer. This root
* node pointer is possibly NULL if the tree is empty. <pNewNode> points to the
* node we want to insert. His left, right and height fields need not be filled,
* but the user will probably have added his own data fields after those. <key>
* is newNode's key, that will be passed to the comparison function. This is
* redundant because it could really be derived from newNode, but the way to do
* this depends on the precise type of newNode so we cannot do this in a generic
* routine. <compare> is the user-provided comparison function.
*
* Note that we cannot have several nodes with the equal keys in the tree, so
* the insertion operation will fail if we try to insert a node that has a
* duplicate key. However, in case of conflict we will remove the old node, we
* will put in its position the new one, and we will return the old node pointer
*
* Also note that because we keep the tree balanced, the root node pointer that
* is pointed by the <pRoot> argument can be modified in this function.
*
* INTERNAL
* The insertion routine works just like in a non-balanced binary tree : we
* walk down the tree like if we were searching a node, and when we reach a leaf
* node we insert newNode at this position.
*
* Because the balancing procedure needs to be able to walk back to the root
* node, we keep a list of pointers to the pointers we followed on our way down
* the tree.
*
* RETURNS: NULL if insertion was carried out without replacement, or if
* replacement occured the pointer to the replaced node
*
*/

void * avlRemoveInsert
    (
    AVL_TREE *          pRoot,              /* ptr to the root node pointer */
    void *              pNewNode,           /* pointer to the candidate node */
    GENERIC_ARGUMENT    key,                /* unique key of new node */
    int compare (void *, GENERIC_ARGUMENT)  /* comparison function */
    )
    {
    AVL_NODE ** nodepp;             /* ptr to current node ptr */
    AVL_NODE ** ancestor[AVL_MAX_HEIGHT];   /* list of pointers to all
                           our ancestor node ptrs */
    int     ancestorCount;          /* number of ancestors */

    nodepp = pRoot;
    ancestorCount = 0;

    while (1)
    {
    AVL_NODE *  nodep;  /* pointer to the current node */
    int     delta;  /* result of the comparison operation */

    nodep = *nodepp;
    if (nodep == NULL)
        break;  /* we can insert a leaf node here ! */

    ancestor[ancestorCount++] = nodepp;

    delta = compare (nodep, key);
    if  (0 == delta)
        {
        /* we copy the tree data from the old node to the new node */
        ((AVL_NODE *)pNewNode)->left = nodep->left;
        ((AVL_NODE *)pNewNode)->right = nodep->right;
        ((AVL_NODE *)pNewNode)->height = nodep->height;

        /* and we make the new node child of the old node's parent */
        *nodepp = (AVL_NODE *)pNewNode;

        /* before we return it we sterilize the old node */
        nodep->left = NULL;
        nodep->right = NULL;
        nodep->height = 1;

        return nodep;
        }
    else if (delta < 0)
        nodepp = (AVL_NODE **)&(nodep->left);
    else
        nodepp = (AVL_NODE **)&(nodep->right);
    }

    ((AVL_NODE *)pNewNode)->left = NULL;
    ((AVL_NODE *)pNewNode)->right = NULL;
    ((AVL_NODE *)pNewNode)->height = 1;
    *nodepp = (AVL_NODE *)pNewNode;

    avlRebalance (ancestor, ancestorCount);

    return NULL;
    }

/*******************************************************************************
*
* avlTreeWalk- walk the whole tree and execute a function on each node
*
* At the time of the call, <pRoot> points to the root node pointer.
*
* RETURNS: OK always
*
*/

int avlTreeWalk(AVL_TREE * pRoot, void walkExec(AVL_TREE * ppNode))
    {
    if  ((NULL == pRoot) || (NULL == *pRoot))
        {
        return 0;
        };

    if  (!(NULL == (*pRoot)->left))
        {
        avlTreeWalk((AVL_TREE *)(&((*pRoot)->left)), walkExec);
        }

    if  (!(NULL == (*pRoot)->right))
        {
        avlTreeWalk((AVL_TREE *)(&((*pRoot)->right)), walkExec);
        }

    walkExec(pRoot);

    return 0;
	}

/*******************************************************************************
*
* avlTreePrint- print the whole tree
*
* At the time of the call, <pRoot> points to the root node pointer.
*
* RETURNS: OK always
*
*/

int avlTreePrint(AVL_TREE * pRoot, void printNode(void * nodep))
    {
    if  ((NULL == pRoot) || (NULL == *pRoot))
        {
        return 0;
        };

    printNode(*pRoot);

    if  (!(NULL == (*pRoot)->left))
        {
        avlTreePrint((AVL_TREE *)(&((*pRoot)->left)), printNode);
        }

    if  (!(NULL == (*pRoot)->right))
        {
        avlTreePrint((AVL_TREE *)(&((*pRoot)->right)), printNode);
        }

    return 0;
	}

/*******************************************************************************
*
* avlTreeErase - erase the whole tree
*
* At the time of the call, <pRoot> points to the root node pointer.
* Unlike the avlDelete routine here we do perform memory management
* ASSUMING that all nodes carry shallow data only. Otherwise we should
* use avlTreeWalk with the appropriate walkExec memory freeing function.
* Since we do not plan to reuse the tree intermediate rebalancing is not needed.
*
* RETURNS: OK always
*
*/

int avlTreeErase(AVL_TREE * pRoot)
    {
    if  ((NULL == pRoot) || (NULL == *pRoot))
        {
        return 0;
        };

    if  (!(NULL == (*pRoot)->left))
        {
        avlTreeErase((AVL_TREE *)(&((*pRoot)->left)));
        free((*pRoot)->left);
        (*pRoot)->left = NULL;
        }

    if  (!(NULL == (*pRoot)->right))
        {
        avlTreeErase((AVL_TREE *)(&((*pRoot)->right)));
        free((*pRoot)->right);
        (*pRoot)->right = NULL;
        }

    free(*pRoot);
    *pRoot = NULL;
    return 0;
	}

/*******************************************************************************
*
* avlTreePrintErase - erase the whole tree assuming that all nodes were
* created using malloc.
*
* At the time of the call, <pRoot> points to the root node pointer.
* Unlike the avlDelete routine here we do perform memory management.
* Since we do not plan to reuse the tree intermediate rebalancing is not needed.
*
* RETURNS: OK always and assigns NULL to *pRoot.
*
*/

int avlTreePrintErase(AVL_TREE * pRoot, void printNode(void * nodep))
    {
    if  ((NULL == pRoot) || (NULL == *pRoot))
        {
        return 0;
        };

    printNode(*pRoot);

    if  (!(NULL == (*pRoot)->left))
        {
        avlTreePrintErase((AVL_TREE *)(&((*pRoot)->left)), printNode);
        free((*pRoot)->left);
        (*pRoot)->left = NULL;
        }

    if  (!(NULL == (*pRoot)->right))
        {
        avlTreePrintErase((AVL_TREE *)(&((*pRoot)->right)), printNode);
        free((*pRoot)->right);
        (*pRoot)->right = NULL;
        }

    free(*pRoot);
    *pRoot = NULL;
    return 0;
	}

