/* *********************************************************************
 * This Source Code Form is copyright of 51Degrees Mobile Experts Limited.
 * Copyright 2017 51Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY
 *
 * This Source Code Form is the subject of the following patents and patent
 * applications, owned by 51Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent No. 2871816;
 * European Patent Application No. 17184134.9;
 * United States Patent Nos. 9,332,086 and 9,350,823; and
 * United States Patent Application No. 15/686,066.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.
 *
 * If a copy of the MPL was not distributed with this file, You can obtain
 * one at http://mozilla.org/MPL/2.0/.
 *
 * This Source Code Form is "Incompatible With Secondary Licenses", as
 * defined by the Mozilla Public License, v. 2.0.
 ********************************************************************** */

#include "cache.h"

// Define NDEBUG if needed, to ensure asserts are not called in release builds.
#if !defined(DEBUG) && !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif
#include <assert.h>

 /**
 * RED BLACK BINARY TREE METHODS
 */

 /**
 * Implementation of a classic red black binary tree adapted to support the
 * result set structure used in the LRU cache. Several important considerations
 * should be noted with this implementation.
 *
 * 1. The maximum number of entries in the tree is known when the tree is
 *    created. All memory allocation is performed at initialisation.
 * 2. Once the tree is full it will remain full and never shrinks. The memory
 *    used is freed when the cache is freed.
 * 3. The node in the tree also contains other data such as the linked list
 *    pointers used to identify the first and last entry in the cache, and
 *    the cache data itself. See structure fiftyoneDegreesCacheNode.
 * 4. The cache structure fiftyoneDegreesCacheNodeCache contains special
 *    fields "empty" and "root". "Empty" is used in place of NULL to indicate
 *    that the left, right or parent pointer of the node has no data. The use
 *    of "empty" makes the algorithm more efficient as the data structure used
 *    to indicate no data is the same as a valid data structure and therefore
 *    does not require custom logic. The "root" fields left pointer is used as
 *    the start of the tree. Similarly the parent element being a valid data
 *    structure simplifies the algorithm.
 *
 * Developers modifying this section of code should be familiar with the red
 * black tree design template. Code comments assume an understanding of the
 * principles involved. For further information see:
 * https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 */

 /**
 * Macros used in the red black tree within the cache.
 */

#define TREE_COLOUR_RED  1 // Indicates the node is black
#define TREE_COLOUR_BLACK 0 // Indicates the node is red

 /* Gets the root of the tree. The TREE_FIRST macro gets the first node. */
#define TREE_ROOT(c) &c->root

 /* Gets the empty node used to indicate no further data. */
#define TREE_EMPTY(c) &c->empty

 /* Gets the first node under the root. */
#define TREE_FIRST(c) c->root.treeLeft

 /**
 * \cond
 * Used by assert statements to validate the number of entries in the cache for
 * debugging should any changes be made to the logic. Should not be compiled in
 * release builds.
 * @param current pointer to the node being counted.
 * @returns the number of children plus 1 for this current node.
 * \endcond
 */
static int32_t cacheTreeCount(fiftyoneDegreesCacheNode *current) {
	int32_t count = 0;
	if (current != TREE_EMPTY(current->shard)) {
		count = 1;
		if (current->treeLeft != TREE_EMPTY(current->shard)) {
			count += cacheTreeCount(current->treeLeft);
			assert(current->key > current->treeLeft->key);
		}
		if (current->treeRight != TREE_EMPTY(current->shard)) {
			count += cacheTreeCount(current->treeRight);
			assert(current->key < current->treeRight->key);
		}
	}
	return count;
}

/**
* \cond
* Rotates the red black tree node to the left.
* @param node pointer to the node being rotated.
* \endcond
*/
static void cacheTreeRotateLeft(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheNode *child = node->treeRight;
	node->treeRight = child->treeLeft;

	if (child->treeLeft != TREE_EMPTY(node->shard)) {
		child->treeLeft->treeParent = node;
	}
	child->treeParent = node->treeParent;

	if (node == node->treeParent->treeLeft) {
		node->treeParent->treeLeft = child;
	}
	else {
		node->treeParent->treeRight = child;
	}

	child->treeLeft = node;
	node->treeParent = child;
}

