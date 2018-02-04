#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "file.h"

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

#ifndef FIFTYONEDEGREES_NO_THREADING
#include "threading.h"
#endif

static void setLength(fiftyoneDegreesFileReader *reader) {
	fiftyoneDegreesFileHandle *handle;
	reader->length = 0;
	handle = fiftyoneDegreesFileHandleGet(reader);
	if (handle != NULL) {
		if (fseek(handle->file, 0, SEEK_END) == 0) {
            reader->length = ftell(handle->file);
		}
		fiftyoneDegreesFileHandleRelease(handle);
	}
}

void fiftyoneDegreesFileReaderFree(
	fiftyoneDegreesFileReader* reader,
	void(*free)(void*)) {

	// Close each of the file handles contained in the reader.
	fiftyoneDegreesFileHandle *current = (fiftyoneDegreesFileHandle*)reader->head;
	while (current != NULL) {
		fclose(current->file);
		current = (fiftyoneDegreesFileHandle*)current->next;
	}

	// Free the memory used by the linked list.
	free(reader->linkedList);

	// Set the values back to the defaults.
	reader->head = NULL;
	reader->length = 0;
	reader->linkedList = NULL;
}

fiftyoneDegreesFileOpenStatus fiftyoneDegreesFileOpen(
	const char* fileName,
	FILE** handle) {
	// Open the file and hold on to the data.ptr.
#ifndef _MSC_FULL_VER
	*handle = fopen(fileName, "rb");
#else
	/* If using Microsoft use the fopen_s method to avoid warning */
	errno_t error = fopen_s(handle, fileName, "rb");
	if (error != 0) {
		switch (error) {
		case ENFILE:
		case EMFILE:
			return FIFTYONEDEGREES_FILE_OPEN_STATUS_TOO_MANY_OPEN_FILES;
		case ENOENT:
		default:
			return FIFTYONEDEGREES_FILE_OPEN_STATUS_FILE_NOT_FOUND;
		}
	}
#endif
	return FIFTYONEDEGREES_FILE_OPEN_STATUS_SUCCESS;
}

fiftyoneDegreesFileOpenStatus fiftyoneDegreesFileReaderInit(
	fiftyoneDegreesFileReader *reader,
	const char *fileName,
	int concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*)) {
	fiftyoneDegreesFileOpenStatus status = 
		FIFTYONEDEGREES_FILE_OPEN_STATUS_FAILED;
	int i;
	fiftyoneDegreesFileHandle *handle;
	reader->linkedList = (fiftyoneDegreesFileHandle*)malloc(
		sizeof(fiftyoneDegreesFileHandle) * concurrency);
	if (reader->linkedList != NULL) {
		reader->head = NULL;
		for (i = 0; i < concurrency; i++) {
			handle = &reader->linkedList[i];

			// Set a reference back to the reader which can be used when the
			// handle is freed.
			handle->reader = reader;

			// Open the file. If anything other than zero is returned then
			// exit.
			status = fiftyoneDegreesFileOpen(fileName, &handle->file);
			if (status != FIFTYONEDEGREES_FILE_OPEN_STATUS_SUCCESS) {
				fiftyoneDegreesFileReaderFree(reader, free);
				return status;
			}

			// Add the handle to the head of the linked list.
			handle->next = reader->head;
			reader->head = handle;
		}

		// Set the length of the file in the reader.
		setLength(reader);
	}
	return status;
}

fiftyoneDegreesFileHandle* fiftyoneDegreesFileHandleGet(
	fiftyoneDegreesFileReader *reader) {
	fiftyoneDegreesFileHandle *first = reader->head;
	assert(reader->head != NULL);
#ifndef FIFTYONEDEGREES_NO_THREADING
	fiftyoneDegreesFileHandle *next;
	do {
		next = first;
		first = (fiftyoneDegreesFileHandle*)
			FIFTYONEDEGREES_INTERLOCK_EXCHANGE_POINTER(
				reader->head,
				first->next,
				first);
	} while (first != next);
#else
	reader->head = first->next;
#endif
	return first;
}

void fiftyoneDegreesFileHandleRelease(fiftyoneDegreesFileHandle* handle) {
	fiftyoneDegreesFileReader *reader = handle->reader;
#ifndef FIFTYONEDEGREES_NO_THREADING
	fiftyoneDegreesFileHandle *first = reader->head;
	fiftyoneDegreesFileHandle *next;
	do {
		handle->next = first;
		next = first;
		first = (fiftyoneDegreesFileHandle*)
			FIFTYONEDEGREES_INTERLOCK_EXCHANGE_POINTER(
				reader->head,
				handle,
				first);
	} while (first != next);
#else
	handle->next = reader->head;
	reader->head = handle;
#endif
}
