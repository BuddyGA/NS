#include "nsUnitTest.h"


static nsTArray<nsTestObject> TestArray_Move()
{
	nsTArray<nsTestObject> testObjects;
	testObjects.Resize(3);

	nsTestObject& obj0 = testObjects[0];
	obj0.I = 110011;
	obj0.F = 11.0011f;
	obj0.S = "test_move_0";

	nsTestObject& obj1 = testObjects[1];
	obj1.I = 220022;
	obj1.F = 22.0022f;
	obj1.S = "test_move_1";

	return testObjects;
}


static void TestArray_Insert()
{
	nsTArray<int> test = { 0, 1, 2, 3, 4, 5 };

	test.InsertAt(9, 3);
	NS_Validate(test.GetCount() == 7);
	NS_Validate(
		test[0] == 0 &&
		test[1] == 1 &&
		test[2] == 2 &&
		test[3] == 9 &&
		test[4] == 3 &&
		test[5] == 4 &&
		test[6] == 5
	);

	nsTArray<int> insert = { 9, 9, 9 };
	test.InsertAt(insert, 2);

	NS_Validate(test.GetCount() == 10);
	NS_Validate(
		test[0] == 0 &&
		test[1] == 1 &&
		test[2] == 9 &&
		test[3] == 9 &&
		test[4] == 9 &&
		test[5] == 2 &&
		test[6] == 9 &&
		test[7] == 3 &&
		test[8] == 4 &&
		test[9] == 5
	);
}


static void TestArray_Remove()
{
	{
		nsTArray<int> testAt = { 0, 1, 2, 3, 4, 5 };
		testAt.RemoveAt(3);
		NS_Validate(testAt.GetCount() == 5);
		NS_Validate(testAt[0] == 0);
		NS_Validate(testAt[1] == 1);
		NS_Validate(testAt[2] == 2);
		NS_Validate(testAt[3] == 4);
		NS_Validate(testAt[4] == 5);

		testAt.RemoveAt(0);
		NS_Validate(testAt.GetCount() == 4);
		NS_Validate(testAt[0] == 1);
		NS_Validate(testAt[1] == 2);
		NS_Validate(testAt[2] == 4);
		NS_Validate(testAt[3] == 5);

		testAt.RemoveAt(NS_ARRAY_INDEX_LAST);
		NS_Validate(testAt.GetCount() == 3);
		NS_Validate(testAt[0] == 1);
		NS_Validate(testAt[1] == 2);
		NS_Validate(testAt[2] == 4);
	}
	
	{
		nsTArray<int> removeAll = { 0, 1, 2, 3, 4, 5 };

		removeAll.RemoveAtRange(0, removeAll.GetCount());
		NS_Validate(removeAll.GetCount() == 0);

		nsTArray<int> range = { 0, 1, 2, 3, 4, 5 };
		range.RemoveAtRange(0, 3);
		NS_Validate(range.GetCount() == 3);
		NS_Validate(range[0] == 3);
		NS_Validate(range[1] == 4);
		NS_Validate(range[2] == 5);

		range.RemoveAtRange(1, 2);
		NS_Validate(range.GetCount() == 1);
		NS_Validate(range[0] == 3);

		range = { 0, 1, 2, 3, 4, 5 };
		NS_Validate(range.GetCount() == 6);
		NS_Validate(range[0] == 0);
		NS_Validate(range[1] == 1);
		NS_Validate(range[2] == 2);
		NS_Validate(range[3] == 3);
		NS_Validate(range[4] == 4);
		NS_Validate(range[5] == 5);

		range.RemoveAtRange(2, 2);
		NS_Validate(range.GetCount() == 4);
		NS_Validate(range[0] == 0);
		NS_Validate(range[1] == 1);
		NS_Validate(range[2] == 4);
		NS_Validate(range[3] == 5);

		range.RemoveAtRange(2, 2);
		NS_Validate(range.GetCount() == 2);
		NS_Validate(range[0] == 0);
		NS_Validate(range[1] == 1);

		range = { 0, 1, 2, 3, 4, 5 };
		range.RemoveAtRange(2);
		NS_Validate(range.GetCount() == 2);
		NS_Validate(range[0] == 0);
		NS_Validate(range[1] == 1);
	}

}


void nsUnitTest::TestArray()
{
	// Allocate big bytes
	{
		nsTArray<uint8> bigBytes(NS_MEMORY_SIZE_MiB(256));
		NS_Validate(bigBytes.GetCount() == NS_MEMORY_SIZE_MiB(256));
	}

	nsTArray<nsTestObject> TestObjects;
	NS_Validate(TestObjects.GetCount() == 0);

	TestObjects.Resize(4);
	NS_Validate(TestObjects.GetCount() == 4);

	nsTestObject& obj0 = TestObjects[0];
	obj0.I = 11111;
	obj0.F = 11.11f;
	obj0.S = "TestObject_0";

	nsTestObject& obj1 = TestObjects[1];
	obj1.I = 22222;
	obj1.F = 22.22f;
	obj1.S = "TestObject_1";

	nsTestObject& obj2 = TestObjects[2];
	obj2.I = 33333;
	obj2.F = 33.33f;
	obj2.S = "TestObject_2";

	nsTestObject& obj3 = TestObjects[3];
	obj3.I = 44444;
	obj3.F = 44.44f;
	obj3.S = "TestObject_3";

	{
		nsTestObject add;
		add.I = 55555;
		add.F = 55.55f;
		add.S = "TestObject_4";

		TestObjects.Add(add);
	}
	NS_Validate(TestObjects[4].I == 55555);
	NS_Validate(TestObjects[4].F == 55.55f);
	NS_Validate(TestObjects[4].S == "TestObject_4");


	// Move
	const nsTArray<nsTestObject> testObjectMove = TestArray_Move();
	NS_Validate(testObjectMove.GetCount() == 3);
	NS_Validate(testObjectMove[0].I == 110011);
	NS_Validate(testObjectMove[0].F == 11.0011f);
	NS_Validate(testObjectMove[0].S == "test_move_0");
	NS_Validate(testObjectMove[1].I == 220022);
	NS_Validate(testObjectMove[1].F == 22.0022f);
	NS_Validate(testObjectMove[1].S == "test_move_1");


	// Find
	const int expectedFindIndex = 2;
	int foundAtIndex = TestObjects.Find(33333, [](const nsTestObject& value, int compare) { return value.I == compare; });
	NS_Validate(foundAtIndex == expectedFindIndex);

	foundAtIndex = TestObjects.FindLast(33333, [](const nsTestObject& value, int compare) { return value.I == compare; });
	NS_Validate(foundAtIndex == expectedFindIndex);

	TestArray_Insert();

	TestArray_Remove();
}
