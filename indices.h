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

#ifndef FIFTYONE_DEGREES_INDICES_H_INCLUDED
#define FIFTYONE_DEGREES_INDICES_H_INCLUDED

 /**
  * @ingroup FiftyOneDegreesCommon
  * @defgroup FiftyOneDegreesIndices Indices
  *
  * A look up structure for profile and property index to the first value
  * associated with the property and profile.
  *
  * ## Introduction
  *
  * Data sets relate profiles to the values associated with them. Values are
  * associated with properties. The values associated with a profile are
  * ordered in ascending order of property. Therefore when a request is made to
  * obtain the value for a property and profile the values needed to be
  * searched using a binary search to find a value related to the property.
  * Then the list of prior values is checked until the first value for the
  * property is found.
  *
  * The indices methods provide common functionality to create a structure
  * that directly relates profile ids and required property indexes to the
  * first value index thus increasing the efficiency of retrieving values.
  *
  * It is expected these methods will be used during data set initialization.
  *
  * ## Structure
  *
  * A sparse array of profile ids and required property indexes is used. Whilst
  * this consumes more linear memory than a binary tree or other structure it
  * is extremely fast to retrieve values from. As the difference between the
  * lowest and highest profile id is relatively small the memory associated
  * with absent profile ids is considered justifiable considering the
  * performance benefit. A further optimization is to use the required property
  * index rather than the index of all possible properties contained in the
  * data set. In most use cases the caller only requires a sub set of
  * properties to be available for retrieval.
  *
  * ## Profile offset keyed indexes
  *
  * Some data files (for example IP Intelligence, and any reduced size data
  * file) do not contain profile ids. For these data files an alternative,
  * profile offset keyed, form of the index is created with
  * fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets. Instead of a sparse
  * array keyed by profile id, a dense array of one row per distinct profile
  * offset is used together with a sorted array of the offsets. Lookup is
  * performed with fiftyoneDegreesIndicesPropertyProfileLookupByOffset which
  * binary searches the sorted offsets to find the row.
  *
  * ## Create
  *
  * fiftyoneDegreesIndicesPropertyProfileCreate should be called once the data
  * set is initialized with the required data structures. Memory is allocated
  * by the method and a pointer to the index data structure is returned. The
  * caller is not expected to use the returned data structure directly.
  *
  * Some working memory is allocated during the indexing process. Therefore
  * this method must be called before a freeze on allocating new memory is
  * required.
  *
  * ## Free
  *
  * fiftyoneDegreesIndicesPropertyProfileFree is used to free the memory used
  * by the index returned from Create. Must be called during the freeing of the
  * related data set.
  *
  * ## Lookup
  *
  * fiftyoneDegreesIndicesPropertyProfileLookup is used to return the index in
  * the values associated with the profile for the profile id and the required
  * property index.
  *
  * @{
  */

#include <stdint.h>
#include <stdbool.h>
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 5105) 
#include <windows.h>
#pragma warning (default: 5105) 
#pragma warning (pop)
#endif
#include "array.h"
#include "data.h"
#include "exceptions.h"
#include "collection.h"
#include "property.h"
#include "properties.h"
#include "common.h"

/**
 * Function that extracts a "pure" profile offset from a value inside the
 * `profileOffsets` collection. Data files with full
 * #fiftyoneDegreesProfileOffset entries use
 * #fiftyoneDegreesProfileOffsetToPureOffset, data files where the entries are
 * plain 32 bit offsets use #fiftyoneDegreesProfileOffsetAsPureOffset. See
 * profile.h.
 * @param rawProfileOffset a "raw" value retrieved from `profileOffsets`
 * @return Offset to the profile in the profiles structure
 */
typedef uint32_t (*fiftyoneDegreesProfileOffsetValueExtractor)(
	const void *rawProfileOffset);

/**
 * Maps the profile index and the property index to the first value index of
 * the profile for the property. Is an array of uint32_t with entries equal to
 * the number of properties multiplied by the difference between the lowest and
 * highest profile id.
 *
 * When created with fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets
 * the index is keyed by profile offset rather than profile id: profileOffsets
 * is a sorted array with profileCount distinct entries, valueIndexes holds one
 * row of availablePropertyCount entries per offset, and minProfileId and
 * maxProfileId are unused. Cells with no value for the property are set to
 * UINT32_MAX.
 */
