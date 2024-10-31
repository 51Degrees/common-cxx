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

#include "json.h"
#include "fiftyone.h"

// Add two characters to the buffer if space is available.
static void addTwo(fiftyoneDegreesJson* s, char a, char b) {
	StringBuilderAddChar(&s->builder, a);
	StringBuilderAddChar(&s->builder, b);
}

// Adds a string of characters escaping special characters.
static void addStringEscape(
	fiftyoneDegreesJson* s,
	const char* value,
	size_t valueLength) {
	for (size_t i = 0; i < valueLength; i++) {
		switch (value[i]) {
		case '\"':
			addTwo(s, '\\', '\"');
			break;
		case '\b':
			addTwo(s, '\\', 'b');
			break;
		case '\f':
			addTwo(s, '\\', 'f');
			break;
		case '\n':
			addTwo(s, '\\', 'n');
			break;
		case '\r':
			addTwo(s, '\\', 'r');
			break;
		case '\t':
			addTwo(s, '\\', 't');
			break;
		default:
			StringBuilderAddChar(&s->builder, value[i]);
			break;
		}
	}
}

// Adds a string including surrounding double quotes and escaping special 
// characters.
static void addString(
	fiftyoneDegreesJson* s,
	const char* value,
	size_t length) {
	StringBuilderAddChar(&s->builder, '\"');
	addStringEscape(s, value, length);
	StringBuilderAddChar(&s->builder, '\"');
}

// Adds a comma separator.
static void addSeparator(fiftyoneDegreesJson* s) {
	StringBuilderAddChar(&s->builder, ',');
}

void fiftyoneDegreesJsonDocumentStart(fiftyoneDegreesJson* s) {
	StringBuilderInit(&s->builder);
	StringBuilderAddChar(&s->builder, '{');
}

void fiftyoneDegreesJsonDocumentEnd(fiftyoneDegreesJson* s) {
	StringBuilderAddChar(&s->builder, '}');
	StringBuilderComplete(&s->builder);
}

void fiftyoneDegreesJsonPropertySeparator(fiftyoneDegreesJson* s) {
	addSeparator(s);
}

void fiftyoneDegreesJsonPropertyStart(fiftyoneDegreesJson* s) {
	fiftyoneDegreesString* name;
	fiftyoneDegreesCollectionItem stringItem;
	fiftyoneDegreesException* exception = s->exception;

	// Check that the property is populated.
	if (s->property == NULL) {
		FIFTYONE_DEGREES_EXCEPTION_SET(
			FIFTYONE_DEGREES_STATUS_NULL_POINTER)
		return;
	}

	// Get the property name as a string.
	fiftyoneDegreesDataReset(&stringItem.data);
	name = fiftyoneDegreesStringGet(
		s->strings,
		s->property->nameOffset,
		&stringItem,
		exception);
	if (name != NULL && FIFTYONE_DEGREES_EXCEPTION_OKAY) {

		// Add the property name to the JSON buffer considering whether 
		// it's a list or single value property.
		const char* value = FIFTYONE_DEGREES_STRING(name);
		addString(s, value, strlen(value));
		StringBuilderAddChar(&s->builder, ':');
		if (s->property->isList) {
			StringBuilderAddChar(&s->builder, '[');
		}

		// Release the string.
		FIFTYONE_DEGREES_COLLECTION_RELEASE(s->strings, &stringItem);
	}
}

void fiftyoneDegreesJsonPropertyEnd(fiftyoneDegreesJson* s) {
    if (s->property == NULL) {
        fiftyoneDegreesException* exception = s->exception;
        FIFTYONE_DEGREES_EXCEPTION_SET(
            FIFTYONE_DEGREES_STATUS_NULL_POINTER)
            return;
    }
	if (s->property->isList) {
		StringBuilderAddChar(&s->builder, ']');
	}
}

void fiftyoneDegreesJsonPropertyValues(fiftyoneDegreesJson* s) {
	const char* value;
	fiftyoneDegreesException* exception = s->exception;

	// Check that the values is populated.
	if (s->values == NULL) {
		FIFTYONE_DEGREES_EXCEPTION_SET(
			FIFTYONE_DEGREES_STATUS_NULL_POINTER)
			return;
	}

	for (uint32_t i = 0; i < s->values->count; i++) {
		if (i > 0) {
			addSeparator(s);
		}
		value = FIFTYONE_DEGREES_STRING(
			(fiftyoneDegreesString*)s->values->items[i].data.ptr);
		if (value != NULL) {
			addString(s, value, strlen(value));
		}
	}
}