/**
* \cond
* Rotates the red black tree node to the right.
* @param node pointer to the node being rotated.
* \endcond
*/
static void cacheTreeRotateRight(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheNode *child = node->treeLeft;
	node->treeLeft = child->treeRight;

	if (child->treeRight != TREE_EMPTY(node->shard)) {
		child->treeRight->treeParent = node;
	}
	child->treeParent = node->treeParent;

	if (node == node->treeParent->treeLeft) {
		node->treeParent->treeLeft = child;
	}
	else {
		node->treeParent->treeRight = child;
	}

	child->treeRight = node;
	node->treeParent = child;
}

/**
* \cond
* Maintains the properties of the binary tree following an insert.
* @param node pointer to the node being repaired after insert.
* \endcond
*/
static void cacheTreeInsertRepair(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheNode *uncle;

	while (node->treeParent->colour == TREE_COLOUR_RED) {
		if (node->treeParent == node->treeParent->treeParent->treeLeft) {
			uncle = node->treeParent->treeParent->treeRight;
			if (uncle->colour == TREE_COLOUR_RED) {
				node->treeParent->colour = TREE_COLOUR_BLACK;
				uncle->colour = TREE_COLOUR_BLACK;
				node->treeParent->treeParent->colour = TREE_COLOUR_RED;
				node = node->treeParent->treeParent;
			}
			else {
				if (node == node->treeParent->treeRight) {
					node = node->treeParent;
					cacheTreeRotateLeft(node);
				}
				node->treeParent->colour = TREE_COLOUR_BLACK;
				node->treeParent->treeParent->colour = TREE_COLOUR_RED;
				cacheTreeRotateRight(node->treeParent->treeParent);
			}
		}
		else {
			uncle = node->treeParent->treeParent->treeLeft;
			if (uncle->colour == TREE_COLOUR_RED) {
				node->treeParent->colour = TREE_COLOUR_BLACK;
				uncle->colour = TREE_COLOUR_BLACK;
				node->treeParent->treeParent->colour = TREE_COLOUR_RED;
				node = node->treeParent->treeParent;
			}
			else {
				if (node == node->treeParent->treeLeft) {
					node = node->treeParent;
					cacheTreeRotateRight(node);
				}
				node->treeParent->colour = TREE_COLOUR_BLACK;
				node->treeParent->treeParent->colour = TREE_COLOUR_RED;
				cacheTreeRotateLeft(node->treeParent->treeParent);
			}
		}
	}
}

/**
* \cond
* Inserts the node into the red black tree.
* @param node pointer to the node being inserted.
* \endcond
*/
static void cacheTreeInsert(const fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheShard *shard = node->shard;
	fiftyoneDegreesCacheNode *current = TREE_FIRST(shard);
	fiftyoneDegreesCacheNode *parent = TREE_ROOT(shard);

	// Work out the correct point to insert the node.
	while (current != TREE_EMPTY(shard)) {
		parent = current;
		assert(node->key != current->key);
		current = node->key < current->key
			? current->treeLeft
			: current->treeRight;
	}

	// Set up the node being inserted in the tree.
	current = (fiftyoneDegreesCacheNode*)node;
	current->treeLeft = TREE_EMPTY(shard);
	current->treeRight = TREE_EMPTY(shard);
	current->treeParent = parent;
	if (parent == TREE_ROOT(shard) ||
		current->key < parent->key) {
		parent->treeLeft = current;
	}
	else {
		parent->treeRight = current;
	}
	current->colour = TREE_COLOUR_RED;

	cacheTreeInsertRepair(current);

	TREE_FIRST(shard)->colour = TREE_COLOUR_BLACK;
}

/**
* \cond
* Returns the node that matches the hash code provided.
* @param cache to search in
* @param hash key to get the item for
* @returns the corresponding node if it exists in the cache, otherwise
* null.
* \endcond
*/
static fiftyoneDegreesCacheNode* cacheTreeFind(
	fiftyoneDegreesCacheShard *shard,
	int64_t hash) {
	int32_t iterations = 0;
	fiftyoneDegreesCacheNode *current = TREE_FIRST(shard);

	while (current != TREE_EMPTY(shard)) {
		iterations++;
		if (hash == current->key) {
			return current;
		}
		current = hash < current->key
			? current->treeLeft
			: current->treeRight;
	}

	return NULL;
}

