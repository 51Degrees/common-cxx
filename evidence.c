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
static int callbackIpAddressCount(void *state, fiftyoneDegreesEvidenceIpType type, const char *start) {
	if (type = FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID) {
		*((int*)state)++;
		if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
			*((int*)state)++;
		}
	}
}

static byte parseIpV6Byte(const char *start) {
	// TODO - implement
}

static int callbackIpAddressBuild(void *state, fiftyoneDegreesEvidenceIpType type, const char *start) {
	fiftyoneDegreesEvidenceIpAddress *address = (fiftyoneDegreesEvidenceIpAddress*)state;
	if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4) {
		*address->next = atoi(start);
		address->next++;
	}
	else if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
		*address->next = parseIpV6Byte(start);
		address->next++;
		*address->next = parseIpV6Byte(start + 2);
		address->next++;
	}
}

static fiftyoneDegreesEvidenceIpType getIpTypeFromSeparator(char separator) {
	switch (separator) {
	case '.':
		return FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4;
	case ':':
		return FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6;
	default:
		return FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID;
	}
}

/**
 * Calls the callback method every time a byte is identified in the value
 * when parsed left to right.
 */
static fiftyoneDegreesEvidenceIpType iterateIpAddress(
	const char *start,
	const char *end, 
	void *state, 
	fiftyoneDegreesEvidenceIpType type,
	parseIterator foundByte) {
	const char *current = start;
	const char *nextByte = current;
	while (current != NULL) {
		if (*current == ',' ||
			*current == ':' ||
			*current == '.') {
			if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID) {
				type = getIpTypeFromSeparator(*current);
			}
			foundByte(state, type, nextByte);
			nextByte = current++;

		}
		current++;
		// TODO - work out when a new full byte has been found and call foundByte. BEN
		foundByte(state, current); // THIS IS WRONG AND NEEDS CHANGING. BEN
	}
	return type;
}

static fiftyoneDegreesEvidenceIpAddress* parseIpAddress(
	fiftyoneDegreesEvidenceCollection *evidence, 
	const char *start,
	const char *end,
	fiftyoneDegreesEvidenceIpType type) {
	int count;
	fiftyoneDegreesEvidenceIpAddress *address;
	fiftyoneDegreesEvidenceIpType = iterateIpAddress(
		start,
		end,
		&count,
		FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID,
		callbackIpAddressCount);
	address = evidence->malloc(
		sizeof(fiftyoneDegreesEvidenceIpAddress) +
		(count * sizeof(byte)));
	if (address != NULL) {
		// Set the address of the byte array to the byte following the
		// IpAddress structure. The previous malloc included the necessary
		// space to make this available.
		address->address = (byte*)(address + 1);
		// Set the next byte to be added during the parse operation.
		address->next = (byte*)(address + 1);
		// Add the bytes from the source value and get the type of address.
		address->type = iterateIpAddress(
			start,
			end,
			address,
			type,
			callbackIpAddressBuild);
	}
	return address;
}

static fiftyoneDegreesEvidenceIpAddress* parseIpAddresses(
	fiftyoneDegreesEvidenceCollection *evidence,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	const char *start = pair->originalValue;
	const char *current = start;
	fiftyoneDegreesEvidenceIpAddress *head = NULL;
	fiftyoneDegreesEvidenceIpAddress *last = NULL;
	fiftyoneDegreesEvidenceIpAddress *item = NULL;
	fiftyoneDegreesEvidenceIpType type = FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID;
	while (current != NULL) {
		if (current == ' ') {
			// We have reached the end of a probable IP address.
			item = parseIpAddress(evidence, start, current - 1, type);
			if (item != NULL) {
				if (last == NULL && head == NULL) {
					// Add the first item to the list.
					head = item;
					last = item;
				}
				else {
					// Add the new item to the end of the list.
					last->next = item;
					last = item;
				}
				item->next = NULL;
			}
			start = current;
		}
		current++;
	}
	return head;
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
	const char *prefix) {
	if (strcmp("header", prefix) == 0) {
		// todo check if known IP address header.
		return FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING;
	}
	if (strcmp("server", prefix) == 0) {
		return FIFTYONEDEGREES_EVIDENCE_SERVER;
	}
	if (strcmp("cookie", prefix) == 0) {
		return FIFTYONEDEGREES_EVIDENCE_COOKIES;
	}
}