#include "nsObject.h"



nsClass::nsClass(nsName name, const nsClass* parentClass) noexcept
	: Name(name)
	, ParentClass(parentClass)
{
	nsObjectManager::Get().RegisterObjectClass(this);
}



const nsClass* nsObject::Class = new nsClass("Object", nullptr);

nsObject::nsObject() noexcept
{
}


nsObject::nsObject(nsName name) noexcept
	: Name(name)
{
}


const nsClass* nsObject::GetClass() const noexcept
{
	return Class;
}



nsObjectManager::nsObjectManager() noexcept
{
	RegisteredObjectClasses.Reserve(64);
}


void nsObjectManager::RegisterObjectClass(const nsClass* objectClass)
{
	RegisteredObjectClasses.AddUnique(objectClass);
}


const nsClass* nsObjectManager::FindObjectClass(const nsName& name) const
{
	const int index = RegisteredObjectClasses.Find(name, [](const nsClass* check, const nsName& name) { return check->GetName() == name; });

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		return nullptr;
	}

	return RegisteredObjectClasses[index];
}
