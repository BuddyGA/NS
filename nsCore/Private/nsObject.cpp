#include "nsObject.h"



NS_CLASS_BEGIN_NO_PARENT(nsObject)
	NS_CLASS_AddProperty(nsObject, nsName, Name, true)
NS_CLASS_END(nsObject)

nsObject::nsObject() noexcept
{
}


nsObject::nsObject(nsName name) noexcept
	: Name(name)
{
}


const nsClass* nsObject::GetClass() const
{
	return Class;
}


const nsPropertyList nsObject::GetProperties() const
{
	return GetClass()->GetProperties();
}
