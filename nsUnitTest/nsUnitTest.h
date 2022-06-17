#pragma once

#include "nsString.h"



class nsTestObject
{
public:
	int I;
	float F;
	nsString S;


public:
	nsTestObject()
		: I(0)
		, F(0.0f)
	{
	}

};



namespace nsUnitTest
{
	extern void TestArray();
	extern void TestString();
	extern void TestMath();

};
