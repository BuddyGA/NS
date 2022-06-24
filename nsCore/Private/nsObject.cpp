#include "nsObject.h"



nsClass::nsClass(nsName name, const nsClass* parentClass) noexcept
	: Name(name)
	, ParentClass(parentClass)
{
	nsObjectManager::Get().RegisterObjectClass(this);
}


bool nsClass::IsSubclassOf(const nsClass* parentClass) const noexcept
{
	if (parentClass == nullptr)
	{
		return false;
	}

	if (this == parentClass)
	{
		return true;
	}

	const nsClass* thisParentClass = ParentClass;

	while (thisParentClass)
	{
		if (thisParentClass == parentClass)
		{
			return true;
		}

		thisParentClass = thisParentClass->ParentClass;
	}

	return false;
}



const nsClass* nsObject::Class = new nsClass("nsObject", nullptr);

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
	const int index = RegisteredObjectClasses.Find(name, 
		[](const nsClass* check, const nsName& name) 
		{ 
			return check->GetName() == name; 
		}
	);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		return nullptr;
	}

	return RegisteredObjectClasses[index];
}


nsTArray<const nsClass*> nsObjectManager::FindAllClasses(const nsClass* baseClass) const
{
	if (baseClass == nullptr)
	{
		return nsTArray<const nsClass*>();
	}

	nsTArray<const nsClass*> classes;
	classes.Reserve(8);

	for (int i = 0; i < RegisteredObjectClasses.GetCount(); ++i)
	{
		const nsClass* checkClass = RegisteredObjectClasses[i];

		if (checkClass->IsSubclassOf(baseClass))
		{
			classes.Add(checkClass);
		}
	}

	return classes;
}
