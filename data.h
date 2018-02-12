#ifndef FIFTYONEDEGREES_DATA_H_INCLUDED
#define FIFTYONEDEGREES_DATA_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef unsigned char byte;

typedef struct fiftyoneDegrees_data_t {
	byte *ptr; /* Pointer to immutable data */
	uint32_t allocated; /* Number of bytes allocated at the pointer */
	uint32_t used; /* Size in bytes of the data at the pointer */
} fiftyoneDegreesData;


EXTERNAL void fiftyoneDegreesDataReset(fiftyoneDegreesData *data);

void* fiftyoneDegreesDataMalloc(
	fiftyoneDegreesData *data,
	size_t bytesNeeded,
	void*(*malloc)(size_t),
	void(*free)(void*));

#endif
