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

#include "evidence.h"

typedef int(*parseIterator)(
	void *state,
	void *segment);

/**
 * State is an integer which is increased everytime the method is called.
 */
static int callbackIpAddressCount(void *state, void *segment) {
	*((int*)state)++;
	return 0;
}

static int callbackIpAddressBuild(void *state, void *segment) {
	// TODO - Fix to add to the array. Need another field / structure.
	*((byte*)state) = *(byte*)segment;
}

/**
 * Calls the callback method every time a byte is identified in the value
 * when parsed left to right.
 */
static void* iterateIpAddress(const char *value, void *state, parseIterator callback) {
	// foreach byte call the callback.
}

static void* parseIpAddress(
	fiftyoneDegreesEvidenceCollection *evidence, 
	const char *value) {
	int count = 0;
	iterateIpAddress(value, &count, callbackIpAddressCount);
	unsigned char* result = evidence->malloc(count * sizeof(byte));
	iterateIpAddress(value, result, callbackIpAddressBuild);
	return result;
}

static void parseIpAddresses(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	int count = 1;
	// TODO - Fix to handle multiple IP addresses.
	fiftyoneDegreesEvidenceIpAddresses *addresses = evidence->malloc(
		sizeof(fiftyoneDegreesEvidenceIpAddresses) +
		(count * sizeof(fiftyoneDegreesEvidenceIpAddress)));
	if (addresses != NULL) {

	}
	return addresses;
}

static void parsePair(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	switch (pair->prefix) {
	case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES:
		parseIpAddresses(evidence, pair);
		break;
	case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING:
	case FIFTYONEDEGREES_EVIDENCE_SERVER:
	case FIFTYONEDEGREES_EVIDENCE_COOKIES:
	default:
		pair->parsedValue = pair->originalValue;
		break;
	}
}

fiftyoneDegreesEvidenceCollection* fiftyoneDegreesEvidenceCreate(
	int capacity,
	void*(*malloc)(size_t),
	void(*free)(void*)) {
	fiftyoneDegreesEvidenceCollection *evidence = malloc(
		sizeof(fiftyoneDegreesEvidenceCollection) +
		(capacity * sizeof(fiftyoneDegreesEvidenceKeyValuePair)));
	if (evidence != NULL) {
		evidence->capacity = capacity;
		evidence->count = 0;
		evidence->malloc = malloc;
		evidence->free = free;
		evidence->items = (fiftyoneDegreesEvidenceKeyValuePair*)(evidence + 1);
	}
	return evidence;
}

static void freeIpAddresses(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceIpAddresses *addresses) {
	int i;
	for (i = 0; i < addresses->count; i++) {
		evidence->free(addresses->items[i].address);
	}
}

void fiftyoneDegreesEvidenceFree(
	fiftyoneDegreesEvidenceCollection *evidence) {
	int i;
	fiftyoneDegreesEvidenceKeyValuePair *pair;
	for (i = 0; i < evidence->count; i++) {
		pair = &evidence->items[i];
		if (pair->parsedValue != NULL) {
			switch (pair->prefix) {
			case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES:
				// The parsed evidence is a collection of IP addresses. 
				// Free the memory for each IP address.
				freeIpAddresses(
					evidence, 
					(fiftyoneDegreesEvidenceIpAddresses*)pair->parsedValue);
				break;
			case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING:
			case FIFTYONEDEGREES_EVIDENCE_SERVER:
			case FIFTYONEDEGREES_EVIDENCE_COOKIES:
			default:
				// Free the parsed value if not the same as the original.
				if (pair->parsedValue != pair->originalValue) {
					evidence->free(pair->parsedValue);
				}
				break;
			}
		}
	}
	evidence->free(evidence);
}

fiftyoneDegreesEvidenceKeyValuePair* fiftyoneDegreesEvidenceAddString(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceHeaderPrefix prefix,
	const char *field,
	const char *originalValue) {
	fiftyoneDegreesEvidenceKeyValuePair *pair = NULL;
	if (evidence->count + 1 < evidence->capacity) {
		pair = &evidence->items[evidence->count];
		pair->prefix = prefix;
		pair->field = field;
		pair->originalValue = (void*)originalValue;
		pair->parsedValue = NULL;
	}
	return pair;
}

int fiftyoneDegreesEvidenceIterate(
	fiftyoneDegreesEvidenceCollection *evidence,
	void *state,
	fiftyoneDegreesEvidenceHeaderPrefix prefix,
	const char *field,
	fiftyoneDegreesEvidenceIterator callback) {
	int i;
	int count = 0;
	fiftyoneDegreesEvidenceKeyValuePair *pair;
	for (i = 0; i < evidence->count; i++) {
		pair = &evidence->items[i];
		if (pair->prefix == prefix &&
			strcmp(pair->field, field)) {
			if (pair->parsedValue == NULL) {
				parsePair(evidence, pair);
			}
			callback(state, pair);
			count++;
		}
	}
	return count;
}

fiftyoneDegreesEvidenceHeaderPrefix fiftyoneDegreesEvidenceMapPrefix(
	const char *prefix) {
	if (strcmp("header", prefix)) {
		return FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING;
	}
}