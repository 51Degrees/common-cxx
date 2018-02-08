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

typedef int(*parseIterator)(
	void *state,
	fiftyoneDegreesEvidenceIpType type,
	const char *start,
	const char *end);

/**
 * State is an integer which is increased everytime the method is called.
 */
static int callbackIpAddressCount(void *state, fiftyoneDegreesEvidenceIpType type, const char *start, const char *end) {
	if (type != FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID) {
		(*(int*)state)++;
		if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
			(*(int*)state)++;
		}
	}
}

//static byte parseIpV6Byte(const char *start) {
//	int i;
//	char hexChars[3];
//	hexChars[2] = '\0';
//	for (i = 0; i < 2; i++) {
//		hexChars[i] = start[i];
//	}
//	return (byte)strtol(hexChars, NULL, 16);
//}
static void parseIpV6Segment(
	fiftyoneDegreesEvidenceIpAddress *address,
	const char *start,
	const char *end) {
	int i;
	char first[3], second[3];
	first[2] = '\0';
	second[2] = '\0';
	char val;
	for (i = 0; i < 4; i++) {
		if (end - i >= start) val = end[-i];
		else val = '0';

		if (i < 2) second[1 - i] = val;
		else first[3 - i] = val;
	}
	*address->current = (byte)strtol(first, NULL, 16);
	address->current++;
	*address->current = (byte)strtol(second, NULL, 16);
	address->current++;

}

static int callbackIpAddressBuild(void *state, fiftyoneDegreesEvidenceIpType type, const char *start, const char *end) {
	fiftyoneDegreesEvidenceIpAddress *address = (fiftyoneDegreesEvidenceIpAddress*)state;
	if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4) {
		*address->current = atoi(start);
		address->current++;
	}
	else if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
		/*
		*address->current = parseIpV6Byte(start);
		address->current++;
		*address->current = parseIpV6Byte(start + 2);
		address->current++;
		*/
		parseIpV6Segment(address, start, end);
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
	parseIterator foundSegment) {
	const char *current = start;
	const char *nextSegment = current;
	while (current <= end) {
		if (*current == ',' ||
			*current == ':' ||
			*current == '.' ||
			*current == ' ' ||
			*current == NULL) {
			if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID) {
				type = getIpTypeFromSeparator(*current);
			}
			foundSegment(state, type, nextSegment, current - 1);
			nextSegment = current + 1;
		}
		current++;
	}
	return type;
}

fiftyoneDegreesEvidenceIpAddress* parseIpAddress(
	fiftyoneDegreesEvidenceCollection *evidence, 
	const char *start,
	const char *end) {
	int count = 0;
	fiftyoneDegreesEvidenceIpAddress *address;
	fiftyoneDegreesEvidenceIpType type = iterateIpAddress(
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
		address->current = (byte*)(address + 1);
		// Add the bytes from the source value and get the type of address.
		address->type = iterateIpAddress(
			start,
			end,
			address,
			type,
			callbackIpAddressBuild);
		address->next = NULL;
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
	while (current != NULL) {
		if (current == ' ') {
			// We have reached the end of a probable IP address.
			item = parseIpAddress(evidence, start, current - 1);
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
	case FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING:
		if (strcmp("host", pair->field) == 0) {
			parseIpAddresses(evidence, pair);
		}
		else {
			pair->parsedValue = pair->originalValue;
		}
		break;
	case FIFTYONEDEGREES_EVIDENCE_SERVER:
		if (strcmp("client-ip", pair->field) == 0) {
			parseIpAddresses(evidence, pair);
		}
		else {
			pair->parsedValue = pair->originalValue;
		}
		break;
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
	fiftyoneDegreesEvidenceIpAddress *addresses) {
	fiftyoneDegreesEvidenceIpAddress *current = addresses;
	fiftyoneDegreesEvidenceIpAddress *prev;
	while (current != NULL) {
		prev = current;
		current = current->next;
		evidence->free(prev->address);
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
		for (int i = 0; i < knownIpHeadersCount; i++) {
			if (strcmp(key, "header.host") == 0) {
				return FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_IP_ADDRESSES;
			}
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