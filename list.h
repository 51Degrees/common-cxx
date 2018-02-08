#include "collection.h"
#include "string.h"

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

#ifndef FIFTYONEDEGREES_LIST_H_INCLUDED
#define FIFTYONEDEGREES_LIST_H_INCLUDED

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef struct fiftyoneDegrees_list_t {
	fiftyoneDegreesCollectionItem *items; // Array of items
	uint32_t capacity; // Capacity of the list to store items
	uint32_t count; // Number of items currently in the list 
	void(*free)(void*); // Used to free memory.
} fiftyoneDegreesList;

EXTERNAL fiftyoneDegreesList* fiftyoneDegreesListInit(
	fiftyoneDegreesList *list,
	int capacity,
	void*(*malloc)(size_t __size),
	void(*free)(void*));

EXTERNAL void fiftyoneDegreesListAdd(
	fiftyoneDegreesList *list,
	fiftyoneDegreesCollectionItem *item);

EXTERNAL fiftyoneDegreesString* fiftyoneDegreesListGetAsString(
	fiftyoneDegreesList *list,
	int index);

EXTERNAL void fiftyoneDegreesListFree(fiftyoneDegreesList *list);

#endif