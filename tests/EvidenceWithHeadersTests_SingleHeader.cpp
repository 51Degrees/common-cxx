#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
#include "../evidence.h"
#include "../headers.h"
}

// Header names
const char* testHeaders_Single[] = {
	"Red",
};

// Class that sets up the headers test structure when there are multiple headers. 
// This stops us having to  do it multiple times.
class EvidenceWithHeadersTest_SingleHeader : public StringCollectionTestBase
{
protected:
	fiftyoneDegreesTestCollectionState state;
	int count;

	void SetUp() {
		count = sizeof(testHeaders_Single) / sizeof(const char*);
		state = buildState(testHeaders_Single, count);
	}
	void TearDown() {
		freeState(&state);
	}

	fiftyoneDegreesHeaders* BuildHeaders() {
		return fiftyoneDegreesHeadersCreate(
			&state,
			count,
			false,
			getStringTestValue,
			malloc,
			free);
	}
};


static bool isHttpHeader(
	void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	return pair->prefix == FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING &&
		fiftyoneDegreesHeaderGetIndex(
		(fiftyoneDegreesHeaders*)state,
			pair->field,
			strlen(pair->field)) >= 0;
}


// These tests use a naming convention suffix of *h_*e_*m.
// This corresponds to the number of possible headers, 
// evidence and matches between headers and evidence respectively.
// 
// The * can be:
// s = single
// m = multiple
// n = none
//
// e.g. sh_me_sm 
// Means that there is only one header expected, multiple evidence
// is supplied and one is expected to match.



//------------------------------------------------------------------
// Check that the intersection of a single piece of evidence and 
// multiple expected headers matches the expected item.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_sh_se_sm[2];
int intersection_sh_se_sm_count = 0;
int evidenceHeaderIntersection_sh_se_sm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_sh_se_sm[intersection_sh_se_sm_count] = *pair;
	intersection_sh_se_sm_count++;
	return intersection_sh_se_sm_count;
}

TEST_F(EvidenceWithHeadersTest_SingleHeader, Intersection_sh_se_sm) {
	fiftyoneDegreesEvidenceCollection *evidence =
		fiftyoneDegreesEvidenceCreate(1, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Red",
		"Value");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		BuildHeaders(),
		isHttpHeader,
		evidenceHeaderIntersection_sh_se_sm);

	ASSERT_EQ(1, result);
	ASSERT_STREQ("Red", intersection_sh_se_sm[0].field);
	ASSERT_STREQ("Value", (char*)intersection_sh_se_sm[0].originalValue);
}


//------------------------------------------------------------------
// Check that the intersection of multiple evidence and multiple
// expected headers matches the expected items when there
// are multiple matches.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_sh_me_sm[2];
int intersection_multiple_sh_me_mm_count = 0;
int evidenceHeaderIntersection_sh_me_mm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_sh_me_sm[intersection_multiple_sh_me_mm_count] = *pair;
	intersection_multiple_sh_me_mm_count++;
	return intersection_multiple_sh_me_mm_count;
}
TEST_F(EvidenceWithHeadersTest_SingleHeader, Intersection_sh_me_sm) {
	fiftyoneDegreesEvidenceCollection *evidence =
		fiftyoneDegreesEvidenceCreate(2, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Black",
		"Value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Red",
		"Value2");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		BuildHeaders(),
		isHttpHeader,
		evidenceHeaderIntersection_sh_me_mm);

	ASSERT_EQ(1, result);
	EXPECT_STREQ("Red", intersection_sh_me_sm[0].field);
	EXPECT_STREQ("Value2", (char*)intersection_sh_me_sm[0].originalValue);
}

//------------------------------------------------------------------
// Check that the intersection of multiple evidence and multiple
// expected headers matches the expected items when there are no 
// matches.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_sh_me_nm[2];
int intersection_sh_me_nm_count = 0;
int evidenceHeaderIntersection_sh_me_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_sh_me_nm[intersection_sh_me_nm_count] = *pair;
	intersection_sh_me_nm_count++;
	return intersection_sh_me_nm_count;
}
TEST_F(EvidenceWithHeadersTest_SingleHeader, Intersection_sh_me_nm) {
	fiftyoneDegreesEvidenceCollection *evidence =
		fiftyoneDegreesEvidenceCreate(2, malloc, free);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Lilac",
		"Value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONEDEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Indigo",
		"Value2");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		BuildHeaders(),
		isHttpHeader,
		evidenceHeaderIntersection_sh_me_nm);

	ASSERT_EQ(0, result);
}


//------------------------------------------------------------------
// Check that the intersection of no evidence and multiple
// expected headers functions as expected
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_sh_ne_nm[2];
int intersection_sh_ne_nm_count = 0;
int evidenceHeaderIntersection_sh_ne_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_sh_ne_nm[intersection_sh_ne_nm_count] = *pair;
	intersection_sh_ne_nm_count++;
	return intersection_sh_ne_nm_count;
}
TEST_F(EvidenceWithHeadersTest_SingleHeader, Intersection_sh_ne_nm) {
	fiftyoneDegreesEvidenceCollection *evidence =
		fiftyoneDegreesEvidenceCreate(1, malloc, free);

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		BuildHeaders(),
		isHttpHeader,
		evidenceHeaderIntersection_sh_ne_nm);

	ASSERT_EQ(0, result);
}