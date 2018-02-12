#ifndef FIFTYONEDEGREES_IP_H_INCLUDED
#define FIFTYONEDEGREES_IP_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef unsigned char byte;

typedef enum e_fiftyone_degrees_evidence_ip_type {
	FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV4 = 0,
	FIFTYONEDEGREES_EVIDENCE_IP_TYPE_IPV6 = 1,
	FIFTYONEDEGREES_EVIDENCE_IP_TYPE_INVALID = 2,
} fiftyoneDegreesEvidenceIpType;

typedef struct fiftyone_degrees_evidence_ip_address
    fiftyoneDegreesEvidenceIpAddress;

typedef struct fiftyone_degrees_evidence_ip_address {
	fiftyoneDegreesEvidenceIpType type;
	byte *address; // The first byte of the address
	byte *current; // When building the address the next byte to update
	fiftyoneDegreesEvidenceIpAddress *next; // Next address in the list
	byte bytesPresent; // Number of bytes in the original string which are not abbreviated
					   // const char *originalStart; // The first character for the IP address - TODO add optimisation
					   // const char *originalEnd; // The last character for the IP addresses
} fiftyoneDegreesEvidenceIpAddress;

EXTERNAL void fiftyoneDegreesIpFreeAddresses(
	void(*free)(void*),
	fiftyoneDegreesEvidenceIpAddress *addresses);

EXTERNAL fiftyoneDegreesEvidenceIpAddress* parseIpAddress(
	void*(*malloc)(size_t),
	const char *start,
	const char *end);

EXTERNAL fiftyoneDegreesEvidenceIpAddress* parseIpAddresses(
	void*(*malloc)(size_t),
	const char *start);
#endif
