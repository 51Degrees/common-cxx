#include "data.h"

void fiftyoneDegreesDataReset(fiftyoneDegreesData *data) {
	data->ptr = NULL;
	data->allocated = 0;
	data->used = 0;
}

void* fiftyoneDegreesDataMalloc(
	fiftyoneDegreesData *data, 
	size_t bytesNeeded, 
	void*(*malloc)(size_t),
	void(*free)(void*)) {
	if (data->allocated > 0 &&
		bytesNeeded > data->allocated) {
		free(data->ptr);
		data->ptr = NULL;
		data->allocated = 0;
	}
	if (data->allocated == 0) {
		data->ptr = (byte*)malloc(bytesNeeded);
		if (data->ptr != NULL) {
			data->allocated = (uint32_t)bytesNeeded;
		}
	}
	return data->ptr;
}