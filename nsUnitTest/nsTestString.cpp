#include "nsUnitTest.h"


static nsString TestString_Move()
{
	nsString abc = TEXT("abc");
	return abc;
}


static nsTArray<nsString> TestStringArray_Move()
{
	nsTArray<nsString> strings;
	strings.Add(TEXT("string_from_array_0"));
	strings.Add(TEXT("string_from_array_1"));
	strings.Add(TEXT("string_from_array_2"));
	strings.Add(strings[0].Substring(3, 8));

	return strings;
}


static nsString TestString_Substring(const nsString& string, int index, int length)
{
	return string.Substring(index, length);
}


static void TestString_Insert()
{
	nsString empty;
	empty.InsertAt(TEXT("AbcD"));
	NS_Validate(empty == TEXT("AbcD"));

	nsString testA = TEXT("_testA_");
	testA.InsertAt(TEXT("somethinG"), 0);
	NS_Validate(testA == TEXT("somethinG_testA_"));

	testA.InsertAt(TEXT("BBCCDD"), NS_STRING_MAX_LENGTH);
	NS_Validate(testA == TEXT("somethinG_testA_BBCCDD"));

}


static void TestString_Remove()
{
	nsString removeAll = TEXT("abcdefgh");
	removeAll.RemoveAtRange(0, 8);
	NS_Validate(removeAll.GetLength() == 0);

	nsString range = TEXT("remove_at_range");
	range.RemoveAtRange(0, 3);
	NS_Validate(range.GetLength() == 12);
	NS_Validate(range == TEXT("ove_at_range"));

	range.RemoveAtRange(3);
	NS_Validate(range.GetLength() == 3);
	NS_Validate(range == TEXT("ove"));

	range = TEXT("new_remove_at_range");
	NS_Validate(range.GetLength() == 19);
	range.RemoveAtRange(3, 5);
	NS_Validate(range.GetLength() == 14);
	NS_Validate(range == TEXT("newve_at_range"));

	range.RemoveAtRange(8, 6);
	NS_Validate(range.GetLength() == 8);
	NS_Validate(range == TEXT("newve_at"));
}


void nsUnitTest::TestString()
{
	nsString testEmpty = TEXT("");
	NS_Validate(testEmpty == TEXT(""));

	const nsString stringABC = TestString_Move();
	NS_Validate(stringABC == TEXT("abc"));

	nsTArray<nsString> stringArray;
	//stringArray.Reserve(8);
	stringArray = TestStringArray_Move();
	NS_Validate(stringArray.GetCount() == 4);
	NS_Validate(stringArray[0] == TEXT("string_from_array_0"));
	NS_Validate(stringArray[1] == TEXT("string_from_array_1"));
	NS_Validate(stringArray[2] == TEXT("string_from_array_2"));
	NS_Validate(stringArray[3] == TEXT("ing_from"));

	nsString stringTestSubstring(TEXT("string_for_substring_test"));
	nsString substringResult = TestString_Substring(stringTestSubstring, 2, 5);
	NS_Validate(substringResult.GetLength() == 5);
	NS_Validate(substringResult == TEXT("ring_"));

	TestString_Insert();
	TestString_Remove();

	// Test name
	{
		const nsName nameA = "actorcomponent";
		const nsName nameB = "ActorComponent";
		NS_Validate(nameA == nameB);
	}
}
