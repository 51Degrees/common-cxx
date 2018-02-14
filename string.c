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

#include "string.h"

/**
* Reads a string from the source file at the offset within the string
* structure.
*/
void* fiftyoneDegreesStringRead(
	const fiftyoneDegreesCollectionFile *file,
	fiftyoneDegreesData *data,
	uint32_t stringOffset) {
	int16_t length = 0, bytesNeeded;
	long fileOffset = file->offset + stringOffset;
	fiftyoneDegreesFileHandle *handle = fiftyoneDegreesFileHandleGet(
		file->reader);

	assert(fileOffset >= file->offset);
	assert(fileOffset < file->offset + (long)file->collection->count);

	data->used = 0;

	// Move to the start of the string in the file.
	if (fseek(
		handle->file,
		fileOffset,
		SEEK_SET) != 0) {
		fiftyoneDegreesFileHandleRelease(handle);
		return NULL;
	}

	// Get the number of bytes in the string including the null terminator.
	// This value is stored as the first two bytes after the offset.
	if (fread(&length, sizeof(int16_t), 1, handle->file) != 1) {
		fiftyoneDegreesFileHandleRelease(handle);
		return NULL;
	}

	// Force the allocation of more memory if the node does not contain
	// sufficient bytes by freeing the existing memory that is too small.
	bytesNeeded = length + sizeof(int16_t);

	// Ensure sufficient memory is allocated for the string being read.
	if (fiftyoneDegreesDataMalloc(
		data,
		bytesNeeded,
		file->malloc,
		file->collection->freeMemory) == NULL) {
		fiftyoneDegreesFileHandleRelease(handle);
		return NULL;
	}

	// Write the number of bytes the string contains in the first two bytes.
	*(int16_t*)data->ptr = length;

	// Read the characters and the final null into the node's data field.
	if (fread((byte*)data->ptr + 2, length, 1, handle->file) != 1) {
		fiftyoneDegreesFileHandleRelease(handle);
		return NULL;
	}
	data->used = bytesNeeded;

	fiftyoneDegreesFileHandleRelease(handle);

	return data->ptr;
}

fiftyoneDegreesString* fiftyoneDegreesStringGet(
	fiftyoneDegreesCollection *collection,
	uint32_t offset,
	fiftyoneDegreesCollectionItem *item) {
	return (fiftyoneDegreesString*)collection->get(
		collection, 
		offset, 
		item)->data.ptr;
}