#include <stdint.h>

#ifndef FIFTYONEDEGREES_STRING_H_INCLUDED
#define FIFTYONEDEGREES_STRING_H_INCLUDED

#define FIFTYONEDEGREES_STRING(s) &((fiftyoneDegreesString*)s)->value

#pragma pack(push, 2)
typedef struct fiftyoneDegrees_string_t {
	int16_t size;
	char value;
} fiftyoneDegreesString;
#pragma pack(pop)

#endif