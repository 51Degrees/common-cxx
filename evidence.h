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

#ifndef FIFTYONEDEGREES_EVIDENCE_H_INCLUDED
#define FIFTYONEDEGREES_EVIDENCE_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ip.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef unsigned char byte;

typedef enum e_fiftyone_degrees_evidence_header_prefix {
	FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING = 0,
	FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES = 1, // A list of IP addresses as a string to be parsed into a IP addresses collection.
	FIFTYONEDEGREES_EVIDENCE_SERVER = 2,
	FIFTYONEDEGREES_EVIDENCE_COOKIES = 3
} fiftyoneDegreesEvidenceHeaderPrefix;

typedef struct fiftyone_degrees_evidence_key_value_pair {
	fiftyoneDegreesEvidenceHeaderPrefix prefix; // e.g. FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER
	const char *field; // e.g. User-Agent
	const void *originalValue; // The original unparsed value
	const void *parsedValue; // The parsed value which may not be a string
} fiftyoneDegreesEvidenceKeyValuePair;

typedef struct fiftyone_degrees_evidence_collection {
	fiftyoneDegreesEvidenceKeyValuePair *items;
	int count;
	int capacity;
	void*(*malloc)(size_t);
	void(*free)(void*);
} fiftyoneDegreesEvidenceCollection;

/**
 * Called by the iterate method with a pointer to the key value pair in the 
 * collection of evidence. The parsedValue will be set to the parsed data or
 * NULL if it has not been possible to parse the original value.
 */
typedef int (*fiftyoneDegreesEvidenceMatched)(
	void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair);

typedef bool(*fiftyoneDegreesEvidenceCompare)(
	void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair);

EXTERNAL fiftyoneDegreesEvidenceCollection* fiftyoneDegreesEvidenceCreate(
	int capacity, 
	void*(*malloc)(size_t),
	void(*free)(void*));

EXTERNAL void fiftyoneDegreesEvidenceFree(
	fiftyoneDegreesEvidenceCollection *evidence);

/**
 * Adds a new entry to the items collection. The memory associated with the 
 * field and original value parameters must not be freed until after the 
 * evidence collection has been freed. This method will NOT copy the values.
 */
EXTERNAL fiftyoneDegreesEvidenceKeyValuePair* fiftyoneDegreesEvidenceAddString(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceHeaderPrefix prefix,
	const char *field,
	const char *originalValue);

/**
 * Iterators over the items in the evidence that match the header value AND the
 * characters of the field. If field is NULL then all the items that match the 
 * header are returned. Where the parsedValue of the key value pair is NULL 
 * prior to calling this method the parsed value will be calculated.
 * @return the number of items iterated over
 */
EXTERNAL int fiftyoneDegreesEvidenceIterate(
	fiftyoneDegreesEvidenceCollection *evidence,
	void *state,
	fiftyoneDegreesEvidenceCompare compareMethod,
	fiftyoneDegreesEvidenceMatched matchedMethod);

EXTERNAL fiftyoneDegreesEvidenceHeaderPrefix fiftyoneDegreesEvidenceMapPrefix(const char *evidence);

#endif