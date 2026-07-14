/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "indices.h"

#include <string.h>

#include "collectionKeyTypes.h"
#include "fiftyone.h"

// Working data structure used to construct the index.
typedef struct map_t {
	uint32_t availableProperty; // available property index
	int16_t propertyIndex; // index in the properties collection
} map;

// Loops through the values associated with the profile setting the index at
// the position for the property and profile to the first value index from the
// profile. The base is the index in valueIndexes of the first cell of the row
// used for the profile.
static void addProfileValuesMethod(
	IndicesPropertyProfile* index, // index in use or null if not available
	map* propertyIndexes, // property indexes in ascending order
	fiftyoneDegreesCollection* values, // collection of values
	Profile* profile,
	uint32_t base,
	Exception* exception) {
	uint32_t valueIndex;
	Item valueItem; // The current value memory
	Value* value; // The current value pointer
	DataReset(&valueItem.data);

	uint32_t* first = (uint32_t*)(profile + 1); // First value for the profile

	CollectionKey valueKey = {
		0,
		CollectionKeyType_Value,
	};
	// For each of the values associated with the profile check to see if it
	// relates to a new property index. If it does then record the first value
	// index and advance the current index to the next pointer.
	for (uint32_t i = 0, p = 0;
		i < profile->valueCount &&
		p < index->availablePropertyCount &&
		EXCEPTION_OKAY;
		i++) {
		valueKey.indexOrOffset.offset = *(first + i);
		value = values->get(values, &valueKey, &valueItem, exception);
		if (value != NULL && EXCEPTION_OKAY) {

			// If the value doesn't relate to the next property index then
			// move to the next property index.
			while (p < index->availablePropertyCount && // first check validity
				// of the subscript and then use it
                propertyIndexes[p].propertyIndex < value->propertyIndex) {
				p++;
			}

			// If the value relates to the next property index being sought
			// then record the first value in the profile associated with the
			// property.
			if (p < index->availablePropertyCount &&
				value->propertyIndex == propertyIndexes[p].propertyIndex) {
				valueIndex = base + propertyIndexes[p].availableProperty;
				index->valueIndexes[valueIndex] = i;
				p++;
				index->filled++;
			}
			COLLECTION_RELEASE(values, &valueItem);
		}
	}
}

#ifndef FIFTYONE_DEGREES_REDUCED_FILE

// Gets the index of the profile id in the property profile index.
static uint32_t getProfileIdIndex(
	IndicesPropertyProfile* index,
	uint32_t profileId) {
	return profileId - index->minProfileId;
}

static void iterateProfiles(
	fiftyoneDegreesCollection* profiles,
	fiftyoneDegreesCollection* profileOffsets,
	IndicesPropertyProfile* index, // index in use or null if not available
	map* propertyIndexes, // property indexes in ascending order
	fiftyoneDegreesCollection* values, // collection of values
	Exception *exception) {
	Profile* profile; // The current profile pointer
	Item profileItem; // The current profile memory
	ProfileOffset* profileOffset; // The current profile offset pointer
	Item profileOffsetItem; // The current profile offset memory
	DataReset(&profileItem.data);
	DataReset(&profileOffsetItem.data);
	CollectionKey profileOffsetKey = {
		0,
		CollectionKeyType_ProfileOffset,
	};
	CollectionKey profileKey = {
		0,
		CollectionKeyType_Profile,
	};
	for (uint32_t i = 0;
		i < index->profileCount && EXCEPTION_OKAY;
		i++) {
		profileOffsetKey.indexOrOffset.offset = i;
		profileOffset = profileOffsets->get(
			profileOffsets,
			&profileOffsetKey,
			&profileOffsetItem,
			exception);
		if (profileOffset != NULL && EXCEPTION_OKAY) {
			profileKey.indexOrOffset.offset = profileOffset->offset;
			profile = profiles->get(
				profiles,
				&profileKey,
				&profileItem,
				exception);
			if (profile != NULL && EXCEPTION_OKAY) {
				addProfileValuesMethod(
					index,
					propertyIndexes,
					values,
					profile,
					getProfileIdIndex(index, profile->profileId) *
						index->availablePropertyCount,
					exception);
				COLLECTION_RELEASE(profiles, &profileItem);
			}
			COLLECTION_RELEASE(profileOffsets, &profileOffsetItem);
		}
	}
}

