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
 * Implements a pool of file handles for use within multi-threaded environments
 * where the overhead of opening and closing a file handle for each thread
 * would be too great. Primarily used to load collection items from file with 
 * file based collections or where a cache is used.
 *
 * The fiftyoneDegreesFileReaderInit method is used to initialise a pointer to 
 * a fiftyoneDegreesFileReader. A concurrency value is provided to indicate the
 * maximum number of threads that will be in operational. If this value is 
 * lower than the actual number of threads the stack can be exhausted which is 
 * not supported. The concurrency value must always be the same or greater than
 * the number of threads. When compiled in single threaded operation a pool is 
 * not strictly required and the implementation maintains a simple stack for 
 * consistency of interface and to minimise divergent code.
 *
 * Handles are retrieved from the pool via the fiftyoneDegreesFileHandleGet 
 * method. The handle MUST be returned with the 
 * fiftyoneDegreesFileHandleRelease method when it is finished with. The handle
 * will always be open and ready for read only operation. The position of the
 * handle within the source file cannot be assumed.
 *
 * The handles are closed when the reader is released via the
 * fiftyoneDegreesFileReaderRelease method. Any memory allocated by the 
 * implementation for the stack is freed.
 *
 * To improve performance in multi-threaded operation a non locking stack is 
 * used where a Compare and Swap (CAS) atomic operation is used to pop and push 
 * handles on and off the stack. The design was adapted from the following 
 * article (http://nullprogram.com/blog/2014/09/02/) which explains some of the 
 * challenges involved including the ABA problem 
 * (https://en.wikipedia.org/wiki/ABA_problem). It is for this reason the head 
 * structure is implemented as a union between the values and the exchange
 * integer. Pointers are not used as the address space for the stack is 
 * continuous and always very small compared to the total addressable memory 
 * space.
 */

#ifndef FIFTYONEDEGREES_FILE_H_INCLUDED
#define FIFTYONEDEGREES_FILE_H_INCLUDED

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include "data.h"
#ifndef FIFTYONEDEGREES_NO_THREADING
#include "threading.h"
#endif

typedef enum e_fiftyoneDegreesFileOpenStatus {
	FIFTYONEDEGREES_FILE_OPEN_STATUS_SUCCESS, /* All okay */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_FILE_NOT_FOUND, /* The data file couldn't
													 be found */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_TOO_MANY_OPEN_FILES, /* Too many files are 
														 open */
	FIFTYONEDEGREES_FILE_OPEN_STATUS_FAILED /* General failure */
} fiftyoneDegreesFileOpenStatus;

typedef struct fiftyonedegrees_file_handle_t fiftyoneDegreesFileHandle;
typedef struct fiftyonedegrees_file_pool_t fiftyoneDegreesFilePool;

/**
 * File handle node in the stack of handles.
 */
typedef struct fiftyonedegrees_file_handle_t {
	FILE *file; /* Open read handle to the source data file. */
	uint16_t next; /* The next handle in the list */
	fiftyoneDegreesFilePool *pool; /* Reader the handle belongs to */
} fiftyoneDegreesFileHandle;

/**
 * The head of the stack used for pop and push operations.
 */
typedef union fiftyonedegrees_file_head_t {
	long exchange; /* Number used in the compare exchange operation */
	struct {
		uint16_t index; /* Index of the item in the linked list */
		uint16_t aba; /* ABA value used to ensure proper operation */
	} values;
} fiftyoneDegreesFileHead;

/**
 * Stack of handles used to read data from a single source file.
 */
 typedef struct fiftyonedegrees_file_pool_t {
	fiftyoneDegreesFileHandle *stack; /* Pointer to the memory used by the 
									  stack */
	fiftyoneDegreesFileHead head; /* Head of the stack */
	long length; /* Length of the file in bytes. */
	uint16_t count; /* Number of items that stack can hold */
} fiftyoneDegreesFilePool;

 /**
 * \ingroup FiftyOneDegreesFile
 * Releases the file handles contained in the pool and frees any internal
 * memory used by the pool. Does not free the memory pointed to by pool.
 * @param pool pointer to the stack of file handles to be release
 * @param free method to use to free the memory
 */
void fiftyoneDegreesFilePoolRelease(
	fiftyoneDegreesFilePool* pool, 
	void(*free)(void*));

/**
 * \ingroup FiftyOneDegreesFile
 * Opens the file path provided placing the file handle in the handle
 * parameter.
 * @param fileName full path to the file to open
 * @param handle to be associated with the open file
 * @return the result of the open operation
*/
fiftyoneDegreesFileOpenStatus fiftyoneDegreesFileOpen(
	const char* fileName,
	FILE** handle);

/**
 * \ingroup FiftyOneDegreesFile
 * Initialises the pool with a stack of open read only file handles all 
 * associated with the file name. The concurrency parameter determines the 
 * number of items in the stack.
 * @param pool to be initialised
 * @param fileName full path to the file to open
 * @param concurrency number of items in the stack
 * @param malloc method to allocate memory for the stack
 * @param free method to use to free the memory
 * @return the result of the open operation
*/
fiftyoneDegreesFileOpenStatus fiftyoneDegreesFilePoolInit(
	fiftyoneDegreesFilePool *pool,
	const char *fileName,
	uint16_t concurrency,
	void*(*malloc)(size_t __size),
	void(*free)(void*));

/**
 * \ingroup FiftyOneDegreesFile
 * Retrieves a read only open file handle from the pool. The handle retrieve
 * must be returned to the pool using fiftyoneDegreesFileHandleGet and must not 
 * be freed or closed directly.
 * @param pool to retrieve the file handle from
 * @return a read only open file handle
*/
fiftyoneDegreesFileHandle* fiftyoneDegreesFileHandleGet(
	fiftyoneDegreesFilePool *pool);

/**
 *\ingroup FiftyOneDegreesFile
 * Returns a handle previously retrieved via fiftyoneDegreesFileHandleGet back
 * to the pool.
 * @param handle to be returned to the pool
 */
void fiftyoneDegreesFileHandleRelease(fiftyoneDegreesFileHandle* handle);

#endif