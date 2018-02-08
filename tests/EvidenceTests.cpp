#include "pch.h"
#include "../evidence.h"



TEST(Evidence, Map_HttpHeaderPrefix) {
    fiftyoneDegreesEvidenceHeaderPrefix prefix = fiftyoneDegreesEvidenceMapPrefix("header");
    EXPECT_EQ((int)prefix, (int)FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING);
}

void assertStringHeaderAdded(
	fiftyoneDegreesEvidenceKeyValuePair *pair,
	const char *expectedField,
	const char *expectedValue) {
	EXPECT_EQ((int)pair->prefix, (int)FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING) <<
		L"Expected 'header' prefix.";
	EXPECT_TRUE(strcmp(pair->field, expectedField) == 0) <<
		L"Expected name '" << expectedField << "' not '" << pair->field << "'";
	EXPECT_TRUE(strcmp((const char*)pair->originalValue, expectedValue) == 0) <<
		L"Expected value '" << expectedValue << "' not '" << pair->originalValue << "'";
}

TEST(Evidence, Add_SingleString) {

	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	assertStringHeaderAdded(&evidence->items[0], "some-header-name", "some-header-value");
}

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
int onMatchIterateString(void *state, fiftyoneDegreesEvidenceKeyValuePair *pair)
{
	EXPECT_TRUE(strcmp((const char*)pair->originalValue, (const char*)pair->parsedValue) == 0) <<
		L"Expected parsed value to be '" << (const char*)pair->originalValue << "' not '" << 
		(const char*)pair->parsedValue << "'";
	return 0;
}

TEST(Evidence, Iterate_String)
{
	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	evidence->items[0].parsedValue == NULL;

	fiftyoneDegreesEvidenceIterate(evidence,
		NULL,
		matchIterateSomeHeader,
		onMatchIterateString);
}

char* parsedValue = "already-parsed";
int onMatchIterateStringAlreadyParsed(void *state, fiftyoneDegreesEvidenceKeyValuePair *pair)
{
	EXPECT_TRUE(strcmp(parsedValue, (const char*)pair->parsedValue) == 0) <<
		L"Expected parsed value to be '" << parsedValue << "' not '" <<
		(const char*)pair->parsedValue << "'";
	return 0;
}
TEST(Evidence, Iterate_String_AlreadyParsed)
{
	fiftyoneDegreesEvidenceCollection *evidence = fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"some-header-name",
		"some-header-value");
	evidence->items[0].parsedValue == parsedValue;

	fiftyoneDegreesEvidenceIterate(evidence,
		NULL,
		matchIterateSomeHeader,
		onMatchIterateStringAlreadyParsed);
}
