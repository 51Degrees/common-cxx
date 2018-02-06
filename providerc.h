#ifndef FIFTYONEDEGREES_PROVIDER_INCLUDED
#define FIFTYONEDEGREES_PROVIDER_INCLUDED

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
	FIFTYONEDEGREES_MUTEX lock; /* Used to lock critical regions where mutable
								variables are written to */
#else
	fiftyoneDegreesActiveResource *active; /* Non volatile wrapper for the
										   provider's data set. */
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
