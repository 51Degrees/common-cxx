#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
#include "../headers.h"
}


// Class that sets up the headers test structure from a string array.
class HeadersTests : public StringCollectionTestBase
{
protected:
	fiftyoneDegreesTestCollectionState state;
	int count;

	void SetUp() {
	}
	void TearDown() {
		freeState(&state);
	}

	fiftyoneDegreesHeaders* BuildHeaders(
		const char** headersList,
		int headersCount,
		bool useUpperPrefixedHeaders) {
		count = headersCount;

		state = buildState(headersList, count);

		return fiftyoneDegreesHeadersCreate(
			&state,
			count,
			useUpperPrefixedHeaders,
			getStringTestValue,
			malloc,
			free);
	}
};

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing a single header
// ----------------------------------------------------------------------
const char* testHeaders_Single[] = {
	"Red"
};

TEST_F(HeadersTests, Single) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_Single,
		sizeof(testHeaders_Single) / sizeof(const char*),
		false);

	ASSERT_EQ(1, headers->unique.count);
	fiftyoneDegreesString* str = (fiftyoneDegreesString*)headers->unique.items[0].data.ptr;
	EXPECT_STREQ("Red", FIFTYONEDEGREES_STRING(str));
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing multiple headers
// ----------------------------------------------------------------------
const char* testHeaders_Multiple[] = {
	"Red",
	"Green",
	"Blue",
	"Yellow",
};

TEST_F(HeadersTests, Multiple) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_Multiple,
		sizeof(testHeaders_Multiple) / sizeof(const char*),
		false);

	ASSERT_EQ(4, headers->unique.count);
	EXPECT_STREQ("Red", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Green", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
	EXPECT_STREQ("Blue", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[2].data.ptr));
	EXPECT_STREQ("Yellow", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[3].data.ptr));
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing a single duplicate
// ----------------------------------------------------------------------
const char* testHeaders_SingleDuplicate[] = {
	"Red",
	"Red",
};

TEST_F(HeadersTests, SingleDuplicate) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_SingleDuplicate,
		sizeof(testHeaders_SingleDuplicate) / sizeof(const char*),
		false);

	ASSERT_EQ(1, headers->unique.count);
	EXPECT_STREQ("Red", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
}


// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing multiple duplicates
// ----------------------------------------------------------------------
const char* testHeaders_MultipleDuplicate[] = {
	"Green",
	"Red",
	"Red",
	"Black",
	"Green"
};

TEST_F(HeadersTests, MultipleDuplicate) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_MultipleDuplicate,
		sizeof(testHeaders_MultipleDuplicate) / sizeof(const char*),
		false);

	ASSERT_EQ(3, headers->unique.count);
	EXPECT_STREQ("Green", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Red", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
	EXPECT_STREQ("Black", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[2].data.ptr));
}


// ----------------------------------------------------------------------
// Check that header collection creation works properly when one of
// the headers is an empty string
// ----------------------------------------------------------------------
const char* testHeaders_EmptyString[] = {
	"Green",
	"",
	"Black",
};

TEST_F(HeadersTests, EmptyString) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_EmptyString,
		sizeof(testHeaders_EmptyString) / sizeof(const char*),
		false);

	ASSERT_EQ(2, headers->unique.count);
	EXPECT_STREQ("Green", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Black", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when one of
// the headers is NULL
// ----------------------------------------------------------------------
const char* testHeaders_NullString[] = {
	"Green",
	NULL,
	"Black",
};

TEST_F(HeadersTests, NullString) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_NullString,
		sizeof(testHeaders_NullString) / sizeof(const char*),
		false);

	ASSERT_EQ(2, headers->unique.count);
	EXPECT_STREQ("Green", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Black", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when two of
// the headers are the same text but different case.
// ----------------------------------------------------------------------
const char* testHeaders_Case[] = {
	"Green",
	"green",
	"Black",
};

TEST_F(HeadersTests, CheckCase) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_Case,
		sizeof(testHeaders_Case) / sizeof(const char*),
		false);

	ASSERT_EQ(2, headers->unique.count);
	EXPECT_STREQ("Green", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Black", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
}


// ----------------------------------------------------------------------
// Check that header collection creation works correctly when the
// 'useUpperPrefixedHeaders' option is enabled.
// ----------------------------------------------------------------------
const char* testHeaders_HttpPrefix[] = {
	"HTTP_Red",
	"Black",
};

TEST_F(HeadersTests, HttpPrefix) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_HttpPrefix,
		sizeof(testHeaders_HttpPrefix) / sizeof(const char*),
		true);

	ASSERT_EQ(2, headers->unique.count);
	EXPECT_STREQ("Red", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[0].data.ptr));
	EXPECT_STREQ("Black", FIFTYONEDEGREES_STRING(
		(fiftyoneDegreesString*)headers->unique.items[1].data.ptr));
}

// ----------------------------------------------------------------------
// Check that header collection creation works correctly when a 
// collection with no headers is passed
// ----------------------------------------------------------------------
const char** testHeaders_None = new const char*[0];

TEST_F(HeadersTests, None) {
	fiftyoneDegreesHeaders* headers = BuildHeaders(
		testHeaders_None,
		sizeof(testHeaders_None) / sizeof(const char*),
		false);

	ASSERT_EQ(0, headers->unique.count);
}