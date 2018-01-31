#include "providerc.h"

static const void* setResource(
	fiftyoneDegreesProvider *provider,
	const void *resource) {
	fiftyoneDegreesProviderTracker *tracker;

	// Create a new active wrapper for the provider.
	tracker = (fiftyoneDegreesProviderTracker*)malloc(
		sizeof(fiftyoneDegreesProviderTracker));
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

static void freeTracker(fiftyoneDegreesProviderTracker *tracker) {
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

fiftyoneDegreesProviderTracker* fiftyoneDegreesProviderIncUse(
	fiftyoneDegreesProvider *provider) {
	fiftyoneDegreesProviderTracker *tracker = NULL;
#ifndef FIFTYONEDEGREES_NO_THREADING
	do {
		if (tracker != NULL) {
			fiftyoneDegreesProviderDecUse(tracker);
		}
		tracker = (fiftyoneDegreesProviderTracker*)provider->active;
		FIFTYONEDEGREES_INTERLOCK_INC(&tracker->inUse);
	} while (tracker != provider->active);
#else
	tracker = (fiftyoneDegreesProviderTracker*)provider->active;
	tracker->inUse--;
#endif
	return tracker;
}

void fiftyoneDegreesProviderDecUse(fiftyoneDegreesProviderTracker *tracker) {
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
	fiftyoneDegreesProviderTracker *existing = 
		(fiftyoneDegreesProviderTracker*)provider->active;

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

