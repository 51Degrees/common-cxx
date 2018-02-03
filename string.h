#ifndef FIFTYONEDEGREES_STRING_H_INCLUDED
#define FIFTYONEDEGREES_STRING_H_INCLUDED

#include <stdint.h>
#include "collection.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

#define FIFTYONEDEGREES_STRING(s) &((fiftyoneDegreesString*)s)->value

#pragma pack(push, 2)
typedef struct fiftyoneDegrees_string_t {
	int16_t size;
	char value;
} fiftyoneDegreesString;
#pragma pack(pop)

void* fiftyoneDegreesStringRead(
	const fiftyoneDegreesCollectionFile *file,
	fiftyoneDegreesData *data,
	uint32_t stringOffset);

EXTERNAL fiftyoneDegreesString* fiftyoneDegreesStringGet(
	fiftyoneDegreesCollection *collection,
	uint32_t offset,
	fiftyoneDegreesCollectionItem *item);

#endif