typedef struct fiftyone_degrees_index_property_profile{
	uint32_t* valueIndexes; // array of value indexes
	uint32_t availablePropertyCount; // number of available properties
	uint32_t minProfileId; // minimum profile id
	uint32_t maxProfileId; // maximum profile id
	uint32_t profileCount; // total number of profiles
	uint32_t size; // number elements in the valueIndexes array
	uint32_t filled; // number of elements with values
	uint32_t* profileOffsets; // sorted distinct profile offsets when keyed by
							  // offset, or NULL when keyed by profile id
} fiftyoneDegreesIndicesPropertyProfile;

/**
 * Create an index for the profiles, available properties, and values provided 
 * such that given the index to a property and profile the index of the first 
 * value can be returned by calling fiftyoneDegreesIndicesPropertyProfileLookup.
 * @param profiles collection of variable sized profiles to be indexed
 * @param profileOffsets collection of fixed offsets to profiles to be indexed
 * @param available properties provided by the caller
 * @param values collection to be indexed
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the index memory structure
 */
EXTERNAL fiftyoneDegreesIndicesPropertyProfile*
fiftyoneDegreesIndicesPropertyProfileCreate(
	fiftyoneDegreesCollection* profiles,
	fiftyoneDegreesCollection* profileOffsets,
	fiftyoneDegreesPropertiesAvailable* available,
	fiftyoneDegreesCollection* values,
	fiftyoneDegreesException* exception);

/**
 * Create an index keyed by profile offset rather than profile id for the
 * profiles, available properties, and values provided. Suitable for data
 * files where the profiles do not contain profile ids (for example IP
 * Intelligence data files, or reduced size data files). Given a profile
 * offset and the index of an available property the index of the first value
 * can be returned by calling
 * fiftyoneDegreesIndicesPropertyProfileLookupByOffset.
 * @param profiles collection of variable sized profiles to be indexed
 * @param profileOffsets collection of fixed entries which contain, or are,
 * offsets to the profiles to be indexed
 * @param offsetValueExtractor function which returns the pure profile offset
 * from an entry in the profileOffsets collection
 * @param available properties provided by the caller
 * @param values collection to be indexed
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the index memory structure, or NULL if the index could
 * not be created
 */
EXTERNAL fiftyoneDegreesIndicesPropertyProfile*
fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets(
	fiftyoneDegreesCollection* profiles,
	fiftyoneDegreesCollection* profileOffsets,
	fiftyoneDegreesProfileOffsetValueExtractor offsetValueExtractor,
	fiftyoneDegreesPropertiesAvailable* available,
	fiftyoneDegreesCollection* values,
	fiftyoneDegreesException* exception);

/**
 * Frees an index previously created by
 * fiftyoneDegreesIndicesPropertyProfileCreate.
 * @param index to be freed
 */
EXTERNAL void fiftyoneDegreesIndicesPropertyProfileFree(
	fiftyoneDegreesIndicesPropertyProfile* index);

/**
 * For a given profile id and available property index returns the first value 
 * index, or null if a first index can not be determined from the index. The
 * indexes relate to the collections for profiles, properties, and values 
 * provided to the fiftyoneDegreesIndicesPropertyProfileCreate method when the 
 * index was created. The availablePropertyIndex is not the index of all 
 * possible properties, but the index of the ones the data set was created 
 * expecting to return.
 * @param index from fiftyoneDegreesIndicesPropertyProfileCreate to use
 * @param profileId the values need to relate to
 * @param availablePropertyIndex in the list of required properties
 * @return the index in the list of values for the profile for the first value 
 * associated with the property
 */
EXTERNAL uint32_t fiftyoneDegreesIndicesPropertyProfileLookup(
	fiftyoneDegreesIndicesPropertyProfile* index,
	uint32_t profileId,
	uint32_t availablePropertyIndex);

/**
 * For a given profile offset and available property index returns the first
 * value index via the firstValueIndex parameter. Only for use with indexes
 * created by fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets. A
 * returned first value index of UINT32_MAX means the profile is known to the
 * index and has no values for the property.
 * @param index from fiftyoneDegreesIndicesPropertyProfileCreateFromOffsets
 * @param profileOffset offset of the profile in the profiles collection
 * @param availablePropertyIndex in the list of required properties
 * @param firstValueIndex set to the index in the list of values for the
 * profile for the first value associated with the property
 * @return true if the profile offset is known to the index and
 * firstValueIndex was set, otherwise false
 */
EXTERNAL bool fiftyoneDegreesIndicesPropertyProfileLookupByOffset(
	const fiftyoneDegreesIndicesPropertyProfile* index,
	uint32_t profileOffset,
	uint32_t availablePropertyIndex,
	uint32_t* firstValueIndex);

/**
 * @}
 */

#endif