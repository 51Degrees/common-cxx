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
	assert(fileOffset < file->offset + (long)file->collection->size);

	data->length = 0;

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
	data->length = length;

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