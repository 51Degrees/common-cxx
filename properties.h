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

#ifndef FIFTYONEDEGREES_PROPERTIES_H_INCLUDED
#define FIFTYONEDEGREES_PROPERTIES_H_INCLUDED

#include <stdint.h>
#include "string.h"
#include "list.h"
#include "data.h"
#include "collection.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

typedef fiftyoneDegreesString*(*fiftyoneDegreesPropertiesGet)(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item);

typedef struct fiftyone_degrees_properties_results_t {
	uint32_t count; /* Number of required properties */
	uint32_t *indexes; /* Array of property indexes */
	fiftyoneDegreesCollectionItem *names; /* Array of property name items from
										  string collection */
	void(*free)(void*); /* Frees memory */
} fiftyoneDegreesPropertiesResults;

EXTERNAL typedef struct fiftyone_degrees_properties_required_t {
	const char **array; /* Array of required properties or NULL if all
                        properties are required. See the count property for the
                        number of items in the array */
	int count; /* Number of properties in array */
	char *string; /* Separated list of required properties or NULL if all
				  properties are required */
	fiftyoneDegreesPropertiesResults *existing; /* A pointer to an existing
												set of property names from
												another data set instance */
} fiftyoneDegreesPropertiesRequired;

EXTERNAL fiftyoneDegreesPropertiesRequired fiftyoneDegreesPropertiesDefault;

fiftyoneDegreesPropertiesResults* fiftyoneDegreesPropertiesCreate(
	void *state,
	uint32_t count,
	fiftyoneDegreesPropertiesRequired *properties,
	fiftyoneDegreesPropertiesGet get,
	void*(*malloc)(size_t),
	void(*free)(void*));

int fiftyoneDegreesPropertiesGetPropertyIndexFromName(
	fiftyoneDegreesPropertiesResults *results,
	const char *propertyName);

int fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(
	fiftyoneDegreesPropertiesResults *results,
	const char *propertyName);

int fiftyoneDegreesPropertiesGetPropertyIndexFromRequiredIndex(
	fiftyoneDegreesPropertiesResults *results,
	int requiredPropertyIndex);

fiftyoneDegreesString* fiftyoneDegreesPropertiesGetNameFromRequiredIndex(
	fiftyoneDegreesPropertiesResults *results,
	int requiredPropertyIndex);

void fiftyoneDegreesPropertiesFree(fiftyoneDegreesPropertiesResults *results);

#endif
