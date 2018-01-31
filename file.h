#include <stdio.h>
#include "data.h"

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

#ifndef FIFTYONEDEGREES_FILE_H_INCLUDED
#define FIFTYONEDEGREES_FILE_H_INCLUDED

typedef enum e_fiftyoneDegreesFileOpenStatus {
	FIFTYONEDEGREES_FILE_OPEN_STATUS_SUCCESS, /* All okay */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_FILE_NOT_FOUND, /* The data file couldn't
													 be found */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_TOO_MANY_OPEN_FILES, /* Too many files are 
														 open */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_FAILED /* General failure */
} fiftyoneDegreesFileOpenStatus;

typedef struct fiftyonedegrees_file_handle_t fiftyoneDegreesFileHandle;
typedef struct fiftyonedegrees_file_reader_t fiftyoneDegreesFileReader;

/**
 * File handle node in the linked list of handles.
 */
typedef struct fiftyonedegrees_file_handle_t {
	FILE *file; /* Open read handle to the source data file. */
	fiftyoneDegreesFileHandle *next; /* Next handle in the linked list. */
	fiftyoneDegreesFileReader *reader; /* Reader the handle belongs to. */
} fiftyoneDegreesFileHandle;

/**
 * Linked list of handles used to read data from a single source file.
 */
typedef struct fiftyonedegrees_file_reader_t {
	fiftyoneDegreesFileHandle *linkedList; /* Pointer to the memory used by 
										      the linked list. */
	fiftyoneDegreesFileHandle *head; /* Current head of the handles linked 
									    list */
	long length; /* Length of the file in bytes. */
} fiftyoneDegreesFileReader;

void fiftyoneDegreesFileReaderFree(fiftyoneDegreesFileReader* reader, void(*free)(void*));

fiftyoneDegreesFileOpenStatus fiftyoneDegreesFileReaderInit(
	fiftyoneDegreesFileReader *reader,
	const char *fileName,
	int concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*),
	fiftyoneDegreesFileOpenStatus(*open)(const char* fileName, FILE** handle));

fiftyoneDegreesFileHandle* fiftyoneDegreesFileHandleGet(
	fiftyoneDegreesFileReader *reader);

void fiftyoneDegreesFileHandleRelease(
	fiftyoneDegreesFileHandle* handle);

#endif