/**
* \cond
* Finds the successor for the node provided.
* @param node pointer to the node whose successor is required.
* @returns the successor for the node which may be empty.
* \endcond
*/
static fiftyoneDegreesCacheNode* cacheTreeSuccessor(
	fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheShard *shard = node->shard;
	fiftyoneDegreesCacheNode *successor = node->treeRight;
	if (successor != TREE_EMPTY(shard)) {
		while (successor->treeLeft != TREE_EMPTY(shard)) {
			successor = successor->treeLeft;
		}
	}
	else {
		for (successor = node->treeParent;
			node == successor->treeRight;
			successor = successor->treeParent) {
			node = successor;
		}
		if (successor == TREE_ROOT(shard)) {
			successor = TREE_EMPTY(shard);
		}
	}
	return successor;
}

/**
* \cond
* Following a deletion repair the section of the tree impacted.
* @param node pointer to the node below the one deleted.
* \endcond
*/
static void cacheTreeDeleteRepair(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheShard *shard = node->shard;
	fiftyoneDegreesCacheNode *sibling;

	while (node->colour == TREE_COLOUR_BLACK && node != TREE_FIRST(shard)) {
		if (node == node->treeParent->treeLeft) {
			sibling = node->treeParent->treeRight;
			if (sibling->colour == TREE_COLOUR_RED) {
				sibling->colour = TREE_COLOUR_BLACK;
				node->treeParent->colour = TREE_COLOUR_RED;
				cacheTreeRotateLeft(node->treeParent);
				sibling = node->treeParent->treeRight;
			}
			if (sibling->treeRight->colour == TREE_COLOUR_BLACK &&
				sibling->treeLeft->colour == TREE_COLOUR_BLACK) {
				sibling->colour = TREE_COLOUR_RED;
				node = node->treeParent;
			}
			else {
				if (sibling->treeRight->colour == TREE_COLOUR_BLACK) {
					sibling->treeLeft->colour = TREE_COLOUR_BLACK;
					sibling->colour = TREE_COLOUR_RED;
					cacheTreeRotateRight(sibling);
					sibling = node->treeParent->treeRight;
				}
				sibling->colour = node->treeParent->colour;
				node->treeParent->colour = TREE_COLOUR_BLACK;
				sibling->treeRight->colour = TREE_COLOUR_BLACK;
				cacheTreeRotateLeft(node->treeParent);
				node = TREE_FIRST(shard);
			}
		}
		else {
			sibling = node->treeParent->treeLeft;
			if (sibling->colour == TREE_COLOUR_RED) {
				sibling->colour = TREE_COLOUR_BLACK;
				node->treeParent->colour = TREE_COLOUR_RED;
				cacheTreeRotateRight(node->treeParent);
				sibling = node->treeParent->treeLeft;
			}
			if (sibling->treeRight->colour == TREE_COLOUR_BLACK &&
				sibling->treeLeft->colour == TREE_COLOUR_BLACK) {
				sibling->colour = TREE_COLOUR_RED;
				node = node->treeParent;
			}
			else {
				if (sibling->treeLeft->colour == TREE_COLOUR_BLACK) {
					sibling->treeRight->colour = TREE_COLOUR_BLACK;
					sibling->colour = TREE_COLOUR_RED;
					cacheTreeRotateLeft(sibling);
					sibling = node->treeParent->treeLeft;
				}
				sibling->colour = node->treeParent->colour;
				node->treeParent->colour = TREE_COLOUR_BLACK;
				sibling->treeLeft->colour = TREE_COLOUR_BLACK;
				cacheTreeRotateRight(node->treeParent);
				node = TREE_FIRST(shard);
			}
		}
	}
	node->colour = TREE_COLOUR_BLACK;
}

