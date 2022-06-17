#pragma once

#include "nsString.h"



class NS_CORE_API nsClass
{
private:
	nsName Name;
	const nsClass* ParentClass;


public:
	nsClass(nsName name, const nsClass* parentClass) noexcept;


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
	enum Flag
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


private:
	nsObjectFlags Flags;

public:
	nsName Name;


public:
	nsObject() noexcept;
	nsObject(nsName name) noexcept;
	virtual const nsClass* GetClass() const noexcept;


	NS_NODISCARD_INLINE nsObjectFlags GetFlags() const noexcept
	{
		return Flags;
	}


	template<typename T>
	NS_NODISCARD_INLINE bool IsClass() const noexcept
	{
		return GetClass() == T::Class;
	}


	template<typename T>
	NS_NODISCARD_INLINE bool IsSubclassType() const noexcept
	{
		const nsClass* checkClass = GetClass();

		while (checkClass)
		{
			if (checkClass == T::Class)
			{
				return true;
			}

			checkClass = checkClass->GetParentClass();
		}

		return false;
	}

};



#define NS_DECLARE_OBJECT() \
public: \
	static const nsClass* Class; \
	virtual const nsClass* GetClass() const noexcept;


#define NS_DEFINE_OBJECT(type, name, baseType) \
const nsClass* type::Class = new nsClass(name, baseType::Class); \
const nsClass* type::GetClass() const noexcept \
{ \
	return Class; \
}
