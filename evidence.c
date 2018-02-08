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
#include "string.h"

static void parsePair(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	switch (pair->prefix) {
	case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES:
		parseIpAddresses(evidence->malloc, pair->originalValue);
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
					(fiftyoneDegreesEvidenceIpAddress*)pair->parsedValue);
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
	if (evidence->count < evidence->capacity) {
		pair = &evidence->items[evidence->count];
		pair->prefix = prefix;
		pair->field = field;
		pair->originalValue = (void*)originalValue;
		pair->parsedValue = NULL;
		evidence->count++;
	}
	return pair;
}

int fiftyoneDegreesEvidenceIterate(
	fiftyoneDegreesEvidenceCollection *evidence,
	void *state,
	fiftyoneDegreesEvidenceCompare compareMethod,
	fiftyoneDegreesEvidenceMatched matchedMethod) {
	int i;
	int count = 0;
	fiftyoneDegreesEvidenceKeyValuePair *pair;
	for (i = 0; i < evidence->count; i++) {
		pair = &evidence->items[i];
		if (compareMethod(state, pair) == true) {
			if (pair->parsedValue == NULL) {
				parsePair(evidence, pair);
			}
			matchedMethod(state, pair);
			count++;
		}
	}
	return count;
}

fiftyoneDegreesEvidenceHeaderPrefix fiftyoneDegreesEvidenceMapPrefix(
	const char *key) {
	if (strncmp("header", key, 6) == 0) {
		// todo check if known IP address header.
		if (strcmp(key, "header.host") == 0) {
			return FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES;
		}
		return FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING;
	}
	if (strncmp("server", key, 6) == 0) {
		return FIFTYONEDEGREES_EVIDENCE_SERVER;
	}
	if (strncmp("cookie", key, 6) == 0) {
		return FIFTYONEDEGREES_EVIDENCE_COOKIES;
	}
}