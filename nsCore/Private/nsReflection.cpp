#include "nsReflection.h"



nsClass::nsClass(nsName name, const nsClass* parentClass) noexcept
	: Name(name)
	, ParentClass(parentClass)
	, bIsAbstract(false)
{
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


const nsPropertyList& nsClass::GetProperties() const noexcept
{
	return Properties;
}





nsMemory nsReflection::Memory("memory_reflection", NS_MEMORY_SIZE_KiB(32));

static nsTArray<const nsClass*> RegisteredClasses;


void nsReflection::RegisterObjectClass(const nsClass* objectClass)
{
	RegisteredClasses.AddUnique(objectClass);
}


const nsClass* nsReflection::FindClass(const nsName& name)
{
	const int index = RegisteredClasses.Find(name,
		[](const nsClass* check, const nsName& name)
	{
		return check->GetName() == name;
	}
	);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		return nullptr;
	}

	return RegisteredClasses[index];
}


nsTArray<const nsClass*> nsReflection::FindAllClasses(const nsClass* baseClass)
{
	if (baseClass == nullptr)
	{
		return nsTArray<const nsClass*>();
	}

	nsTArray<const nsClass*> classes;
	classes.Reserve(8);

	for (int i = 0; i < RegisteredClasses.GetCount(); ++i)
	{
		const nsClass* checkClass = RegisteredClasses[i];

		if (checkClass->IsSubclassOf(baseClass))
		{
			classes.Add(checkClass);
		}
	}

	return classes;
}


nsProperty* nsReflection::CreateProperty(nsName name, const nsType& type, int offset, bool bSerializable) noexcept
{
	return Memory.AllocateConstruct<nsProperty>(name, type, offset, bSerializable);
}
