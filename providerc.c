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

#include "providerc.h"

static const void* setResource(
	fiftyoneDegreesProvider *provider,
	const void *resource) {
	fiftyoneDegreesProviderResourceHandle *tracker;

	// Create a new active wrapper for the provider.
	tracker = (fiftyoneDegreesProviderResourceHandle*)provider->malloc(
		sizeof(fiftyoneDegreesProviderResourceHandle));
	if (tracker != NULL) {

		// Set the number of offsets using the active dataset to zero.
		tracker->inUse = 0;

		// Set a link between the new active resource and the provider. Used to
		// check if the resource can be freed when the last thread has finished
		// using it.
		tracker->provider = provider;

		// Switch the active dataset for the provider to the newly created one.
		tracker->resource = resource;
		provider->active = tracker;
	}

	return provider->active->resource;
}

static void freeTracker(fiftyoneDegreesProviderResourceHandle *tracker) {
	tracker->provider->freeResource((void*)tracker->resource);
	tracker->provider->freeMemory((void*)tracker);
}

const void* fiftyoneDegreesProviderInit(
	fiftyoneDegreesProvider *provider,
	const void *resource,
	void*(*malloc)(size_t),
	void(*freeMemory)(void*),
	void(*freeResource)(void*)) {

	// Set the malloc and free methods which never change.
	provider->malloc = malloc;
	provider->freeResource = freeResource;
	provider->freeMemory = freeMemory;

#ifndef FIFTYONEDEGREES_NO_THREADING
	// Create a new lock if needed.
	FIFTYONEDEGREES_MUTEX_CREATE(provider->lock);
#endif

	// Initialise the provider with the resource.
	return setResource(provider, resource);
}

void fiftyoneDegreesProviderFree(fiftyoneDegreesProvider *provider) {
#ifndef FIFTYONEDEGREES_NO_THREADING
	FIFTYONEDEGREES_MUTEX_CLOSE(provider->lock);
#endif
	provider->freeResource((void*)provider->active->resource);
	provider->freeMemory((void*)provider->active);
}

fiftyoneDegreesProviderResourceHandle* fiftyoneDegreesProviderIncUse(
	fiftyoneDegreesProvider *provider) {
	fiftyoneDegreesProviderResourceHandle *tracker = NULL;
#ifndef FIFTYONEDEGREES_NO_THREADING
	do {
		if (tracker != NULL) {
			fiftyoneDegreesProviderDecUse(tracker);
		}
		tracker = (fiftyoneDegreesProviderResourceHandle*)provider->active;
		FIFTYONEDEGREES_INTERLOCK_INC(&tracker->inUse);
	} while (tracker != provider->active);
#else
	tracker = (fiftyoneDegreesProviderResourceHandle*)provider->active;
	tracker->inUse--;
#endif
	return tracker;
}

void fiftyoneDegreesProviderDecUse(fiftyoneDegreesProviderResourceHandle *tracker) {
	assert(tracker->inUse > 0);
#ifndef FIFTYONEDEGREES_NO_THREADING
	if (FIFTYONEDEGREES_INTERLOCK_DEC(&tracker->inUse) == 0 &&
#else
	tracker->inUse--;
	if (tracker->inUse == 0 &&
#endif
		tracker->provider->active != tracker) {
		freeTracker(tracker);
	}
}

const void* fiftyoneDegreesProviderReplace(
	fiftyoneDegreesProvider *provider,
	const void *newResource) {
	fiftyoneDegreesProviderResourceHandle *existing =
		(fiftyoneDegreesProviderResourceHandle*)provider->active;

#ifndef FIFTYONEDEGREES_NO_THREADING
	FIFTYONEDEGREES_MUTEX_LOCK(&provider->lock);
#endif
	// Add the new resource to the provider replacing the existing one.
	if (setResource(provider, newResource) != NULL) {
		// Check if the existing resource can be freed. If so then free it
		// as it will never be checked again.
		if (existing->inUse == 0) {
			freeTracker(existing);
		}
	}
#ifndef FIFTYONEDEGREES_NO_THREADING
	FIFTYONEDEGREES_MUTEX_UNLOCK(&provider->lock);
#endif

	// Return the currently active resource for the provider.
	return provider->active->resource;
}