#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
#include "../evidence.h"
#include "../headers.h"
}

// Header names
const char** testEvidenceHeaders_None = new const char*[0];

// Class that sets up the headers test structure when there are no headers. 
// This stops us having to do it multiple times.
class EvidenceWithHeadersTest_NoHeader : public StringCollectionTestBase
{
protected:
	fiftyoneDegreesTestCollectionState state;
	int count;

	void SetUp() {
		count = 0;
		state = buildState(testEvidenceHeaders_None, count);
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
// no expected headers works as expected.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_nh_se_nm[2];
int intersection_nh_se_nm_count = 0;
void evidenceHeaderIntersection_nh_se_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_nh_se_nm[intersection_nh_se_nm_count] = *pair;
	intersection_nh_se_nm_count++;
}

TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_se_nm) {
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
		evidenceHeaderIntersection_nh_se_nm);

	ASSERT_EQ(0, result);
}


//------------------------------------------------------------------
// Check that the intersection of multiple evidence and no
// expected headers matches the expected items.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_nh_me_nm[2];
int intersection_multiple_nh_me_mm_count = 0;
void evidenceHeaderIntersection_nh_me_mm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_nh_me_nm[intersection_multiple_nh_me_mm_count] = *pair;
	intersection_multiple_nh_me_mm_count++;
}
TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_me_nm) {
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
		evidenceHeaderIntersection_nh_me_mm);

	ASSERT_EQ(0, result);
}

//------------------------------------------------------------------
// Check that the intersection of no evidence and no
// expected headers functions as expected
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_nh_ne_nm[2];
int intersection_nh_ne_nm_count = 0;
void evidenceHeaderIntersection_nh_ne_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_nh_ne_nm[intersection_nh_ne_nm_count] = *pair;
	intersection_nh_ne_nm_count++;
}
TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_ne_nm) {
	fiftyoneDegreesEvidenceCollection *evidence =
		fiftyoneDegreesEvidenceCreate(1, malloc, free);

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		BuildHeaders(),
		isHttpHeader,
		evidenceHeaderIntersection_nh_ne_nm);

	ASSERT_EQ(0, result);
}