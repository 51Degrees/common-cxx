/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "weightedItem.h"
#include "memory.h"
#include "collection.h"
#include <assert.h>
#include <string.h>

void fiftyoneDegreesWeightedItemListInit(
	fiftyoneDegreesWeightedItemList *list,
	uint32_t initialCapacity,
	float loadFactor) {
	list->count = 0;
	list->capacity = initialCapacity;
	list->loadFactor = loadFactor;
	if (initialCapacity > 0) {
		list->items = (fiftyoneDegreesWeightedItem*)fiftyoneDegreesMalloc(
			sizeof(fiftyoneDegreesWeightedItem) * initialCapacity);
	}
	else {
		list->items = NULL;
	}
}

void fiftyoneDegreesWeightedItemListRelease(
	fiftyoneDegreesWeightedItemList *list) {
	uint32_t i;
	for (i = 0; i < list->count; i++) {
		FIFTYONE_DEGREES_COLLECTION_RELEASE(list->items[i].item.collection, &list->items[i].item);
	}
	list->count = 0;
}

void fiftyoneDegreesWeightedItemListFree(
	fiftyoneDegreesWeightedItemList *list) {
	fiftyoneDegreesWeightedItemListRelease(list);
	if (list->items != NULL) {
		fiftyoneDegreesFree(list->items);
		list->items = NULL;
	}
	list->capacity = 0;
}

void fiftyoneDegreesWeightedItemListExtend(
	fiftyoneDegreesWeightedItemList *list,
	uint32_t newCapacity) {
	// Allocate new list
	if (newCapacity > list->capacity) {
		const size_t newSize = newCapacity * sizeof(fiftyoneDegreesWeightedItem);
		fiftyoneDegreesWeightedItem * const newItems = 
			(fiftyoneDegreesWeightedItem*)fiftyoneDegreesMalloc(newSize);

		if (newItems == NULL) {
			return;
		}

		fiftyoneDegreesWeightedItem * const oldItems = list->items;
		if (oldItems != NULL) {
			const size_t oldSize = list->count * sizeof(fiftyoneDegreesWeightedItem);
			memcpy(newItems, oldItems, oldSize);
			fiftyoneDegreesFree(oldItems);
		}
		list->items = newItems;
		list->capacity = newCapacity;
	}
}

void fiftyoneDegreesWeightedItemListAdd(
	fiftyoneDegreesWeightedItemList *list,
	const fiftyoneDegreesWeightedItem *item) {
	assert(list->count < list->capacity);
	assert(item->item.collection != NULL);
	list->items[list->count++] = *item;
	// Check if the list has reached its load factor
	if (list->capacity > 0 && 
		(float)(list->count) / (float)(list->capacity) > list->loadFactor) {
		// Get new capacity
		const uint32_t newCapacity = 
			list->capacity * FIFTYONE_DEGREES_WEIGHTED_ITEM_LIST_RESIZE_FACTOR;
		fiftyoneDegreesWeightedItemListExtend(list, newCapacity);
	}
}
