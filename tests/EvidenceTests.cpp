#include "pch.h"
#include "../evidence.h"


void assertStringHeaderAdded(
	fiftyoneDegreesEvidenceKeyValuePair *pair,
	const char *expectedField,
	const char *expectedValue) {
	EXPECT_EQ((int)pair->prefix, (int)FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING) <<
		L"Expected 'header' prefix.";
	EXPECT_STREQ(pair->field, expectedField) <<
		L"Expected name '" << expectedField << "' not '" << pair->field << "'";
	EXPECT_TRUE(strcmp((const char*)pair->originalValue, expectedValue) == 0) <<
		L"Expected value '" << expectedValue << "' not '" << pair->originalValue << "'";
}

/*
Check that a single string can be added to evidence.
*/
TEST(Evidence, Add_SingleString) {

	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	assertStringHeaderAdded(&evidence->items[0], "some-header-name", "some-header-value");
}

/*
Check that multiple strings can be added to evidence.
*/
TEST(Evidence, Add_MultipleStrings)
{
	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(2, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-other-header-name",
		"some-header-value");
	assertStringHeaderAdded(&evidence->items[0], "some-header-name", "some-header-value");
	assertStringHeaderAdded(&evidence->items[1], "some-other-header-name", "some-header-value");
}

bool matchIterateSomeHeader(void *state, fiftyoneDegreesEvidenceKeyValuePair *pair)
{
	if (pair->prefix == (int)FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING &&
		strcmp((const char*)pair->field, "some-header-name") == 0) {
		return true;
	}
	return false;
}
void onMatchIterateString(void *state, fiftyoneDegreesEvidenceKeyValuePair *pair)
{
	EXPECT_TRUE(strcmp((const char*)pair->originalValue, (const char*)pair->parsedValue) == 0) <<
		L"Expected parsed value to be '" << (const char*)pair->originalValue << "' not '" << 
		(const char*)pair->parsedValue << "'";
}
/*
Check that the parsed version of a string evidence value will be the same string.
*/
TEST(Evidence, Iterate_String)
{
	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	evidence->items[0].parsedValue = NULL;

	fiftyoneDegreesEvidenceIterate(evidence,
		NULL,
		matchIterateSomeHeader,
		onMatchIterateString);
}


char* parsedValue = "already-parsed";
void onMatchIterateStringAlreadyParsed(void *state, fiftyoneDegreesEvidenceKeyValuePair *pair)
{
	EXPECT_TRUE(strcmp(parsedValue, (const char*)pair->parsedValue) == 0) <<
		L"Expected parsed value to be '" << parsedValue << "' not '" <<
		(const char*)pair->parsedValue << "'";
}
/*
Check that an evidence value is not parsed again if it has already been parsed.
*/
TEST(Evidence, Iterate_String_AlreadyParsed)
{
	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	evidence->items[0].parsedValue = parsedValue;

	fiftyoneDegreesEvidenceIterate(evidence,
		NULL,
		matchIterateSomeHeader,
		onMatchIterateStringAlreadyParsed);
}
TEST(Evidence, Parse_MultipleStringSingleImportant)
{

	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(2, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-other-header-name",
		"some-header-value");

	fiftyoneDegreesEvidenceIterate(evidence,
		NULL,
		matchIterateSomeHeader,
		onMatchIterateString);

	EXPECT_STREQ((const char*)evidence->items[0].originalValue,
		(const char*)evidence->items[0].parsedValue) <<
		L"Expected name '" << (const char*)evidence->items[0].originalValue << "' not '" <<
		(const char*)evidence->items[0].parsedValue << "'";
	EXPECT_EQ(0, (int)evidence->items[1].parsedValue) <<
		L"Expected '" << (const char*)evidence->items[1].originalValue << "' not to be parsed";
}

