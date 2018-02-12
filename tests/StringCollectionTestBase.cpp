#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
	#include "../collection.h"
	#include "../string.h"
}

// Function used to return string names when the collections code 
// requests them
fiftyoneDegreesString* getStringTestValue(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item)
{
	fiftyoneDegreesTestCollectionState *strings = (fiftyoneDegreesTestCollectionState*)state;
	fiftyoneDegreesString *result = NULL;
	strings->collection->get(strings->collection, strings->offsets[index], item);
	item->collection = strings->collection;
	return (fiftyoneDegreesString*)item->data.ptr;
}

// Class that sets up the properties structure. This stops us having to 
// do it multiple times.
void StringCollectionTestBase::freeState(fiftyoneDegreesTestCollectionState *state) {
	free(state->offsets);
	free(state->data);
}

fiftyoneDegreesTestCollectionState StringCollectionTestBase::buildState(const char **values, int count) {
	fiftyoneDegreesTestCollectionState state;
	int i;
	fiftyoneDegreesMemoryReader reader;
	size_t dataLength = 0;
	for (i = 0; i < count; i++) {
		dataLength += 2 + strlen(values[i]) + 1;
	}
	reader.length = dataLength + sizeof(uint32_t);
	state.data = malloc(reader.length);
	*(int32_t*)state.data = dataLength;
	state.offsets = (uint32_t*)malloc(count * sizeof(uint32_t));
	byte *start = ((byte*)state.data) + sizeof(uint32_t);
	reader.current = start;
	for (i = 0; i < count; i++) {
		fiftyoneDegreesString *string = (fiftyoneDegreesString*)reader.current;
		string->size = (int16_t)strlen(values[i]) + 1;
		strncpy(&string->value, values[i], string->size);
		state.offsets[i] = reader.current - start;
		reader.current += 2 + string->size;
	}
	reader.lastByte = reader.current;
	reader.current = (byte*)state.data;
	state.collection = fiftyoneDegreesCollectionCreateFromMemory(
		&reader,
		0,
		0,
		malloc);
	assert(state.collection->size == dataLength);
	return state;
}