/**
* \cond
* Removes the node from the tree so that it can be used again to store
* another result. The node will come from the last item in the cache's
* linked list.
* @param node pointer to be deleted.
* \endcond
*/
static void cacheTreeDelete(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheShard *shard = node->shard;
	fiftyoneDegreesCacheNode *x, *y;

	if (node->treeLeft == TREE_EMPTY(shard) ||
		node->treeRight == TREE_EMPTY(shard)) {
		y = node;
	}
	else {
		y = cacheTreeSuccessor(node);
	}
	x = y->treeLeft == TREE_EMPTY(shard) ? y->treeRight : y->treeLeft;

	x->treeParent = y->treeParent;
	if (x->treeParent == TREE_ROOT(shard)) {
		TREE_FIRST(shard) = x;
	}
	else {
		if (y == y->treeParent->treeLeft) {
			y->treeParent->treeLeft = x;
		}
		else {
			y->treeParent->treeRight = x;
		}
	}

	if (y->colour == TREE_COLOUR_BLACK) {
		cacheTreeDeleteRepair(x);
	}
	if (y != node) {
		y->treeLeft = node->treeLeft;
		y->treeRight = node->treeRight;
		y->treeParent = node->treeParent;
		y->colour = node->colour;
		node->treeLeft->treeParent = y;
		node->treeRight->treeParent = y;
		if (node == node->treeParent->treeLeft) {
			node->treeParent->treeLeft = y;
		}
		else {
			node->treeParent->treeRight = y;
		}
	}
}

/**
* \cond
* Validates the shard by checking the number of entries in the linked list and
* the tree. Used by assert statements to validate the integrity of the cache
* during development. Should not be compiled in release builds.
* @param cache pointer to the cache being validated.
* @returns always return 0 as the purpose is to execute asserts in debug builds.
* \endcond
*/
#ifndef NDEBUG

static int cacheValidateShard(const fiftyoneDegreesCacheShard *shard) {
	int linkedListEntriesForward = 0;
	int linkedListEntriesBackwards = 0;
	int binaryTreeEntries = 0;
	fiftyoneDegreesCacheNode *node;

	// Check the list from first to last.
	node = shard->first;
	while (node != NULL &&
		linkedListEntriesForward <= shard->allocated) {
		linkedListEntriesForward++;
		node = node->listNext;
	}
	assert(linkedListEntriesForward == shard->allocated ||
		linkedListEntriesForward == shard->allocated - 1);

	// Check the list from last to first.
	node = shard->last;
	while (node != NULL &&
		linkedListEntriesBackwards <= shard->allocated) {
		linkedListEntriesBackwards++;
		node = node->listPrevious;
	}
	assert(linkedListEntriesBackwards == shard->allocated ||
		linkedListEntriesBackwards == shard->allocated - 1);

	// Check the binary tree. We need to remove one because the root
	// node doesn't contain any data.
	binaryTreeEntries = cacheTreeCount(TREE_FIRST(shard));
	assert(binaryTreeEntries == shard->allocated ||
		binaryTreeEntries == shard->allocated - 1);

	return 0;
}

static int cacheValidate(const fiftyoneDegreesCache *cache) {
	int i = 0;
	for (i = 0; i < cache->concurrency; i++) {
		cacheValidateShard(&cache->shards[i]);
	}
}

#endif

/**
* CACHE LOCK METHODS
*/

#ifndef FIFTYONEDEGREES_NO_THREADING

static void cacheLockCreate(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_CREATE(lock->readLock);
	FIFTYONEDEGREES_SIGNAL_CREATE(lock->writeLock);
	assert(lock->readLock != NULL);
	assert(lock->writeLock != NULL);
	lock->readers = 0;
}

static void cacheLockClose(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_CLOSE(lock->readLock);
	FIFTYONEDEGREES_SIGNAL_CLOSE(lock->writeLock);
}

static void cacheLockBeginWrite(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_WAIT(lock->writeLock);
}

static void cacheLockEndWrite(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_SET(lock->writeLock);
}

static void cacheLockBeginRead(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_WAIT(lock->readLock);
	if (lock->readers == 0) {
		FIFTYONEDEGREES_SIGNAL_WAIT(lock->writeLock);
	}
	lock->readers++;
	FIFTYONEDEGREES_SIGNAL_SET(lock->readLock);
}

static void cacheLockEndRead(fiftyoneDegreesCacheLock *lock) {
	FIFTYONEDEGREES_SIGNAL_WAIT(lock->readLock);
	lock->readers--;
	if (lock->readers == 0) {
		cacheLockEndWrite(lock);
	}
	FIFTYONEDEGREES_SIGNAL_SET(lock->readLock);
}

#else

static void cacheLockCreate(fiftyoneDegreesCacheLock *lock) {}

static void cacheLockClose(fiftyoneDegreesCacheLock *lock) {}

static void cacheLockBeginRead(fiftyoneDegreesCacheLock *lock) {}

static void cacheLockEndRead(fiftyoneDegreesCacheLock *lock) {}

