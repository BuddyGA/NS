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
	const nsPropertyList GetProperties() const;


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
