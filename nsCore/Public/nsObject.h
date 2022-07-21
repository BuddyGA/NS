#pragma once

#include "nsReflection.h"



class NS_CORE_API nsObject
{
	NS_DECLARE_NOCOPY(nsObject)

public:
	static const nsClass* Class;

public:
	nsString Name;


public:
	nsObject() noexcept;
	nsObject(nsString name) noexcept;
	virtual const nsClass* GetClass() const;
	const nsPropertyList& GetProperties() const;


	template<typename T>
	NS_NODISCARD_INLINE bool IsClass() const noexcept
	{
		return GetClass()->IsSubclassOf(T::Class);
	}

};



template<typename T>
NS_NODISCARD_INLINE T* ns_Cast(nsObject* obj) noexcept
{
	if (obj == nullptr)
	{
		return nullptr;
	}

	return obj->IsClass<T>() ? static_cast<T*>(obj) : nullptr;
}



NS_NODISCARD_INLINE nsObject* ns_GetDefaultObject(const nsClass* objClass) noexcept
{
	nsClass* mutableClass = const_cast<nsClass*>(objClass);
	return mutableClass->GetDefaultObject();
}


template<typename T>
NS_NODISCARD_INLINE T* ns_GetDefaultObjectAs(const nsClass* objClass) noexcept
{
	return static_cast<T*>(ns_GetDefaultObject(objClass));
}