// As the profileOffsets collection is ordered in ascending profile id the
// first and last entries are the min and max available profile ids.
static uint32_t getProfileId(
	fiftyoneDegreesCollection* profileOffsets,
	uint32_t index,
	Exception* exception) {
	uint32_t profileId = 0;
	ProfileOffset* profileOffset; // The profile offset pointer
	Item profileOffsetItem; // The profile offset memory
	DataReset(&profileOffsetItem.data);
	const CollectionKey profileOffsetKey = {
		index,
		CollectionKeyType_ProfileOffset,
	};
	profileOffset = profileOffsets->get(
		profileOffsets,
		&profileOffsetKey,
		&profileOffsetItem,
		exception);
	if (profileOffset != NULL && EXCEPTION_OKAY) {
		profileId = profileOffset->profileId;
		COLLECTION_RELEASE(profileOffsets, &profileOffsetItem);
	}
	return profileId;
}

#endif

static int comparePropertyIndexes(const void* a, const void* b) {
	return ((map*)a)->propertyIndex - ((map*)b)->propertyIndex;
}

// Build an ascending ordered array of the property indexes.
static map* createPropertyIndexes(
	PropertiesAvailable* available,
	Exception* exception) {
	map* index = (map*)Malloc(sizeof(map) * available->count);
	if (index == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		return NULL;
	}
	for (uint32_t i = 0; i < available->count; i++) {
		index[i].availableProperty = i;
		index[i].propertyIndex = (int16_t)available->items[i].propertyIndex;
	}
	qsort(index, available->count, sizeof(map*), comparePropertyIndexes);
	return index;
}

fiftyoneDegreesIndicesPropertyProfile*
fiftyoneDegreesIndicesPropertyProfileCreate(
	fiftyoneDegreesCollection* profiles,
	fiftyoneDegreesCollection* profileOffsets,
	fiftyoneDegreesPropertiesAvailable* available,
	fiftyoneDegreesCollection* values,
	fiftyoneDegreesException* exception) {
#ifdef FIFTYONE_DEGREES_REDUCED_FILE
	// A reduced size data file does not contain profile ids, so a profile id
	// keyed index cannot be created. Use
	// fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets instead.
#ifdef _MSC_VER
	UNREFERENCED_PARAMETER(profiles);
	UNREFERENCED_PARAMETER(profileOffsets);
	UNREFERENCED_PARAMETER(available);
	UNREFERENCED_PARAMETER(values);
#endif
	EXCEPTION_SET(NOT_IMPLEMENTED);
	return NULL;
#else

	// Create the ordered list of property indexes.
	map* propertyIndexes = createPropertyIndexes(available, exception);
	if (propertyIndexes == NULL) {
		return NULL;
	}

	// Allocate memory for the index and set the fields.
	IndicesPropertyProfile* index = (IndicesPropertyProfile*)Malloc(
		sizeof(IndicesPropertyProfile));
	if (index == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		return NULL;
	}
	index->filled = 0;
	index->profileOffsets = NULL; // keyed by profile id
	index->profileCount = CollectionGetCount(profileOffsets);
	index->minProfileId = getProfileId(profileOffsets, 0, exception);
	if (!EXCEPTION_OKAY) {
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}
	index->maxProfileId = getProfileId(
		profileOffsets,
		index->profileCount - 1,
		exception);
	if (!EXCEPTION_OKAY) {
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}
	index->availablePropertyCount = available->count;
	index->size = (index->maxProfileId - index->minProfileId + 1) * 
		available->count;
	
	// Allocate memory for the values index and set the fields.
	index->valueIndexes =(uint32_t*)Malloc(sizeof(uint32_t) * index->size);
	if (index->valueIndexes == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}

	// For each of the profiles in the collection call add the property value
	// indexes to the index array.
	iterateProfiles(
		profiles, 
		profileOffsets, 
		index, 
		propertyIndexes,
		values,
		exception);
	Free(propertyIndexes);

	// Return the index or free the memory if there was an exception.
	if (EXCEPTION_OKAY) {
		return index;
	}
	else {
		Free(index->valueIndexes);
		Free(index);
		return NULL;
	}
#endif
}

