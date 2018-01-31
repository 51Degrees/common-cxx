#include "list.h"

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

fiftyoneDegreesList* fiftyoneDegreesListInit(
	fiftyoneDegreesList *list,
	fiftyoneDegreesCollection *collection,
	int capacity,
	void*(*malloc)(size_t __size)) {
	list->items = (fiftyoneDegreesCollectionItem*)malloc(
		capacity * sizeof(fiftyoneDegreesCollectionItem));
	if (list->items == NULL) {
		return NULL;
	}
	list->collection = collection;
	list->capacity = capacity;
	list->count = 0;
	return list;
}

void fiftyoneDegreesListAdd(
	fiftyoneDegreesList *list,
	fiftyoneDegreesCollectionItem *item) {
	assert(list->count < list->capacity);
	list->items[list->count++] = *item;
}

fiftyoneDegreesString* fiftyoneDegreesListGetAsString(
	fiftyoneDegreesList *list,
	int index) {
	return (fiftyoneDegreesString*)list->items[index].data.ptr;
}

void fiftyoneDegreesListFree(fiftyoneDegreesList *list) {
	int i;
	if (list->items != NULL) {
		for (i = 0; i < list->count; i++) {
			list->collection->release(&list->items[i]);
		}
		list->collection->freeMemory(list->items);
		list->items = NULL;
		list->capacity = 0;
		list->count = 0;
	}
}

void fiftyoneDegreesListReset(fiftyoneDegreesList *list) {
	list->items = NULL;
	list->collection = NULL;
	list->count = 0;
	list->capacity = 0;
	list->memoryToFree = NULL;
}