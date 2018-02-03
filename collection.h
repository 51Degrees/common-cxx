#include "cache.h"
#include "file.h"
#include "memory.h"
#include <assert.h>

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

#ifndef FIFTYONEDEGREES_COLLECTION_H_INCLUDED
#define FIFTYONEDEGREES_COLLECTION_H_INCLUDED

/**
 * TODO - Add introduction documentation.
 */

#include <stdio.h>
#include <string.h>

typedef struct t_fiftyone_degrees_collection_config {
	uint32_t loaded; // Number of items to load into memory from the start of the collection
	int capacity; // Number of items the cache should store, 0 for no cache
	int concurrency; // Expected number of concurrent requests
} fiftyoneDegreesCollectionConfig;

typedef struct fiftyoneDegrees_collection_t fiftyoneDegreesCollection;
typedef struct fiftyoneDegrees_collection_item_t fiftyoneDegreesCollectionItem;
typedef struct fiftyoneDegrees_collection_file_t fiftyoneDegreesCollectionFile;

typedef struct fiftyoneDegrees_collection_item_t {
	fiftyoneDegreesData data; /* Item data */
	void *handle; /* A handle that relates to the data. i.e. a cache node */
	fiftyoneDegreesCollection *collection; /* Collection the item came from */
} fiftyoneDegreesCollectionItem;

typedef fiftyoneDegreesCollectionItem* (*fiftyoneDegreesCollectionGetMethod)(
	fiftyoneDegreesCollection*,
	uint32_t,
	fiftyoneDegreesCollectionItem*);

typedef void* (*fiftyoneDegreesCollectionFileRead)(
	const fiftyoneDegreesCollectionFile*,
	fiftyoneDegreesData*,
	uint32_t);

typedef void (*fiftyoneDegreesCollectionReleaseMethod)(
	fiftyoneDegreesCollectionItem*);

typedef void(*fiftyoneDegreesCollectionFreeMethod)(
	fiftyoneDegreesCollection*);

typedef struct fiftyoneDegrees_collection_t {
	fiftyoneDegreesCollectionGetMethod get; /* Gets an entry into the item */
	fiftyoneDegreesCollectionReleaseMethod release; /* Releases the item handle */
	fiftyoneDegreesCollectionFreeMethod freeCollection; /* Frees collection memory */
	void(*freeMemory)(void*);
	void *state; /* Implementation data for memory, cache or file */
	fiftyoneDegreesCollection *next; /* The next collection implementation or
										NULL */
	uint32_t count; /* The number of items, or 0 if not available */
	uint32_t elementSize; /* The size of each entry, or 0 if variable length */
	uint32_t size; /* Number of bytes in the source data structure containing
					  the collection's data */
} fiftyoneDegreesCollection;

typedef struct fiftyoneDegrees_collection_memory_t {
	fiftyoneDegreesCollection *collection; /* The generic collection */
	byte *firstByte; /* The first byte in memory of the collection */
	byte *lastByte; /* The last byte in memory of the collection */
	byte *memoryToFree; /* Memory to free when freeing the collection, or NULL
						if no memory to free*/
} fiftyoneDegreesCollectionMemory;

typedef struct fiftyoneDegrees_collection_file_t {
	fiftyoneDegreesCollection *collection; /* The generic collection */
	fiftyoneDegreesFileReader *reader; /* Reader used to load items into the
									      cache, or NULL if no cache */
	long offset; /* Offset to the collection in the source data structure */
	fiftyoneDegreesCollectionFileRead read;
	void*(*malloc)(size_t __size);
} fiftyoneDegreesCollectionFile;

typedef struct fiftyoneDegrees_collection_cache_t {
	fiftyoneDegreesCollection *source; /* The source collection used to load
									   items into the cache */
	fiftyoneDegreesCache *cache; /* Cache to use as data source */
} fiftyoneDegreesCollectionCache;

/**
 *
 * @param file a file handle positioned at the start of the collection
 * @param reader a pool of file handles to use operationally to retrieve data
 * from the file
 * @param config settings for the implementation of the collection to be used
 * @param elementSize the number of bytes in each item, or 0 if variable
 * @param isCount true if the first 4 byte integer is a count and not the
 * number of bytes in the collection
 * @param malloc
 * @param free
 * @param read a pointer to a function to read an item into the collection
 */
fiftyoneDegreesCollection* fiftyoneDegreesCollectionCreateFromFile(
	FILE *file,
	fiftyoneDegreesFileReader *reader,
	fiftyoneDegreesCollectionConfig *config,
	uint32_t elementSize,
	int isCount,
	void*(*malloc)(size_t),
	void(*free)(void*),
	fiftyoneDegreesCollectionFileRead read);

fiftyoneDegreesCollection* fiftyoneDegreesCollectionCreateFromMemory(
	fiftyoneDegreesMemoryReader *reader,
	uint32_t elementSize,
	int isCount,
	void*(*malloc)(size_t));

/**
 * Used with collections where each item is a fixed number of bytes recorded in
 * elementSize. The method will read that number of bytes into the data item
 * ensuring sufficient memory is allocated.
 */
void* fiftyoneDegreesReadFileFixed(
	const fiftyoneDegreesCollectionFile *file,
	fiftyoneDegreesData *data,
	uint32_t index);

#endif
