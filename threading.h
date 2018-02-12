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

/**
 * Define macros if compiler directive is not explicitly requesting single
 * threaded operation.
 */

#ifndef FIFTYONEDEGREES_THREADING_INCLUDED
#define FIFTYONEDEGREES_THREADING_INCLUDED

#ifndef FIFTYONEDEGREES_NO_THREADING

/**
* MUTEX AND THREADING MACROS
*/

#include <stdlib.h>
#include <stdbool.h>

#ifdef _MSC_VER
#include <windows.h>
#include <intrin.h>
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#else
#include <pthread.h>
#include <signal.h>
#endif

/**
* Mutex used to synchronise access to data structures that could be used
* in parallel in a multi threaded environment.
*/
#ifdef _MSC_VER
typedef HANDLE fiftyoneDegreesMutex;
#else
typedef pthread_mutex_t fiftyoneDegreesMutex;
void fiftyoneDegreesMutexCreate(fiftyoneDegreesMutex *mutex);
void fiftyoneDegreesMutexClose(fiftyoneDegreesMutex *mutex);
void fiftyoneDegreesMutexLock(fiftyoneDegreesMutex *mutex);
void fiftyoneDegreesMutexUnlock(fiftyoneDegreesMutex *mutex);
#endif

/**
* A signal used to limit the number of worksets that can be created by
* the pool.
*/
#ifdef _MSC_VER
typedef HANDLE fiftyoneDegreesSignal;
#else
typedef struct fiftyoneDegrees_signal_t {
    volatile bool wait;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} fiftyoneDegreesSignal;
#endif
fiftyoneDegreesSignal* fiftyoneDegreesSignalCreate();
void fiftyoneDegreesSignalClose(fiftyoneDegreesSignal *signal);
void fiftyoneDegreesSignalSet(fiftyoneDegreesSignal *signal);
void fiftyoneDegreesSignalWait(fiftyoneDegreesSignal *signal);

/**
 * A thread created with the CREATE_THREAD macro.
 */
#ifdef _MSC_VER
#define FIFTYONEDEGREES_THREAD HANDLE
#else
#define FIFTYONEDEGREES_THREAD pthread_t
#endif

/**
* Creates a new signal that can be used to wait for
* other operations to complete before continuing.
*/
#define FIFTYONEDEGREES_SIGNAL_CREATE(s) s = fiftyoneDegreesSignalCreate()

/**
* Frees the handle provided to the macro.
*/
#define FIFTYONEDEGREES_SIGNAL_CLOSE(s) fiftyoneDegreesSignalClose(s)

/**
* Signals a thread waiting for the signal to proceed.
*/
#define FIFTYONEDEGREES_SIGNAL_SET(s) fiftyoneDegreesSignalSet(s)

/**
* Waits for the signal to become set by another thread.
*/
#define FIFTYONEDEGREES_SIGNAL_WAIT(s) fiftyoneDegreesSignalWait(s)

/**
* Creates a new mutex at the pointer provided.
*/
#ifdef _MSC_VER
#define FIFTYONEDEGREES_MUTEX_CREATE(m) m = CreateMutex(NULL,FALSE,NULL)
#else
#define FIFTYONEDEGREES_MUTEX_CREATE(m) fiftyoneDegreesMutexCreate(&m)
#endif

/**
* Frees the mutex at the pointer provided.
*/
#ifdef _MSC_VER
#define FIFTYONEDEGREES_MUTEX_CLOSE(m) if (m != NULL) { CloseHandle(m); }
#else
#define FIFTYONEDEGREES_MUTEX_CLOSE(m) fiftyoneDegreesMutexClose(&m)
#endif

/**
* Locks the mutex at the pointer provided.
*/
#ifdef _MSC_VER
#define FIFTYONEDEGREES_MUTEX_LOCK(m) WaitForSingleObject(*m, INFINITE)
#else
#define FIFTYONEDEGREES_MUTEX_LOCK(m) fiftyoneDegreesMutexLock(m)
#endif

/**
* Unlocks the mutex at the pointer provided.
*/
#ifdef _MSC_VER
#define FIFTYONEDEGREES_MUTEX_UNLOCK(m) ReleaseMutex(*m)
#else
#define FIFTYONEDEGREES_MUTEX_UNLOCK(m) fiftyoneDegreesMutexUnlock(m)
#endif

/**
 * Returns true if the signal is valid.
 */
#ifdef _MSC_VER
#define FIFTYONEDEGREES_MUTEX_VALID(m) (*m != NULL)
#else
#define FIFTYONEDEGREES_MUTEX_VALID(m) fiftyoneDegreesMutexValid(m)
#endif

/**
 * Creates a new thread with the following parameters:
 * t - pointer to FIFTYONEDEGREES_THREAD memory
 * m - the method to call when the thread runs
 * s - pointer to the state data to pass to the method
 */
#ifdef _MSC_VER
#define FIFTYONEDEGREES_THREAD_CREATE(t, m, s) t = \
	(FIFTYONEDEGREES_THREAD)CreateThread(NULL, 0, m, s, 0, NULL)
#else
#define FIFTYONEDEGREES_THREAD_CREATE(t, m, s) pthread_create(&t, NULL, m, s)
#endif

/**
 * Joins the thread provided to the current thread waiting
 * indefinitely for the operation to complete.
 * t - pointer to a previously created thread
 */
#ifdef _MSC_VER
#define FIFTYONEDEGREES_THREAD_JOIN(t) WaitForSingleObject(t, INFINITE)
#else
#define FIFTYONEDEGREES_THREAD_JOIN(t) pthread_join(t, NULL)
#endif

/**
 * Closes the thread passed to the macro.
 */
#ifdef _MSC_VER
#define FIFTYONEDEGREES_THREAD_CLOSE(t) CloseHandle(t)
#else
#define FIFTYONEDEGREES_THREAD_CLOSE(t) pthread_exit(NULL)
#endif

#ifdef _MSC_VER
#define FIFTYONEDEGREES_THREAD_EXIT ExitThread(0)
#else
#define FIFTYONEDEGREES_THREAD_EXIT pthread_exit(NULL)
#endif

#endif
#endif

#ifdef _MSC_VER
#define FIFTYONEDEGREES_INTERLOCK_INC(v) _InterlockedIncrement(v)
#else
#define FIFTYONEDEGREES_INTERLOCK_INC(v) __sync_fetch_and_add(v, 1)
#endif

#ifdef _MSC_VER
#define FIFTYONEDEGREES_INTERLOCK_DEC(v) _InterlockedDecrement(v)
#else
#define FIFTYONEDEGREES_INTERLOCK_DEC(v) __sync_fetch_and_add(v, -1)
#endif

#ifdef _MSC_VER
#define FIFTYONEDEGREES_INTERLOCK_EXCHANGE(d,e,c) \
	InterlockedCompareExchange(&d, e, c)
#else
#define FIFTYONEDEGREES_INTERLOCK_EXCHANGE(d,e,c) \
    __sync_val_compare_and_swap(&d,c,e)
#endif
