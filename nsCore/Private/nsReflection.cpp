#include "nsReflection.h"
#include "nsObject.h"




nsMemory nsReflection::Memory("reflection", NS_MEMORY_SIZE_MiB(1));

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
