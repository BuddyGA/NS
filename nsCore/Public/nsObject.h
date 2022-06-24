#pragma once

#include "nsString.h"



class NS_CORE_API nsClass
{
private:
	nsName Name;
	const nsClass* ParentClass;


public:
	nsClass(nsName name, const nsClass* parentClass) noexcept;
	NS_NODISCARD bool IsSubclassOf(const nsClass* parentClass) const noexcept;


	NS_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}

	NS_INLINE const nsClass* GetParentClass() const noexcept
	{
		return ParentClass;
	}

};




namespace nsEObject
{
	enum Flag : uint32
	{
		None			= (0),
		Default			= (1 << 0),
	};
};

typedef uint32 nsObjectFlags;



class NS_CORE_API nsObject
{
	NS_DECLARE_NOCOPY(nsObject)

public:
	static const nsClass* Class;


public:
	nsName Name;


public:
	nsObject() noexcept;
	nsObject(nsName name) noexcept;
	virtual const nsClass* GetClass() const noexcept;


	template<typename T>
	NS_NODISCARD_INLINE bool IsClass() const noexcept
	{
		return GetClass()->IsSubclassOf(T::Class);
	}

};



class NS_CORE_API nsObjectManager
{
	NS_DECLARE_SINGLETON(nsObjectManager)

private:
	nsTArray<const nsClass*> RegisteredObjectClasses;


public:
	void RegisterObjectClass(const nsClass* objectClass);
	NS_NODISCARD const nsClass* FindObjectClass(const nsName& name) const;
	NS_NODISCARD nsTArray<const nsClass*> FindAllClasses(const nsClass* baseClass) const;


	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses(const nsName& name) const
	{
		return FindAllClasses(FindObjectClass(name));
	}


	template<typename TObject>
	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses() const
	{
		static_assert(std::is_base_of<nsObject, TObject>::value, "FindAllClasses() type of <TObject> must be derived from type <nsObject>!");
		return FindAllClasses(TObject::Class);
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



#define NS_DECLARE_OBJECT() \
public: \
	static const nsClass* Class; \
	virtual const nsClass* GetClass() const noexcept;


#define NS_DEFINE_OBJECT(type, baseType) \
const nsClass* type::Class = new nsClass(#type, baseType::Class); \
const nsClass* type::GetClass() const noexcept \
{ \
	return Class; \
}
