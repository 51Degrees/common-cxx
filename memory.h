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

#ifndef FIFTYONEDEGREES_MEMORY_H_INCLUDED
#define FIFTYONEDEGREES_MEMORY_H_INCLUDED

#include <stdlib.h>
#include "data.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

#ifdef _MSC_VER
#define FIFTYONEDEGREES_CALL_CONV __cdecl
#else
#define FIFTYONEDEGREES_CALL_CONV
#endif

/**
* Used to read data from memory in a similar manner to a file handle.
*/
typedef struct fiftyoneDegrees_memory_reader_t {
	byte *current; // The current byte being read from
	byte *lastByte; // The maximum byte that can be read from
	long length; // Length of the file in bytes
} fiftyoneDegreesMemoryReader;

int fiftyoneDegreesMemoryAdvance(
	fiftyoneDegreesMemoryReader *reader,
	size_t advanceBy);

EXTERNAL void *(FIFTYONEDEGREES_CALL_CONV *fiftyoneDegreesMalloc)(size_t __size);

EXTERNAL void (FIFTYONEDEGREES_CALL_CONV *fiftyoneDegreesFree)(void *__ptr);

#endif
