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

#include "file.h"

static void setLength(fiftyoneDegreesFilePool *reader) {
	fiftyoneDegreesFileHandle *handle;
	reader->length = 0;
	handle = fiftyoneDegreesFileHandleGet(reader);
	assert(handle != NULL);
	if (fseek(handle->file, 0, SEEK_END) == 0) {
        reader->length = ftell(handle->file);
	}
	fiftyoneDegreesFileHandleRelease(handle);
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

fiftyoneDegreesFileOpenStatus fiftyoneDegreesFilePoolInit(
	fiftyoneDegreesFilePool *pool,
	const char *fileName,
	uint16_t concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*)) {
	fiftyoneDegreesFileOpenStatus status =
		FIFTYONEDEGREES_FILE_OPEN_STATUS_FAILED;
	uint16_t i;
	fiftyoneDegreesFileHandle *handle;
	pool->stack = (fiftyoneDegreesFileHandle*)malloc(
		sizeof(fiftyoneDegreesFileHandle) * concurrency);
	if (pool->stack != NULL) {
		pool->count = concurrency;
		pool->head.values.aba = 0;
		pool->head.values.index = 0;
		for (i = 0; i < concurrency; i++) {
			handle = &pool->stack[i];

			// Set a reference back to the reader which can be used when the
			// handle is freed.
			handle->pool = pool;

			// Open the file. If anything other than zero is returned then
			// exit.
			status = fiftyoneDegreesFileOpen(fileName, &handle->file);
			if (status != FIFTYONEDEGREES_FILE_OPEN_STATUS_SUCCESS) {
				fiftyoneDegreesFilePoolRelease(pool, free);
				return status;
			}

			// Link the handle to the next one in the list.
			handle->next = pool->head.values.index;
			pool->head.values.index = i;
		}

		// Set the length of the file in the reader.
		setLength(pool);
	}
	return status;
}

fiftyoneDegreesFileHandle* fiftyoneDegreesFileHandleGet(
	fiftyoneDegreesFilePool *pool) {
	fiftyoneDegreesFileHead orig;
#ifndef FIFTYONEDEGREES_NO_THREADING
	fiftyoneDegreesFileHead next;
	do {
		orig = pool->head;
		next.values.aba = orig.values.aba + 1;
		next.values.index = pool->stack[orig.values.index].next;
	} while (FIFTYONEDEGREES_INTERLOCK_EXCHANGE(
		pool->head.exchange,
		next.exchange,
		orig.exchange) != orig.exchange);
#else
	orig = pool->head;
	pool->head.values.index = pool->stack[orig.values.index].next;
#endif
	return &pool->stack[orig.values.index];
}

void fiftyoneDegreesFileHandleRelease(fiftyoneDegreesFileHandle* handle) {
#ifndef FIFTYONEDEGREES_NO_THREADING
	fiftyoneDegreesFileHead orig, next;
	do {
		orig = handle->pool->head;
		handle->next = orig.values.index;
		next.values.aba = orig.values.aba + 1;
		next.values.index = (uint16_t)(handle - handle->pool->stack);
	} while(FIFTYONEDEGREES_INTERLOCK_EXCHANGE(
		handle->pool->head.exchange,
		next.exchange,
		orig.exchange) != orig.exchange);
#else
	handle->next = handle->pool->head.values.index;
	handle->pool->head.values.index =
		(uint16_t)(handle - handle->pool->stack);
#endif
}

void fiftyoneDegreesFilePoolRelease(
	fiftyoneDegreesFilePool* pool,
	void(*free)(void*)) {
	uint16_t i;

	// Close each of the file handles contained in the reader.
	for (i = 0; i < pool->count; i++) {
		fclose(pool->stack[i].file);
	}

	// Free the memory used by the linked list.
	free(pool->stack);

	// Set the values back to the defaults.
	pool->head.values.aba = 0;
	pool->head.values.index = 0;
	pool->length = 0;
	pool->stack = NULL;
}