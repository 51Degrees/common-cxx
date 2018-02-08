#include "pch.h"
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

typedef struct fiftyone_degrees_test_properties_state {
	fiftyoneDegreesCollection *collection;
	uint32_t *offsets;
	void *data;
} fiftyoneDegreesTestPropertiesState;

// Function used to return property names when the properties code 
// requests them
fiftyoneDegreesString* getProperty(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item)
{
	fiftyoneDegreesTestPropertiesState *strings = (fiftyoneDegreesTestPropertiesState*)state;
	fiftyoneDegreesString *result = NULL;
	strings->collection->get(strings->collection, strings->offsets[index], item);
	item->collection = strings->collection;
	return (fiftyoneDegreesString*)item->data.ptr;
}

// Class that sets up the properties structure. This stops us having to 
// do it multiple times.
class PropertiesTest_SimpleSetup: public ::testing::Test
{
protected:
	fiftyoneDegreesTestPropertiesState state;
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

private:

	void freeState(fiftyoneDegreesTestPropertiesState *state) {
		free(state->offsets);
		free(state->data);
	}

	fiftyoneDegreesTestPropertiesState buildState(const char **values, int count) {
		fiftyoneDegreesTestPropertiesState state;
		int i;
		fiftyoneDegreesMemoryReader reader;
		size_t dataLength = 0;
		for (i = 0; i < count; i++) {
			dataLength += 2 + strlen(values[i]) + 1;
		}
		reader.length = dataLength + sizeof(uint32_t);
		state.data = malloc(reader.length);
		*(int32_t*)state.data = dataLength;
		state.offsets = (uint32_t*)malloc(count * sizeof(uint32_t));
		byte *start = ((byte*)state.data) + sizeof(uint32_t);
		reader.current = start;
		for (i = 0; i < count; i++) {
			fiftyoneDegreesString *string = (fiftyoneDegreesString*)reader.current;
			string->size = (int16_t)strlen(values[i]) + 1;
			strncpy(&string->value, values[i], string->size);
			state.offsets[i] = reader.current - start;
			reader.current += 2 + string->size;
		}
		reader.lastByte = reader.current;
		reader.current = (byte*)state.data;
		state.collection = fiftyoneDegreesCollectionCreateFromMemory(
			&reader,
			0,
			0,
			malloc);
		assert(state.collection->size == dataLength);
		return state;
	}
};

// Test that the property names are present at the expected indicies
TEST_F(PropertiesTest_SimpleSetup, PropertyIndicies) {
	fiftyoneDegreesPropertiesResults *properties = BuildProperties(NULL);
	for (int i = 0; i < count; i++) {
		int reqIndex = fiftyoneDegreesPropertiesGetRequiredPropertyIndexFromName(properties, testValues[i]);
		const char *name = FIFTYONEDEGREES_STRING(
			fiftyoneDegreesPropertiesGetNameFromRequiredIndex(properties, reqIndex));
		EXPECT_STREQ(testValues[i], name);
	}
	fiftyoneDegreesPropertiesFree(properties);
}

TEST_F(PropertiesTest_SimpleSetup, OneMissingProperty) {
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

TEST_F(PropertiesTest_SimpleSetup, StringTwoPropertiesOrdered) {
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

TEST_F(PropertiesTest_SimpleSetup, ArrayTwoPropertiesOrdered) {
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
