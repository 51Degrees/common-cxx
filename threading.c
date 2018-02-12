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

#ifndef FIFTYONEDEGREES_NO_THREADING

#include "threading.h"
#include <assert.h>

#ifdef _MSC_VER

#include <windows.h>

fiftyoneDegreesSignal* fiftyoneDegreesSignalCreate()  {
	fiftyoneDegreesSignal *signal = (fiftyoneDegreesSignal*)CreateEventEx(
		NULL,
		NULL,
		CREATE_EVENT_INITIAL_SET,
		EVENT_MODIFY_STATE | SYNCHRONIZE);
	assert(signal != NULL);
	return signal;
}
void fiftyoneDegreesSignalClose(fiftyoneDegreesSignal *signal) {
	if (signal != NULL) {
		CloseHandle(signal);
	}
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4189) 
#endif
void fiftyoneDegreesSignalSet(fiftyoneDegreesSignal *signal) {
	BOOL result = SetEvent(signal);
	assert(result != 0);
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4189) 
#endif
void fiftyoneDegreesSignalWait(fiftyoneDegreesSignal *signal) {
	DWORD result = WaitForSingleObject(signal, INFINITE);
	assert(result == WAIT_OBJECT_0);
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#else

#include <pthread.h>
#ifdef __APPLE__
#include <sys/time.h>
#endif

/**
 * GCC / PTHREAD SIGNAL IMPLEMENTATION - NOT USED BY WINDOWS
 */

/**
 * Initialises the mutex passed to the method.
 * @param mutex to be initialised.
 */
void fiftyoneDegreesMutexCreate(fiftyoneDegreesMutex *mutex) {
    int result = pthread_mutex_init(mutex, NULL);
	assert(result == 0);
}

/**
 * Closes the mutex passed to the method.
 * @param mutex to be closed.
 */
void fiftyoneDegreesMutexClose(fiftyoneDegreesMutex *mutex) {
	pthread_mutex_destroy(mutex);
}

/**
 * Locks the mutex passed to the method.
 * @param mutex to be locked.
 */
void fiftyoneDegreesMutexLock(fiftyoneDegreesMutex *mutex) {
	pthread_mutex_lock(mutex);
}

/**
 * Unlocks the mutex passed to the method.
 * @param mutex to be unlocked.
 */
void fiftyoneDegreesMutexUnlock(fiftyoneDegreesMutex *mutex) {
	pthread_mutex_unlock(mutex);
}

/**
 * Initialises the signal pointer by setting the condition first followed by
 * the mutex if the condition was set correctly. Destroyed is set to false to
 * indicate to the other methods that the signal is still valid. The memory
 * used by the signal should be part of another structure and will be released
 * when that structure is released. If there is a problem creating the mutex
 * the condition is also released.
 * @param signal to be initialised
 */
fiftyoneDegreesSignal* fiftyoneDegreesSignalCreate() {
    fiftyoneDegreesSignal *signal = (fiftyoneDegreesSignal*)
        malloc(sizeof(fiftyoneDegreesSignal));
    if (signal != NULL) {
        signal->cond = NULL;
        signal->mutex = NULL;
        signal->wait = false;
        if (pthread_cond_init(&signal->cond, NULL) != 0 ||
            pthread_mutex_init(&signal->mutex, NULL) != 0) {
            free(signal);
            signal = NULL;
        }
    }
    return signal;
}

/**
 * Closes the signal ensuring there is a lock on the signal before destroying
 * the signal. This means that no other process can be waiting on the signal
 * before it is destroyed. The destroyed field of the signal structure is set
 * to true after the condition is destroyed. All methods that could
 * subsequently try and get a lock on the signal MUST check the destroyed
 * field before trying to get the lock.
 * @param signal to be closed.
 */
void fiftyoneDegreesSignalClose(fiftyoneDegreesSignal *signal) {
	if (signal != NULL) {
        pthread_mutex_destroy(&signal->mutex);
        pthread_cond_destroy(&signal->cond);
	}
}

/**
 * If the signal has not been destroyed then sends a signal to a waiting
 * thread that the signal has been set and one can continue. This possible
 * because the condition will auto reset only enabling a signal thread to
 * continue even if multi threads are waiting.
 * @param signal to be set.
 */
void fiftyoneDegreesSignalSet(fiftyoneDegreesSignal *signal) {
    if (pthread_mutex_lock(&signal->mutex) == 0) {
        signal->wait = false;
        pthread_cond_signal(&signal->cond);
        pthread_mutex_unlock(&signal->mutex);
    }
}

/**
 * Wait for a signal to be set. Only waits for the signal if the signal has not
 * been destroyed. Locks the mutex before the signal is waited for. This ensures
 * only one thread can be waiting on the signal at any one time.
 * @param signal pointer to the signal used to wait on.
 */
void fiftyoneDegreesSignalWait(fiftyoneDegreesSignal *signal) {
	int result;
    if (pthread_mutex_lock(&signal->mutex) == 0) {
        while (signal->wait == true) {
            result = pthread_cond_wait(&signal->cond, &signal->mutex);
            assert(result == 0);
        }
        signal->wait = true;
        pthread_mutex_unlock(&signal->mutex);
    }
}

#endif
#endif