// Ascending order comparison for two profile offsets.
static int compareProfileOffsets(const void* a, const void* b) {
	const uint32_t offsetA = *(const uint32_t*)a;
	const uint32_t offsetB = *(const uint32_t*)b;
	return offsetA < offsetB ? -1 : (offsetA > offsetB ? 1 : 0);
}

// Gets the pure profile offset for the entry at entryIndex in the
// profileOffsets collection using the extractor provided. Returns true if the
// entry was read successfully.
static bool getPureProfileOffset(
	fiftyoneDegreesCollection* profileOffsets,
	fiftyoneDegreesProfileOffsetValueExtractor offsetValueExtractor,
	uint32_t entryIndex,
	uint32_t* pureOffset,
	Exception* exception) {
	Item entryItem; // The current profile offset entry memory
	DataReset(&entryItem.data);
	const CollectionKey entryKey = {
		entryIndex,
		CollectionKeyType_ProfileOffset,
	};
	const void* rawEntry = profileOffsets->get(
		profileOffsets,
		&entryKey,
		&entryItem,
		exception);
	if (rawEntry == NULL || EXCEPTION_FAILED) {
		return false;
	}
	*pureOffset = offsetValueExtractor(rawEntry);
	COLLECTION_RELEASE(profileOffsets, &entryItem);
	return true;
}

fiftyoneDegreesIndicesPropertyProfile*
fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets(
	fiftyoneDegreesCollection* profiles,
	fiftyoneDegreesCollection* profileOffsets,
	fiftyoneDegreesProfileOffsetValueExtractor offsetValueExtractor,
	fiftyoneDegreesPropertiesAvailable* available,
	fiftyoneDegreesCollection* values,
	fiftyoneDegreesException* exception) {
	uint32_t i;
	Profile* profile; // The current profile pointer
	Item profileItem; // The current profile memory

	// Check the number of cells needed can be counted and addressed.
	const uint32_t entryCount = CollectionGetCount(profileOffsets);
	if (entryCount == 0 ||
		available->count == 0 ||
		(uint64_t)entryCount * available->count > (uint64_t)UINT32_MAX) {
		return NULL;
	}

	// Create the ordered list of property indexes.
	map* propertyIndexes = createPropertyIndexes(available, exception);
	if (propertyIndexes == NULL) {
		return NULL;
	}

	// Allocate memory for the index and set the fields.
	IndicesPropertyProfile* index = (IndicesPropertyProfile*)Malloc(
		sizeof(IndicesPropertyProfile));
	if (index == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		Free(propertyIndexes);
		return NULL;
	}
	index->filled = 0;
	index->minProfileId = 0; // unused when keyed by profile offset
	index->maxProfileId = 0; // unused when keyed by profile offset
	index->availablePropertyCount = available->count;

	// Read all the profile offsets into an array sorted in ascending order
	// with any duplicate entries removed.
	index->profileOffsets = (uint32_t*)Malloc(
		sizeof(uint32_t) * entryCount);
	if (index->profileOffsets == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}
	uint32_t count = 0;
	for (i = 0; i < entryCount && EXCEPTION_OKAY; i++) {
		uint32_t pureOffset;
		if (getPureProfileOffset(
			profileOffsets,
			offsetValueExtractor,
			i,
			&pureOffset,
			exception)) {
			index->profileOffsets[count++] = pureOffset;
		}
	}
	if (EXCEPTION_FAILED || count == 0) {
		Free(index->profileOffsets);
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}
	qsort(
		index->profileOffsets,
		count,
		sizeof(uint32_t),
		compareProfileOffsets);
	uint32_t distinct = 1;
	for (i = 1; i < count; i++) {
		if (index->profileOffsets[i] != index->profileOffsets[distinct - 1]) {
			index->profileOffsets[distinct++] = index->profileOffsets[i];
		}
	}
	index->profileCount = distinct;
	index->size = distinct * available->count;

	// Allocate memory for the value indexes and set every cell to the no
	// value marker so that profiles without values for a property can be
	// identified.
	index->valueIndexes = (uint32_t*)Malloc(sizeof(uint32_t) * index->size);
	if (index->valueIndexes == NULL) {
		EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY);
		Free(index->profileOffsets);
		Free(index);
		Free(propertyIndexes);
		return NULL;
	}
	memset(index->valueIndexes, 0xFF, sizeof(uint32_t) * index->size);

	// For each of the distinct profile offsets add the property value indexes
	// to the row for the profile.
	DataReset(&profileItem.data);
	CollectionKey profileKey = {
		0,
		CollectionKeyType_Profile,
	};
	for (i = 0; i < distinct && EXCEPTION_OKAY; i++) {
		profileKey.indexOrOffset.offset = index->profileOffsets[i];
		profile = (Profile*)profiles->get(
			profiles,
			&profileKey,
			&profileItem,
			exception);
		if (profile != NULL && EXCEPTION_OKAY) {
			addProfileValuesMethod(
				index,
				propertyIndexes,
				values,
				profile,
				i * index->availablePropertyCount,
				exception);
			COLLECTION_RELEASE(profiles, &profileItem);
		}
	}
	Free(propertyIndexes);

	// Return the index or free the memory if there was an exception.
	if (EXCEPTION_OKAY) {
		return index;
	}
	else {
		Free(index->valueIndexes);
		Free(index->profileOffsets);
		Free(index);
		return NULL;
	}
}

