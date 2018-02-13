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

#include "headers.h"

#include <stdlib.h>
#ifdef _MSC_FULL_VER
#include <string.h>
#else
#include <strings.h>
#define _stricmp strcasecmp
#endif

/* HTTP header prefix used when processing collections of parameters. */
#define HTTP_PREFIX_UPPER "HTTP_"

static bool doesHeaderExist(
	fiftyoneDegreesHeaders *headers, 
	fiftyoneDegreesCollectionItem *item) {
	uint32_t i;
	fiftyoneDegreesString *compare = (fiftyoneDegreesString*)item->data.ptr;
	fiftyoneDegreesString *test;
	for (i = 0; i < headers->names.count; i++) {
		test = fiftyoneDegreesListGetAsString(&headers->names, i);
		if (compare->size == test->size &&
			strncmp(
				FIFTYONEDEGREES_STRING(compare),
				FIFTYONEDEGREES_STRING(test), 
				compare->size) == 0) {
			return true;
		}
	}
	return false;
}

static void addUniqueHeaders(
	fiftyoneDegreesHeaders *headers,
	void *state,
	int count,
	fiftyoneDegreesHeadersGet getHeaderMethod) {
	int i;
	uint32_t uniqueId;
	fiftyoneDegreesCollectionItem nameItem;
	headers->names.count = 0;
	for (i = 0; i < count; i++) {
		fiftyoneDegreesDataReset(&nameItem.data);
		nameItem.collection = NULL;
		nameItem.handle = NULL;
		uniqueId = getHeaderMethod(state, i, &nameItem);
		if (doesHeaderExist(headers, &nameItem) == false) {
			headers->uniqueIds[headers->names.count] = uniqueId;
			fiftyoneDegreesListAdd(&headers->names, &nameItem);
		}
		else {
			nameItem.collection->release(&nameItem);
		}
	}
}

static fiftyoneDegreesHeaders* createHeaders(
	void*(*malloc)(size_t), 
	void(*free)(void*),
	int count) {
	fiftyoneDegreesHeaders *headers = (fiftyoneDegreesHeaders*)malloc(
		sizeof(fiftyoneDegreesHeaders));
	if (headers != NULL) {
		headers->uniqueIds = (uint32_t*)malloc(count * sizeof(uint32_t));
		if (headers->uniqueIds != NULL) {
			fiftyoneDegreesListInit(&headers->names, count, malloc, free);
			headers->malloc = malloc;
			headers->free = free;
		}
		else {
			free(headers);
			headers = NULL;
		}
	}
	return headers;
}

fiftyoneDegreesHeaders* fiftyoneDegreesHeadersCreate(
	void *state,
	int count,
	bool useUpperPrefixedHeaders,
	fiftyoneDegreesHeadersGet getHeaderMethod,
	void*(*malloc)(size_t),
	void(*free)(void*)) {
	fiftyoneDegreesHeaders *headers = createHeaders(malloc, free, count);
	if (headers != NULL) {
		headers->useUpperPrefixedHeaders = useUpperPrefixedHeaders;
		addUniqueHeaders(headers, state, count, getHeaderMethod);
	}
	return headers;
}

uint32_t fiftyoneDegreesHeadersGetUniqueId(
	fiftyoneDegreesHeaders *headers, int index) {
	return headers->uniqueIds[index];
}

int fiftyoneDegreesHeaderGetIndex(
	fiftyoneDegreesHeaders *headers,
	const char* httpHeaderName,
	size_t length) {
	uint32_t i;
	fiftyoneDegreesString *compare;

	// Check if header is from a Perl or PHP wrapper in the form of HTTP_*
	// and if present skip these characters.
	if (headers->useUpperPrefixedHeaders == true &&
		strncmp(
		httpHeaderName,
		HTTP_PREFIX_UPPER,
		sizeof(HTTP_PREFIX_UPPER) - 1) == 0) {
		httpHeaderName += sizeof(HTTP_PREFIX_UPPER) - 1;
	}

	// Perform a case insensitive compare of the remaining characters.
	for (i = 0; i < headers->names.count; i++) {
		compare = fiftyoneDegreesListGetAsString(&headers->names, i);
		if (compare->size - 1 == length &&
			_stricmp(httpHeaderName, FIFTYONEDEGREES_STRING(compare)) == 0) {
			return i;
		}
	}

	return -1;
}

void fiftyoneDegreesHeadersFree(fiftyoneDegreesHeaders *headers) {
	if (headers != NULL) {
		fiftyoneDegreesListFree(&headers->names);
		headers->free(headers->uniqueIds);
		headers->free(headers);
	}
}