static void cacheLockBeginWrite(fiftyoneDegreesCacheLock *lock) {}

static void cacheLockEndWrite(fiftyoneDegreesCacheLock *lock) {}

#endif

static void cacheInitShard(fiftyoneDegreesCacheShard *shard) {
	int i;
	fiftyoneDegreesCacheNode *current;

	// Configure the empty node.
	current = TREE_EMPTY(shard);
	current->treeLeft = TREE_EMPTY(shard);
	current->treeRight = TREE_EMPTY(shard);
	current->treeParent = TREE_EMPTY(shard);
	current->colour = TREE_COLOUR_BLACK;
	current->key = 0;
	current->shard = shard;

	// Configure the fake root node to avoid splitting the root.
	current = TREE_ROOT(shard);
	current->treeLeft = TREE_EMPTY(shard);
	current->treeRight = TREE_EMPTY(shard);
	current->treeParent = TREE_EMPTY(shard);
	current->colour = TREE_COLOUR_BLACK;
	current->key = 0;
	current->shard = shard;

	// Set the linked list for the shard.
	shard->first = shard->nodes;
	shard->first->listNext = NULL;
	shard->first->listPrevious = NULL;
	shard->last = shard->first;
#ifndef FIFTYONEDEGREES_NO_THREADING
	cacheLockCreate(&shard->lock);
#endif

	// Set the default values for an empty cache.
	for (i = 0; i < shard->capacity; i++) {
		current = &shard->nodes[i];
		current->shard = shard;
		current->data.ptr = (byte*)NULL;
		current->data.allocated = 0;
		current->data.length = 0;
		if (shard->last != current) {
			// Add this node to the end of the shard's linked list.
			current->listPrevious = shard->last;
			current->listNext = NULL;
			shard->last->listNext = current;
			shard->last = current;
		}
#ifndef FIFTYONEDEGREES_NO_THREADING
		current->activeCount = 0;
#endif
	}

	// In debug check the validity of the linked list.
	assert(shard->first != NULL);
	assert(shard->last != NULL);
	assert(shard->first->listNext != NULL);
	assert(shard->last->listPrevious != NULL);
	assert(shard->first->listPrevious == NULL);
	assert(shard->last->listNext == NULL);
}

/**
 * \cond
 * Initialises the cache by setting pointers for the linked list and binary
 * tree.
 * @param cache pointer to the cache to be initialised
 * @param init optional external init function to call
 * \endcond
 */
static void cacheInit(
	fiftyoneDegreesCache *cache) {
	int i;
	fiftyoneDegreesCacheShard *shard;
	for (i = 0; i < cache->concurrency; i++) {
		shard = &cache->shards[i];
		shard->cache = cache;
		shard->capacity = cache->capacity / cache->concurrency;
		shard->allocated = 0;
		shard->nodes = &cache->nodes[shard->capacity * i];
		cacheInitShard(shard);
	}
}

/**
 * CACHE METHODS
 */

/**
 * \cond
 * Moves the node to the head of the linked list. Used when a node
 * which is already in the cache is being returned and the cache needs to know
 * that is has recently been used and shouldn't be considered for removal. Also
 * used when a new entry is added to the cache.
 * The list must be locked for write before the method is called.
 * @param node pointer to the node to move to the head of the linked list.
 * \endcond
 */
static void cacheMoveToHead(fiftyoneDegreesCacheNode *node) {
	fiftyoneDegreesCacheShard *shard = node->shard;

	// Only consider moving if not already the first entry.
	if (shard->first != node) {

		// If the entry is the last one in the list then set the last pointer
		// to the entry before the last one.
		if (node == shard->last) {
			assert(node->listNext == NULL);
			assert(node->listPrevious != NULL);
			shard->last = shard->last->listPrevious;
			shard->last->listNext = NULL;
		}
		else {
			assert(node->listNext != NULL);
			assert(node->listPrevious != NULL);
			node->listPrevious->listNext = node->listNext;
			node->listNext->listPrevious = node->listPrevious;
		}

		// Position this node as the first entry in the list.
		node->listNext = shard->first;
		node->listPrevious = NULL;
		shard->first->listPrevious = node;
		shard->first = node;
	}

	// Validate the state of the list.
	assert(shard->first == node);
	assert(shard->first->listPrevious == NULL);
	assert(shard->first->listNext != NULL);
	assert(shard->last->listPrevious != NULL);
}

