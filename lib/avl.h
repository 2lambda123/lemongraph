#ifndef _AVL_H
#define _AVL_H

#include<sys/types.h>

/*
 * The goals of this AVL tree implimentation are:
 *  * constant time reset
 *  * efficient upserts
 *
 * Tree operations return pointers to client keys, allowing client to directly
 * manipulate stored objects, with some restrictions detailed below.
 *
 * All key memory management is the responsibility of the client.
 */

/*
 * opaque tree and node structs
 */
typedef struct _avl_tree_t * avl_tree_t;
typedef struct _avl_node_t * avl_node_t;

/*
 * callback to determine sort order within tree
 * must return less-than-zero/zero/greater-than-zero if user_key is less-than/equal-to/greater-than tree_node_key
 */
typedef int (*avl_cmp_func)(void *user_key, void *tree_node_key, void *data);

/*
 * callback prototype for avl_walk()
 */
typedef int (*avl_walk_cb)(void **key, void *data);


/*
 * initialize new avl tree object
 * notes:
 *  pre-allocates one page
 *  optionally supply key compare function
 *  optionally supply data arg to be passed as 3rd arg to cmp()
 */
avl_tree_t avl_new(avl_cmp_func cmp, void *data);

/*
 * initialize new avl tree object
 * notes:
 *  pre-allocates one page
 *  optionally supply key compare function
 *  optionally supply data arg to be passed as 3rd arg to cmp()
 *  optionally indicate number of nodes (ish) to allocate at once
 *   will pad out (sizeof(struct _avl_node_t) * nodes) to multiple of OS page size
 *   at least one node's worth of memory is reserved from each page for bookkeeping, more for the first page
 */
avl_tree_t avl_new2(avl_cmp_func cmp, void *data, size_t nodes);

/*
 * destroy tree
 * runtime: O(m), where m is number of pages required to hold node structures
 * notes:
 *  node sizes are 16/32 bytes each, on 32/64 bit architectures
 *  some space is used internally from each page (one or two nodes-worth)
 */
void avl_free(avl_tree_t tree);

/*
 * instantly reset tree contents
 * runtime: O(1)
 * notes:
 *  memory is not released
 */
void avl_reset(avl_tree_t tree);

/*
 * reset tree contents and release all but first page
 * runtime: O(m), where m is number of pages required to hold node structures
 */
void avl_release(avl_tree_t tree);

/*
 * attempt to insert key into tree
 * runtime: O(log(n)), where n is number of tree nodes
 *  on success: return pointer to key found/inserted
 *  on failure: return NULL
 * notes:
 *  returned pointer is guaranteed to be valid until node is deleted, or avl_reset()/avl_release()/avl_free()
 *  you may update/replace the existing key, if returned, as long as it would not alter sort order
 */
void **avl_insert(avl_tree_t tree, void *key);


/*
 * attempt to locate key in tree
 * runtime: O(log(n)), where n is number of tree nodes
 *  on success: return pointer to existing key
 *  on failure: return NULL
 * notes:
 *  returned pointer is valid until node is deleted, or avl_reset()/avl_release()/avl_free()
 *  you may update/replace the existing key, if returned, as long as it would not alter sort order
 */
void **avl_find(avl_tree_t tree, void *key);

/*
 * attempt to delete key from tree
 * runtime: O(log(n)), where n is number of tree nodes
 *  on success: return pointer to found key
 *  on failure: return NULL
 * notes:
 *  returned pointer is guaranteed to be valid until next avl_insert()/avl_reset()/avl_release()/avl_free()
 */
void **avl_delete(avl_tree_t tree, void *key);

/*
 * in order tree traversal in ascending/descending order (with desc: zero/non-zero, respectively)
 * return whether traversal was halted (1) or not (0)
 * runtime: O(n), where n is number of tree nodes
 * notes:
 *  halts if supplied callback returns non-zero
 *  do not call avl_insert()/avl_delete() while traversing
 *  client may alter keys here, as long as either of the following is true:
 *   a) sort order is not altered
 *   b) there will be no further calls to avl_insert()/avl_delete()
 *      before calling avl_reset()/avl_release()/avl_free()
 */
int avl_walk(avl_tree_t tree, avl_walk_cb cb, void *data, int desc);

/*
 * returns node height in tree for key pointers provided via avl_insert()/avl_find()/avl_delete()/avl_walk()
 * runtime: O(1)
 */
int avl_node_height(void **key);

/*
 * return current height of tree root (-1 for empty tree)
 * runtime: O(1)
 */
int avl_height(avl_tree_t tree);

/*
 * return current number of nodes in tree
 * runtime: O(1)
 */
size_t avl_size(avl_tree_t tree);

/*
 * return allocated memory size (not including user-supplied keys)
 * runtime: O(1)
 */
size_t avl_mem(avl_tree_t tree);

/*
 * return number of pages
 * runtime: O(1)
 */
int avl_pages(avl_tree_t tree);

#endif
