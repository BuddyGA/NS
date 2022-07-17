#include "nsObject.h"



NS_CLASS_BEGIN_NO_PARENT(nsObject)
	NS_CLASS_AddProperty(nsObject, nsString, Name, true)
NS_CLASS_END(nsObject)

nsObject::nsObject() noexcept
{
}


nsObject::nsObject(nsString name) noexcept
	: Name(name)
{
}


const nsClass* nsObject::GetClass() const
{
	return Class;
}


const nsPropertyList& nsObject::GetProperties() const
{
	return GetClass()->GetProperties();
}
