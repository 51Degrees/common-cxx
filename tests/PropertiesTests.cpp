#include "pch.h"
#include "StringCollectionTestBase.h"
extern "C" {
	#include "../properties.h"
}

// Property names
const char* p0 = "BP";
const char* p1 = "AProperty";
const char* p2 = "CPropertyWithAReallyQuiteLongName";

const char* testValues[] = {
	"Red",
	"Yellow",
	"Green",
	"Blue",
	"Brown",
	"Black",
	"White"
};

// Function used to return property names when the properties code 
// requests them
fiftyoneDegreesString* getProperty(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item)
{
	fiftyoneDegreesTestCollectionState *strings = (fiftyoneDegreesTestCollectionState*)state;
	fiftyoneDegreesString *result = NULL;
	strings->collection->get(strings->collection, strings->offsets[index], item);
	item->collection = strings->collection;
	return (fiftyoneDegreesString*)item->data.ptr;
}

// Class that sets up the properties structure. This stops us having to 
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
			getProperty,
			malloc,
			free);
	}
};

// Test that the property names are present at the expected indicies
TEST_F(PropertiesTest, PropertyIndicies) {
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(NULL);
	for (int i = 0; i < count; i++) {
		int reqIndex = fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, testValues[i]);
		const char *name = FIFTYONEDEGREES_STRING(
			fiftyoneDegreesPropertiesGetNameFromRequiredIndex(properties, reqIndex));
		EXPECT_STREQ(testValues[i], name);
	}
	fiftyoneDegreesPropertiesFree(properties);
}

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



//int result = fiftyoneDegreesPropertiesGetPropertyIndexFromName(properties, "AProperty");
//EXPECT_EQ(0, result);
//result = fiftyoneDegreesPropertiesGetPropertyIndexFromName(properties, "BP");
//EXPECT_EQ(1, result);
//result = fiftyoneDegreesPropertiesGetPropertyIndexFromName(properties, "CPropertyWithAReallyQuiteLongName");
//EXPECT_EQ(2, result);