/**
 * \cond
 * Returns the next free node from the cache which can be used to add a
 * new entry to. Once the cache is full then the node returned is the one
 * at the end of the linked list which will contain the least recently used
 * data.
 * The cache must be write locked before calling this method.
 * @param cache cache to return the next free node from.
 * @returns a pointer to a free node.
 * \endcond
 */
static fiftyoneDegreesCacheNode *cacheGetNextFree(
		fiftyoneDegreesCacheShard *shard) {
	int countBefore, countAfter;

	fiftyoneDegreesCacheNode *node; // The oldest node in the shard.

	if (shard->allocated < shard->capacity) {
		// Return the free element at the end of the cache and update
		// the number of allocated elements.
		node = &shard->nodes[shard->allocated++];
	}
	else {
		// Use the oldest element in the list.
		node = shard->last;

		// Move the last element to the head of the list as it's about to be
		// populated.
		cacheMoveToHead(node);

		// Remove the last result from the binary tree.
		countBefore = cacheTreeCount(TREE_FIRST(shard));
		cacheTreeDelete(node);
		countAfter = cacheTreeCount(TREE_FIRST(shard));
		assert(countBefore - 1 == countAfter);
	}

	// Set the pointers of the node to null indicating that the
	// entry is not part of the dictionary anymore.
	node->treeLeft = NULL;
	node->treeRight = NULL;
	node->treeParent = NULL;
	node->colour = 0;

	return node;
}

/**
 * EXTERNAL CACHE METHODS
 */

/**
 * \cond
 * Creates a new cache.The cache must be destroyed with the
 * fiftyoneDegreesFreeCache method.
 * @param capacity number of items that the cache will store
 * @param concurrency maximum number of concurrent requests the cache should
 *		  expect to be able to service
 * @param malloc function to allocate memory
 * @param free function to free memory
 * @param load function to load a new item with a key into the cache
 * @param params configuration passed to the load function in the 2nd parameter
 * @returns a pointer to the cache created, or NULL if one was not created.
 * \endcond
 */
fiftyoneDegreesCache* fiftyoneDegreesCacheCreate(
	int32_t capacity,
	int32_t concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*),
	void(*load)(const fiftyoneDegreesCache*, const void*, fiftyoneDegreesData*, long),
	const void *params) {
	size_t cacheSize, nodesSize, shardsSize;
	fiftyoneDegreesCache *cache;

	// The capacity of the cache must allow for a minimum of one entry in
	// each shard.
	if (capacity < concurrency) {
		return NULL;
	}

	// Work out the total amount of memory used by the cache. Keep the list
	// of nodes and header together so they're in the same continuous memory
	// space and can be allocated and freed in a single operation.
	shardsSize = sizeof(fiftyoneDegreesCacheShard) * concurrency;
	nodesSize = sizeof(fiftyoneDegreesCacheNode) * capacity;
	cacheSize = sizeof(fiftyoneDegreesCache) + shardsSize + nodesSize;
	cache = (fiftyoneDegreesCache*)malloc(cacheSize);
	if (cache != NULL) {

		// The shards are set to the byte after the header.
		cache->shards = (fiftyoneDegreesCacheShard*)(cache + 1);

		// The nodes are set to the byte after the shards.
		cache->nodes = (fiftyoneDegreesCacheNode*)(&cache->shards[concurrency]);

		// Set the parameters for the cache.
		cache->mallocCacheData = malloc;
		cache->freeCacheData = free;
		cache->loadCacheData = load;
		cache->params = params;
		cache->hits = 0;
		cache->misses = 0;
		cache->concurrency = concurrency;
		cache->capacity = capacity;

		// Initialise the linked lists and binary tree.
		cacheInit(cache);
	}
	return cache;
}

static void cacheShardFree(fiftyoneDegreesCacheShard *shard) {
	int i;
	fiftyoneDegreesCacheNode *node;
	for (i = 0; i < shard->capacity; i++) {
		node = &shard->nodes[i];
		if (node->data.ptr != NULL && node->data.allocated > 0) {
			shard->cache->freeCacheData(node->data.ptr);
			fiftyoneDegreesDataReset(&shard->nodes[i].data);
		}
	}
#ifndef FIFTYONEDEGREES_NO_THREADING
	cacheLockClose(&shard->lock);
#endif
}

