#pragma once
#include "pch.h"
extern "C" {
#include "../collection.h"
#include "../string.h"
}

typedef struct fiftyone_degrees_test_collection_state {
	fiftyoneDegreesCollection *collection;
	uint32_t *offsets;
	void *data;
} fiftyoneDegreesTestCollectionState;

// Function used to return string names when the collections code 
// requests them
fiftyoneDegreesString* getStringTestValue(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item);

class StringCollectionTestBase : public ::testing::Test {
protected:
	fiftyoneDegreesTestCollectionState state;
	int count;

	void freeState(fiftyoneDegreesTestCollectionState *state);
	fiftyoneDegreesTestCollectionState buildState(const char **values, int count);
};
