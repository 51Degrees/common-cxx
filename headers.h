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

#ifndef FIFTYONEDEGREES_HEADERS_H_INCLUDED
#define FIFTYONEDEGREES_HEADERS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include "list.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef struct fiftyone_degrees_headers_t {
	fiftyoneDegreesList names; /* List of unique HTTP header string from a 
							   strings collection */
	int *uniqueIds; /* List of unique HTTP header ids corresponding to names */
	bool useUpperPrefixedHeaders; /* True if an upper case prefixed checks are 
								  should be used */
	void*(*malloc)(size_t); /* Used to add strings to the headers lists */
	void(*free)(void*); /* Used to free the memory used by the headers */
} fiftyoneDegreesHeaders;

typedef uint32_t(*fiftyoneDegreesHeadersGet)(
	void *state,
	uint32_t index, 
	fiftyoneDegreesCollectionItem *nameItem);

fiftyoneDegreesHeaders* fiftyoneDegreesHeadersCreate(
	void *state,
	int count,
	bool useUpperPrefixedHeaders,
	fiftyoneDegreesHeadersGet getHeaderMethod,
	void*(*malloc)(size_t),
	void(*free)(void*));

int fiftyoneDegreesHeaderGetIndex(
	fiftyoneDegreesHeaders *headers,
	const char* httpHeaderName,
	size_t length);

uint32_t fiftyoneDegreesHeadersGetUniqueId(
	fiftyoneDegreesHeaders *headers, int index);

void fiftyoneDegreesHeadersFree(fiftyoneDegreesHeaders *headers);

#endif