/**
 * \cond
 * Frees the cache structure, all allocated nodes and their data.
 * @param cache to be freed
 * \endcond
 */
void fiftyoneDegreesCacheFree(fiftyoneDegreesCache *cache) {
	int i;

	// Free any data items that are created and are marked to be freed by the
	// cache and shard locks.
	for (i = 0; i < cache->concurrency; i++) {
		cacheShardFree(&cache->shards[i]);
	}

	// Finally free all the memory used by the cache.
	cache->freeCacheData(cache);
}

/**
 * \cond
 * Gets an item from the cache. If an item is not in the cache, it is loaded
 * using the loader the cache was initialized with.
 *
 * The cache being used as a loading cache must have a load method defined
 * which returns a pointer to the data relating to the key used. This method
 * may, or may not, allocate memory or free memory previously allocated to
 * data in the cache node.
 *
 * Note that this cache is not entirely thread safe. Although all cache
 * operations are thread safe, any pointers returned are not reserved
 * while they are in use, so another thread may remove the entry from the
 * cache, changing the entry being used by the first thread. For this
 * reason, when a loading cache is being used, only one thread should use it at
 * once. In practical terms, this means that when compiling 51Degrees.c with
 * FIFTYONEDEGREES_INDIRECT, FIFTYONEDEGREES_NO_THREADING must also be
 * defined.
 * @param cache to get the entry from
 * @param key that indicates the item to get
 * @returns void* pointer to the requested item
 * \endcond
 */
fiftyoneDegreesCacheNode* fiftyoneDegreesCacheGet(
		fiftyoneDegreesCache *cache,
		const long key) {
	fiftyoneDegreesCacheShard *shard =
		&cache->shards[key % cache->concurrency];
	bool exists = true;

	// Check if the key already exists in the cache shard.
	cacheLockBeginRead(&shard->lock);
	fiftyoneDegreesCacheNode *node = cacheTreeFind(shard, key);
	cacheLockEndRead(&shard->lock);

	if (node == NULL) {
#ifndef FIFTYONEDEGREES_NO_THREADING
		// The key does not exist. Lock the cache and check again to confirm
		// that another thread has not loaded the same value.
		cacheLockBeginWrite(&shard->lock);
		node = cacheTreeFind(shard, key);
		if (node == NULL) {
			// The key is still not in the cache so use the loader to get the
			// value.
			node = cacheGetNextFree(shard);
			cache->loadCacheData(cache, cache->params, &node->data, key);
			node->key = key;
			cacheTreeInsert(node);
			exists = false;
		}
		cacheLockEndWrite(&shard->lock);
#else
		// The key does not exist. Get the next free node and set it's key and
		// data from the loader.
		node = cacheGetNextFree(shard);
		cache->loadCacheData(cache, cache->params, &node->data, key);
		node->key = key;
		cacheTreeInsert(node);
		exists = FALSE;
#endif
	}

	// Move the node to the head of it's list and update the cache statistics
	// for both misses and hits if in debug mode.
	if (exists == true) {
		cacheLockBeginWrite(&shard->lock);
		cacheMoveToHead(node);
		cacheLockEndWrite(&shard->lock);

#ifndef FIFTYONEDEGREES_NO_THREADING
		FIFTYONEDEGREES_INTERLOCK_INC(&cache->hits);
#else
		cache->hits++;
#endif
	}
	else
	{
#ifndef FIFTYONEDEGREES_NO_THREADING
		FIFTYONEDEGREES_INTERLOCK_INC(&cache->misses);
#else
		cache->misses++;
#endif
	}

	// Increase the active count for the node being returned.
#ifndef FIFTYONEDEGREES_NO_THREADING
	FIFTYONEDEGREES_INTERLOCK_INC(&node->activeCount);
#endif

	// Check that data is present for the node.
#ifndef FIFTYONEDEGREES_NO_THREADING
	assert(node->activeCount > 0);
#endif
	assert(node->data.ptr != NULL);

	return node;
}

void fiftyoneDegreesCacheRelease(fiftyoneDegreesCacheNode* node) {
#ifndef FIFTYONEDEGREES_NO_THREADING
	FIFTYONEDEGREES_INTERLOCK_DEC(&node->activeCount);
#endif
}
