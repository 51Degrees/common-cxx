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

#ifndef FIFTYONEDEGREES_PROVIDER_INCLUDED
#define FIFTYONEDEGREES_PROVIDER_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "threading.h"

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

/* Initial declaration of the provider structure. */
typedef struct fiftyoneDegrees_provider_t fiftyoneDegreesProvider;

/**
* Tracks the number of active uses of the resource within the provider.
*/
#pragma pack(push, 4)
typedef struct fiftyoneDegrees_provider_resource_handle_t {
	const void *resource; /* Pointer to the resource the provider is providing.
						  */
	const fiftyoneDegreesProvider *provider; /* Pointer to the provider the
											 tracker relates to. */
	volatile long inUse; /* Counter indicating the number of active
						 uses of the resource. */
} fiftyoneDegreesProviderResourceHandle;
#pragma pack(pop)

/**
 * Provider structure used to provide access to a shared and changing resource.
 */
typedef struct fiftyoneDegrees_provider_t {
#ifndef FIFTYONEDEGREES_NO_THREADING
	fiftyoneDegreesProviderResourceHandle volatile *active; /* Current handle 
															for resource used 
															by the provider. */
	fiftyoneDegreesMutex lock; /* Used to lock critical regions where mutable
							   variables are written to */
#else
	fiftyoneDegreesProviderResourceHandle *active; /* Non volatile wrapper for 
												   the provider's data set. */
#endif
	void*(*malloc)(size_t);
	void(*freeMemory)(void*);
	void(*freeResource)(void*);
} fiftyoneDegreesProvider;

EXTERNAL const void* fiftyoneDegreesProviderInit(
	fiftyoneDegreesProvider *provider,
	const void *resource,
	void*(*malloc)(size_t),
	void(*freeMemory)(void*),
	void(*freeResource)(void*));

EXTERNAL void fiftyoneDegreesProviderFree(fiftyoneDegreesProvider *provider);

EXTERNAL fiftyoneDegreesProviderResourceHandle* fiftyoneDegreesProviderIncUse(
	fiftyoneDegreesProvider *provider);

EXTERNAL void fiftyoneDegreesProviderDecUse(fiftyoneDegreesProviderResourceHandle *tracker);

EXTERNAL const void* fiftyoneDegreesProviderReplace(
	fiftyoneDegreesProvider *provider,
	const void *newResource);

#endif
