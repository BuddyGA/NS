#pragma once

#include "nsString.h"
#include "nsMath.h"


struct nsType
{
	const char* Name;
	int Size;
};


constexpr nsType TypeVoid({ "void", 0 });
constexpr nsType TypeBool({ "bool", sizeof(bool) });
constexpr nsType TypeInt({ "int", sizeof(int) });
constexpr nsType TypeFloat({ "float", sizeof(float) });
constexpr nsType TypeString({ "nsString", sizeof(nsString) });
constexpr nsType TypeName({ "nsName", sizeof(nsName) });
constexpr nsType TypeVector2({ "nsVector2", sizeof(nsVector2) });
constexpr nsType TypeVector3({ "nsVector3", sizeof(nsVector3) });



class NS_CORE_API nsProperty
{
	NS_DECLARE_NOCOPY(nsProperty)

private:
	const char* Name;
	nsType Type;
	int Offset;


public:
	nsProperty(const char* name, const nsType& type, int offset) noexcept
		: Name(name)
		, Type(type)
		, Offset(offset)
	{
	}

};
