/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "string.h"
#include "fiftyone.h"

static uint32_t getFinalStringSize(void *initial) {
	return (uint32_t)(sizeof(int16_t) + (*(int16_t*)initial));
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

void* fiftyoneDegreesStringRead(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t offset,
	fiftyoneDegreesData *data,
	fiftyoneDegreesException *exception) {
	int16_t length;
	return CollectionReadFileVariable(
		file, 
		data, 
		offset,
		&length, 
		sizeof(int16_t),
		getFinalStringSize,
		exception);
}

#endif

fiftyoneDegreesString* fiftyoneDegreesStringGet(
	fiftyoneDegreesCollection *strings,
	uint32_t offset, 
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception) {
	return (String*)strings->get(
		strings,
		offset,
		item,
		exception);
}

int fiftyoneDegreesStringCompare(const char *a, const char *b) {
	for (; *a != '\0' && *b != '\0'; a++, b++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0) {
			return d;
		}
	}
	if (*a == '\0' && *b != '\0') return -1;
	if (*a != '\0' && *b == '\0') return 1;
	assert(*a == '\0' && *b == '\0');
	return 0;
}

int fiftyoneDegreesStringCompareLength(
	char const *a, 
	char const *b, 
	size_t length) {
	size_t i;
	for (i = 0; i < length; a++, b++, i++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0) {
			return d;
		}
	}
	return 0;
}

char *fiftyoneDegreesStringSubString(const char *a, const char *b) {
	int d;
	const char *a1, *b1;
	for (; *a != '\0' && *b != '\0'; a++) {
		d = tolower(*a) - tolower(*b);
		if (d == 0) {
			a1 = a + 1;
			b1 = b + 1;
			for (; *a1 != '\0' && *b1 != '\0'; a1++, b1++) {
				d = tolower(*a1) - tolower(*b1);
				if (d != 0) {
					break;
				}
			}
			if (d == 0 && *b1 == '\0') {
				return (char *)a;
			}
		}
	}
	return NULL;
}

fiftyoneDegreesStringBuilder* fiftyoneDegreesStringBuilderInit(
	fiftyoneDegreesStringBuilder* builder) {
	builder->current = builder->ptr;
	builder->remaining = builder->length;
	builder->added = 0;
	builder->full = false;
	return builder;
}

fiftyoneDegreesStringBuilder* fiftyoneDegreesStringBuilderAddChar(
	fiftyoneDegreesStringBuilder* builder,
	char const value) {
	if (builder->remaining > 1) {
		*builder->current = value;
		builder->current++;
		builder->remaining--;
	}
	else {
		builder->full = true;
	}
	builder->added++;
	return builder;
}

fiftyoneDegreesStringBuilder* fiftyoneDegreesStringBuilderAddInteger(
	fiftyoneDegreesStringBuilder* builder,
	int const value) {
    // 64-bit INT_MIN is  -9,223,372,036,854,775,807 => 21 characters
	char temp[22];
	if (snprintf(temp, sizeof(temp), "%d", value) > 0) {
		StringBuilderAddChars(
			builder,
			temp,
			strlen(temp));
	}
	return builder;
}

fiftyoneDegreesStringBuilder* fiftyoneDegreesStringBuilderAddChars(
	fiftyoneDegreesStringBuilder* builder,
	char* const value,
	size_t const length) {
	assert(strlen(value) == length);
	if (length < builder->remaining &&
		memcpy(builder->current, value, length) == builder->current) {
		builder->remaining -= length;
		builder->current += length;
	}
	else {
		builder->full = true;
	}
	builder->added += length;
	return builder;
}

fiftyoneDegreesStringBuilder* fiftyoneDegreesStringBuilderComplete(
	fiftyoneDegreesStringBuilder* builder) {

	// Always ensures that the string is null terminated even if that means 
	// overwriting the last character to turn it into a null.
	if (builder->remaining >= 1) {
		*builder->current = '\0';
		builder->current++;
		builder->remaining--;
		builder->added++;
	}
	else {
        if (builder->ptr && builder->length > 0) {
            *(builder->ptr + builder->length - 1) = '\0';
        }
		builder->full = true;
	}
	return builder;
}