void fiftyoneDegreesIndicesPropertyProfileFree(
	fiftyoneDegreesIndicesPropertyProfile* index) {
	if (index->profileOffsets != NULL) {
		Free(index->profileOffsets);
	}
	Free(index->valueIndexes);
	Free(index);
}

uint32_t fiftyoneDegreesIndicesPropertyProfileLookup(
	fiftyoneDegreesIndicesPropertyProfile* index,
	uint32_t profileId,
	uint32_t availablePropertyIndex) {
#ifdef FIFTYONE_DEGREES_REDUCED_FILE
	// A reduced size data file does not contain profile ids, so a profile id
	// keyed index is never created.
#ifdef _MSC_VER
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(profileId);
	UNREFERENCED_PARAMETER(availablePropertyIndex);
#endif
	assert(false);
	return 0;
#else
	uint32_t valueIndex =
		(getProfileIdIndex(index, profileId) * index->availablePropertyCount) +
		availablePropertyIndex;
	assert(valueIndex < index->size);
	return index->valueIndexes[valueIndex];
#endif
}

bool fiftyoneDegreesIndicesPropertyProfileLookupByOffset(
	const fiftyoneDegreesIndicesPropertyProfile* index,
	uint32_t profileOffset,
	uint32_t availablePropertyIndex,
	uint32_t* firstValueIndex) {
	if (index == NULL ||
		index->profileOffsets == NULL ||
		availablePropertyIndex >= index->availablePropertyCount) {
		return false;
	}

	// Binary search for the row relating to the profile offset.
	uint32_t lower = 0;
	uint32_t upper = index->profileCount;
	while (lower < upper) {
		const uint32_t middle = lower + (upper - lower) / 2;
		if (index->profileOffsets[middle] < profileOffset) {
			lower = middle + 1;
		}
		else {
			upper = middle;
		}
	}
	if (lower >= index->profileCount ||
		index->profileOffsets[lower] != profileOffset) {
		return false;
	}
	*firstValueIndex = index->valueIndexes[
		((uint64_t)lower * index->availablePropertyCount) +
			availablePropertyIndex];
	return true;
}
