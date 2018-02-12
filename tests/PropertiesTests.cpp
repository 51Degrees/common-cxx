#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
	#include "../properties.h"
}

// Property names
const char* testValues[] = {
	"Red",
	"Yellow",
	"Green",
	"Blue",
	"Brown",
	"Black",
	"White"
};

// Class that sets up the properties test structure. This stops us having to 
// do it multiple times.
class PropertiesTest: public StringCollectionTestBase
{
protected:
	fiftyoneDegreesTestCollectionState state;
	int count;

	void SetUp() {
		count = sizeof(testValues) / sizeof(const char*);
		state = buildState(testValues, count);
	}
	void TearDown() {
		freeState(&state);
	}

	fiftyoneDegreesPropertiesResults* BuildProperties(
		fiftyoneDegreesPropertiesRequired *required) {
		return fiftyoneDegreesPropertiesCreate(
			&state,
			count,
			required,
			getStringTestValue,
			malloc,
			free);
	}
};


/**
 * Check that all the properties are present as expected.
 */
TEST_F(PropertiesTest, AllProperties) {
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(NULL);
	for (int i = 0; i < count; i++) {
		int reqIndex = fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, testValues[i]);
		const char *name = FIFTYONEDEGREES_STRING(
			fiftyoneDegreesPropertiesGetNameFromRequiredIndex(properties, reqIndex));
		EXPECT_STREQ(testValues[i], name);
	}
	fiftyoneDegreesPropertiesFree(properties);
}

/**
* Check that passing a string list of required properties works as expected.
* Yellow is in the list of properties and required properties so should 
* have an index.
* Red is in the list of properties but not required properties so should
* return an index of -1.
* Beige is not in the list of properties but is in required properties so 
* should also return an index of -1.
*/
TEST_F(PropertiesTest, OneMissingProperty) {
	fiftyoneDegreesPropertiesRequired required;
	required.string = "Yellow,Beige";
	required.array = NULL;
	required.count = 0;
	required.existing = NULL;
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(&required);
	int reqIndex = fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Yellow");
	EXPECT_EQ(0, reqIndex);
	const char *name = FIFTYONEDEGREES_STRING(
		fiftyoneDegreesPropertiesGetNameFromRequiredIndex(properties, reqIndex));
	EXPECT_STREQ("Yellow", name);
	EXPECT_EQ(-1, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Beige"));
	EXPECT_EQ(-1, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Red"));
	fiftyoneDegreesPropertiesFree(properties);
}

/**
* Check that passing a string list of required properties works as expected.
* Yellow and Black are both in the list of properties and required properties 
* so should have index values.
* Since they are sorted alphabetically, Black should have index 0 and Yellow 
* index 1.
*/
TEST_F(PropertiesTest, StringTwoPropertiesOrdered) {
	fiftyoneDegreesPropertiesRequired required;
	required.string = "Yellow,Black";
	required.array = NULL;
	required.count = 0;
	required.existing = NULL;
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(&required);
	EXPECT_EQ(0, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Black"));
	EXPECT_EQ(1, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Yellow"));
	fiftyoneDegreesPropertiesFree(properties);
}

/**
* Check that passing a string list of required properties works as expected.
* Function should still work if there is a space after the comma (?)
*/
TEST_F(PropertiesTest, StringTwoPropertiesOrderedSpace) {
	fiftyoneDegreesPropertiesRequired required;
	required.string = "Yellow, Black";
	required.array = NULL;
	required.count = 0;
	required.existing = NULL;
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(&required);
	EXPECT_EQ(0, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Black"));
	EXPECT_EQ(1, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Yellow"));
	fiftyoneDegreesPropertiesFree(properties);
}

/**
* Check that passing an array of strings of required properties works as expected.
* Yellow and Black are both in the list of properties and required properties 
* so should have index values.
* Since they are sorted alphabetically, Black should have index 0 and Yellow 
* index 1.
*/
TEST_F(PropertiesTest, ArrayTwoPropertiesOrdered) {
	const char* tests[] = { "Yellow", "Black" };
	fiftyoneDegreesPropertiesRequired required;
	required.string = NULL;
	required.array = tests;
	required.count = sizeof(tests) / sizeof(const char*);
	required.existing = NULL;
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(&required);
	EXPECT_EQ(0, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Black"));
	EXPECT_EQ(1, fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, "Yellow"));
	fiftyoneDegreesPropertiesFree(properties);
}

