#include "ip.h"

typedef int(*parseIterator)(
	void *state,
	fiftyoneDegreesEvidenceIpType type,
	const char *start,
	const char *end);

/**
* State is an integer which is increased everytime the method is called.
*/
static int callbackIpAddressCount(void *state, fiftyoneDegreesEvidenceIpType type, const char *start, const char *end) {
	if (start < end) {
		if (type != FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID) {
			(*(int*)state)++;
			if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
				(*(int*)state)++;
			}
		}
	}
}

static void parseIpV6Segment(
	fiftyoneDegreesEvidenceIpAddress *address,
	const char *start,
	const char *end) {
	int i;
	char first[3], second[3], val;
	if (start > end) {
		// This is an abbreviation, so fill it in.
		for (i = 0; i < 16 - address->bytesPresent; i++) {
			*address->current = (byte)0;
			address->current++;
		}
	}
	else {
		// Add the two bytes of the segment.
		first[2] = '\0';
		second[2] = '\0';
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
}

static int callbackIpAddressBuild(void *state, fiftyoneDegreesEvidenceIpType type, const char *start, const char *end) {
	fiftyoneDegreesEvidenceIpAddress *address = (fiftyoneDegreesEvidenceIpAddress*)state;
	if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4) {
		*address->current = atoi(start);
		address->current++;
	}
	else if (type == FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6) {
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
	while (current <= end && nextSegment < end) {
		if (*current == ',' ||
			*current == ':' ||
			*current == '.' ||
			*current == ' ' ||
			*current == '\0') {
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

fiftyoneDegreesEvidenceIpAddress* mallocIpAddress(
	void*(*malloc)(size_t),
	fiftyoneDegreesEvidenceIpType type) {
	switch (type) {
	case FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4:
		return malloc(
			sizeof(fiftyoneDegreesEvidenceIpAddress) +
			(4 * sizeof(byte)));
	case FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6:
	default:
		return malloc(
			sizeof(fiftyoneDegreesEvidenceIpAddress) +
			(16 * sizeof(byte)));
	}
}

void fiftyoneDegreesIpFreeAddresses(
	void(*free)(void*),
	fiftyoneDegreesEvidenceIpAddress *addresses) {
	fiftyoneDegreesEvidenceIpAddress *current = addresses;
	fiftyoneDegreesEvidenceIpAddress *prev;
	while (current != NULL) {
		prev = current;
		current = current->next;
		free(prev->address);
	}
}

fiftyoneDegreesEvidenceIpAddress* parseIpAddress(
	void*(*malloc)(size_t),
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

	address = mallocIpAddress(malloc, type);
	if (address != NULL) {
		// Set the address of the byte array to the byte following the
		// IpAddress structure. The previous malloc included the necessary
		// space to make this available.
		address->address = (byte*)(address + 1);
		// Set the next byte to be added during the parse operation.
		address->current = (byte*)(address + 1);
		address->bytesPresent = (byte)count;
		// Add the bytes from the source value and get the type of address.
		iterateIpAddress(
			start,
			end,
			address,
			type,
			callbackIpAddressBuild);
		address->next = NULL;
	}
	return address;
}

fiftyoneDegreesEvidenceIpAddress* parseIpAddresses(
	void*(*malloc)(size_t),
	const char *start) {
	const char *current = start;
	fiftyoneDegreesEvidenceIpAddress *head = NULL;
	fiftyoneDegreesEvidenceIpAddress *last = NULL;
	fiftyoneDegreesEvidenceIpAddress *item = NULL;
	while (*current != '\0') {
		current++;
		if (*current == ' ' ||
		    *current == ',' ||
		    *current == '\0') {
			// We have reached the end of a probable IP address.
			item = parseIpAddress(malloc, start, current);
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
			start = current + 1;
		}
	}
	return head;
}
