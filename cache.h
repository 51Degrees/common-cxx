/* *********************************************************************
 * This Source Code Form is copyright of 51Degrees Mobile Experts Limited.
 * Copyright 2017 51Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY
 *
 * This Source Code Form is the subject of the following patent
 * applications, owned by 51Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent Application No. 17184134.9;
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.
 *
 * If a copy of the MPL was not distributed with this file, You can obtain
 * one at http://mozilla.org/MPL/2.0/.
 *
 * This Source Code Form is “Incompatible With Secondary Licenses”, as
 * defined by the Mozilla Public License, v. 2.0.
 ********************************************************************** */

#ifndef FIFTYONEDEGREES_CACHE_H_INCLUDED
#define FIFTYONEDEGREES_CACHE_H_INCLUDED
#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

#include <stdint.h>
#include <stdbool.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "data.h"
#ifndef FIFTYONEDEGREES_NO_THREADING
#include "threading.h"
#endif

#ifndef FIFTYONEDEGREES_NO_THREADING
typedef struct fiftyoneDegrees_cache_lock_t {
	fiftyoneDegreesSignal *readLock; /* Locks for read operations */
	fiftyoneDegreesSignal *writeLock; /* Locks for write operations */
	volatile int readers; /* Count of the number of active readers */
} fiftyoneDegreesCacheLock;
#else
typedef fiftyoneDegreesCacheLock;
#endif

/* Initial definition of the cache structure. */
typedef struct fiftyoneDegrees_cache_node_t fiftyoneDegreesCacheNode;
typedef struct fiftyoneDegrees_cache_shard_t fiftyoneDegreesCacheShard;
typedef struct fiftyoneDegrees_cache_t fiftyoneDegreesCache;

/**
 * Cache node structure used for storing data in the cache along with
 * its key.
 */
typedef struct fiftyoneDegrees_cache_node_t {
	int64_t key; /* Key associated with the data value */
	fiftyoneDegreesData data; /* Data contained in the node */
	fiftyoneDegreesCacheShard *shard;/* Shard the node is associated with */
	fiftyoneDegreesCacheNode *listPrevious; /* Previous node or NULL if first */
	fiftyoneDegreesCacheNode *listNext; /* Next node or NULL if last */
	fiftyoneDegreesCacheNode *treeParent; /* Parent node or NULL if root */
	fiftyoneDegreesCacheNode *treeLeft; /* Left node or NULL if none */
	fiftyoneDegreesCacheNode *treeRight; /* Right node or NULL if none */
	byte colour; /* The colour of the node in the red black tree */
#ifndef FIFTYONEDEGREES_NO_THREADING
	volatile long activeCount; /* Number of threads actively using the
									   data*/
#endif
} fiftyoneDegreesCacheNode;

typedef struct fiftyoneDegrees_cache_shard_t {
	fiftyoneDegreesCache *cache; /* Pointer to the cache to which the node
								    belongs */
	fiftyoneDegreesCacheNode root; /* Root node of the red black tree */
	fiftyoneDegreesCacheNode empty; /* Empty node - set to black */
	int32_t capacity; /* Capacity of the shard */
	int32_t allocated; /* Number of node currently used in the shard */
	fiftyoneDegreesCacheNode *nodes; /* Pointer to the array of all nodes */
	fiftyoneDegreesCacheNode *first; /* Pointer to the first node in the
									    linked list */
	fiftyoneDegreesCacheNode *last; /* Pointer to the last node in the
									   linked list */
	fiftyoneDegreesCacheLock lock; /* Read write lock used to control access
									  to the shard*/
} fiftyoneDegreesCacheShard;

/**
* Cache structure to store the root of the red black tree and a list of
* allocated cache nodes. This also contains cache metrics and pointers to
* methods used when being used as a loading cache.
*/
struct fiftyoneDegrees_cache_t {
	fiftyoneDegreesCacheShard *shards; /* Array of shards / concurrency */
	fiftyoneDegreesCacheNode *nodes; /* Array of nodes / capacity */
	int32_t concurrency; /* Expected concurrency / number of shards */
	int32_t capacity; /* Capacity of the cache */
#ifndef FIFTYONEDEGREES_NO_THREADING
	volatile long hits; 
#else
	unsigned long hits;
#endif
	/* The number of times an item was found in the cache */
#ifndef FIFTYONEDEGREES_NO_THREADING
	volatile long misses;
#else
	unsigned long misses;
#endif
	/* The number of times an item was not found in the cache */
	void*(*mallocCacheData)(size_t __size); /* Function to allocate memory */
	void(*freeCacheData)(void*); /* Function to free memory */
	/* Used by the cache to load an item into the cache */
	void(*loadCacheData)(
		const fiftyoneDegreesCache*,
		const void*,
		fiftyoneDegreesData*,
		long);
	const void* params; /* Cache loader specific state */
};

/**
 * \ingroup FiftyOneDgreesFunctions
 * Creates a new cache.The cache must be destroyed with the
 * fiftyoneDegreesFreeCache method.
 * @param capacity maximum number of items that the cache should store
 * @param concurrency the expected number of parallel operations
 * @param malloc pointer to method used to allocate memory for cache entries
 * @param free pointer to method used to free cache entries
 * @param load pointer to method used to load an entry into the cache
 * @returns a pointer to the cache created, or NULL if one was not created.
 */
EXTERNAL fiftyoneDegreesCache *fiftyoneDegreesCacheCreate(
	int32_t capacity,
	int32_t concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*),
	void(*load)(const fiftyoneDegreesCache*, const void*, fiftyoneDegreesData*, long),
	const void *state);

/**
 * \ingroup FiftyOneDegreesFunctions
 * Frees the cache structure, all allocated nodes and their data.
 * @param cache to be freed
 */
EXTERNAL void fiftyoneDegreesCacheFree(fiftyoneDegreesCache *cache);

/**
 * \ingroup FiftyOneDegreesFunctions
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
 * FIFTYONEDEGREES_STREAM_MODE, FIFTYONEDEGREES_NO_THREADING must also be
 * defined.
 * @param cache to get the entry from
 * @param key that indicates the item to get
 * @returns void* pointer to the requested item
 */
EXTERNAL fiftyoneDegreesCacheNode* fiftyoneDegreesCacheGet(
		fiftyoneDegreesCache *cache,
		const long key);

EXTERNAL void fiftyoneDegreesCacheRelease(fiftyoneDegreesCacheNode *node);

#endif
