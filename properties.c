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

#include "properties.h"

fiftyoneDegreesPropertiesRequired fiftyoneDegreesPropertiesDefault = {
	NULL, // No array of properties
	0, // No required properties
	NULL, // No string with properties
	NULL // No list
};

typedef struct properties_source_t {
	uint32_t count; /* Number of properties available in the source */
	void *state; /* State for the get method. Usually a data set */
	fiftyoneDegreesPropertiesGet get; /* Gets a property as a string from the
									  source */
	void*(*malloc)(size_t); /* Allocates memory for the results */
	void(*free)(void*); /* Frees memory */
} propertiesSource;

typedef void(*matchedPropertyMethod)(
	fiftyoneDegreesPropertiesResults*, uint32_t, uint32_t);

static fiftyoneDegreesPropertiesResults* initRequiredPropertiesMemory(
	uint32_t count,
	propertiesSource *source) {
	fiftyoneDegreesPropertiesResults *results = source->malloc(
		sizeof(fiftyoneDegreesPropertiesResults) +
		(count * sizeof(uint32_t)) +
		(count * sizeof(fiftyoneDegreesCollectionItem)));
	if (results != NULL) {
		results->count = count;
		results->indexes = (uint32_t*)(results + 1);
		results->names = (fiftyoneDegreesCollectionItem*)
			(results->indexes + count);
		results->free = source->free;
	}
	return results;
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100)
#endif
static void increaseRequiredPropertiesCount(
	fiftyoneDegreesPropertiesResults *results,
	uint32_t propertyIndex,
	uint32_t requiredPropertyIndex) {
	results->count++;
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

static void addRequiredProperty(
	fiftyoneDegreesPropertiesResults *results,
	uint32_t propertyIndex,
	uint32_t requiredPropertyIndex) {
	results->indexes[requiredPropertyIndex] = propertyIndex;
}

static int getPropertyIndex(
	propertiesSource *source,
	const char *requiredPropertyName,
	int requiredPropertyLength) {
	uint32_t i = 0;
	fiftyoneDegreesString *test;
	fiftyoneDegreesCollectionItem string;
	fiftyoneDegreesDataReset(&string.data);
	for (i = 0; i < source->count; i++) {
		test = source->get(source->state, i, &string);
		// Size includes the NULL terminator. Ignore this character.
		if (test->size - 1 == requiredPropertyLength &&
			strncmp(FIFTYONEDEGREES_STRING(test),
					requiredPropertyName,
					requiredPropertyLength) == 0) {
			string.collection->release(&string);
			return i;
		}
		string.collection->release(&string);
	}
	return -1;
}

static void iteratePropertiesFromExisting(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *results,
	fiftyoneDegreesPropertiesResults *existing,
	matchedPropertyMethod match) {
	uint32_t i;
	fiftyoneDegreesString *propertyName;
	uint32_t propertyIndex, requiredIndex = 0;
	for (i = 0; i < existing->count; i++) {
		propertyName = (fiftyoneDegreesString*)existing->names[i].data.ptr;
		propertyIndex = getPropertyIndex(
			source,
			FIFTYONEDEGREES_STRING(propertyName),
			propertyName->size - 1);
		if (propertyIndex >= 0) {
			match(results, propertyIndex, requiredIndex++);
		}
	}
}

static void iteratePropertiesFromString(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *results,
	const char* properties,
	matchedPropertyMethod match) {
	int propertyIndex, requiredIndex = 0;
	char *property = (char*)properties;
	const char *end = properties - 1;
	do {
		end++;
		if (*end == '|' || *end == ',' || *end == '\0') {
			// If the property name is one that is valid in the data set then
			// use the callback matchedProperty to provide the index.
			propertyIndex = getPropertyIndex(
				source,
				property,
				(int)(end - property));
			if (propertyIndex >= 0) {
				match(results, propertyIndex, requiredIndex++);
			}
			property = (char*)end + 1;
		}
	} while (*end != '\0');
}

static void iteratePropertiesFromArray(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *results,
	const char** properties,
	int count,
	matchedPropertyMethod match) {
	int i, propertyIndex, requiredIndex = 0;
	for (i = 0; i < count; i++) {
		propertyIndex = getPropertyIndex(
			source,
			properties[i],
			(int)strlen(properties[i]));
		if (propertyIndex >= 0) {
			match(results, propertyIndex, requiredIndex++);
		}
	}
}

static uint32_t countPropertiesFromString(
	propertiesSource *source,
	const char *properties) {
	fiftyoneDegreesPropertiesResults counter;
	counter.count = 0;
	iteratePropertiesFromString(
		source,
		&counter,
		properties,
		increaseRequiredPropertiesCount);
	return counter.count;
}

static uint32_t countPropertiesFromArray(
	propertiesSource *source,
	const char **properties,
	int count) {
	fiftyoneDegreesPropertiesResults counter;
	counter.count = 0;
	iteratePropertiesFromArray(
		source,
		&counter,
		properties,
		count,
		increaseRequiredPropertiesCount);
	return counter.count;
}

static uint32_t countPropertiesFromExisting(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *properties) {
	fiftyoneDegreesPropertiesResults counter;
	counter.count = 0;
	iteratePropertiesFromExisting(
		source,
		&counter,
		properties,
		increaseRequiredPropertiesCount);
	return counter.count;
}

static fiftyoneDegreesPropertiesResults* initRequiredPropertiesFromString(
	propertiesSource *source,
	const char* properties) {
	fiftyoneDegreesPropertiesResults *results;
	uint32_t count = countPropertiesFromString(source, properties);
	if (count == 0) {
		return NULL;
	}
	results = initRequiredPropertiesMemory(count, source);
	if (results != NULL) {
		iteratePropertiesFromString(
			source,
			results,
			properties,
			addRequiredProperty);
	}
	return results;
}


static fiftyoneDegreesPropertiesResults* initSpecificPropertiesFromExisting(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *properties) {
	uint32_t count = countPropertiesFromExisting(source, properties);
	fiftyoneDegreesPropertiesResults *results =
		initRequiredPropertiesMemory(count, source);
	if (results != NULL) {
		iteratePropertiesFromExisting(
			source,
			results,
			properties,
			addRequiredProperty);
	}
	return results;
}


static fiftyoneDegreesPropertiesResults* initSpecificPropertiesFromArray(
	propertiesSource *source,
	const char** properties,
	int propertyCount) {
	uint32_t count = countPropertiesFromArray(source, properties, propertyCount);
	fiftyoneDegreesPropertiesResults *results =
		initRequiredPropertiesMemory(count, source);
	if (results != NULL) {
		iteratePropertiesFromArray(
			source,
			results,
			properties,
			propertyCount,
			addRequiredProperty);
	}
	return results;
}

static fiftyoneDegreesPropertiesResults* initAllProperties(
	propertiesSource *source) {
	uint32_t i;
	fiftyoneDegreesPropertiesResults *results =
		initRequiredPropertiesMemory(source->count, source);
	if (results != NULL) {
		for (i = 0; i < source->count; i++) {
			results->indexes[i] = i;
		}
	}
	return results;
}

static void setPropertyNames(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *results) {
	uint32_t i;
	for (i = 0; i < results->count; i++) {
		source->get(source->state, results->indexes[i], &results->names[i]);
	}
}

static int comparePropertyNamesAscending(const void *a, const void *b) {
	fiftyoneDegreesCollectionItem *ai = (fiftyoneDegreesCollectionItem*)a;
	fiftyoneDegreesCollectionItem *bi = (fiftyoneDegreesCollectionItem*)b;
	fiftyoneDegreesString *as = (fiftyoneDegreesString*)ai->data.ptr;
	fiftyoneDegreesString *bs = (fiftyoneDegreesString*)bi->data.ptr;
	return strcmp(FIFTYONEDEGREES_STRING(as), FIFTYONEDEGREES_STRING(bs));
}

static void sortRequiredProperties(
	fiftyoneDegreesPropertiesResults *results) {
	qsort((void*)results->names,
		results->count,
		sizeof(fiftyoneDegreesCollectionItem),
		comparePropertyNamesAscending);
}

static void initRequiredPropertyNames(
	propertiesSource *source,
	fiftyoneDegreesPropertiesResults *results) {
	uint32_t i;
	fiftyoneDegreesString *string;

	// Set the names for each of the properties.
	setPropertyNames(source, results);

	// Sort these names in ascending order.
	sortRequiredProperties(results);

	// The property indexes are now invalid so need to be reset from the names.
	for (i = 0; i < results->count; i++) {
		string = (fiftyoneDegreesString*)results->names[i].data.ptr;
		results->indexes[i] = getPropertyIndex(
			source,
			FIFTYONEDEGREES_STRING(string),
			string->size - 1);
	}
}

static int comparePropertyNamesAscendingSearch(const void *a, const void *b) {
	return strcmp(
		(char*)a,
		FIFTYONEDEGREES_STRING(((fiftyoneDegreesCollectionItem*)b)->data.ptr));
}

fiftyoneDegreesPropertiesResults* fiftyoneDegreesPropertiesCreate(
	void *state,
	uint32_t count,
	fiftyoneDegreesPropertiesRequired *properties,
	fiftyoneDegreesPropertiesGet get,
	void*(*malloc)(size_t),
	void(*free)(void*)) {
	propertiesSource source;
	source.count = count;
	source.state = state;
	source.get = get;
	source.malloc = malloc;
	source.free = free;
	fiftyoneDegreesPropertiesResults *results = NULL;
	if (properties != NULL) {
		if (properties->existing != NULL) {
			// Use an existing list of properties.
			results = initSpecificPropertiesFromExisting(
				&source,
				properties->existing);
		}
		else if (properties->array != NULL && properties->count > 0) {
			// Set the required properties from the array.
			results = initSpecificPropertiesFromArray(
				&source,
				properties->array,
				properties->count);
		}
		else if (properties->string != NULL) {
			// Set the required properties from the comma separated string.
			results = initRequiredPropertiesFromString(
				&source,
				properties->string);
		}
		else {
			// Set all the properties as required properties.
			results = initAllProperties(&source);
		}
	}
	else {
		// Set all the properties as required properties.
		results = initAllProperties(&source);
	}

	// Set the require property name strings to match the require property
	// index.
	if (results != NULL) {
		initRequiredPropertyNames(&source, results);
	}

	return results;
}

int fiftyoneDegreesPropertiesGetPropertyIndexFromRequiredIndex(
	fiftyoneDegreesPropertiesResults *results,
	int requiredPropertyIndex) {
	if (requiredPropertyIndex >= 0 &&
		requiredPropertyIndex < (int)results->count) {
		return results->indexes[requiredPropertyIndex];
	}
	return -1;
}

int fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(
	fiftyoneDegreesPropertiesResults *results,
	const char *propertyName) {
	int requiredPropertyIndex;
	fiftyoneDegreesCollectionItem *found = (fiftyoneDegreesCollectionItem*)
		bsearch(
			propertyName,
			results->names,
			results->count,
			sizeof(fiftyoneDegreesCollectionItem),
			comparePropertyNamesAscendingSearch);
	if (found == NULL) {
		requiredPropertyIndex = -1;
	}
	else {
		requiredPropertyIndex = (int)(found - results->names);
		assert(requiredPropertyIndex >= 0);
		assert(requiredPropertyIndex < (int)results->count);
	}
	return requiredPropertyIndex;
}

int fiftyoneDegreesPropertiesGetPropertyIndexFromName(
	fiftyoneDegreesPropertiesResults *results,
	const char *propertyName) {
	int requiredPropertyIndex =
		fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(
			results,
			propertyName);
	if (requiredPropertyIndex >= 0) {
		return fiftyoneDegreesPropertiesGetPropertyIndexFromRequiredIndex(
			results,
			requiredPropertyIndex);
	}
	else
	{
		return -1;
	}
}

fiftyoneDegreesString* fiftyoneDegreesPropertiesGetNameFromRequiredIndex(
	fiftyoneDegreesPropertiesResults *results,
	int requiredPropertyIndex) {
	return (fiftyoneDegreesString*)results->names[requiredPropertyIndex].data.ptr;
}

void fiftyoneDegreesPropertiesFree(
	fiftyoneDegreesPropertiesResults *results) {
	uint32_t i;
	if (results != NULL) {
		for (i = 0; i < results->count; i++) {
			results->names[i].collection->release(&results->names[i]);
		}
		results->free(results);
	}
}
