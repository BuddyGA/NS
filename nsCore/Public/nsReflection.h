#pragma once

#include "nsString.h"
#include "nsMath.h"
#include "nsMemory.h"



namespace nsEType
{
	enum Info
	{
		Void = 0,
		Pointer,
		Bool,
		Int,
		Float,
		String,
		Name,
		Vector2,
		Vector3,
		Array,

		COUNT
	};
};


constexpr const char* NS_TYPE_NAMES[nsEType::COUNT] =
{
	"void",
	"pointer",
	"bool",
	"int",
	"float",
	"nsString",
	"nsName",
	"nsVector2",
	"nsVector3",
	"nsTArray",
};


struct nsType
{
	int Size;
	nsEType::Info Info;


public:
	nsType(int size, nsEType::Info info) noexcept
		: Size(size)
		, Info(info)
	{
	}


	NS_NODISCARD_INLINE const char* GetName() const noexcept
	{
		return NS_TYPE_NAMES[Info];
	}


	NS_INLINE bool operator==(const nsType& rhs) noexcept
	{
		return Size == rhs.Size && Info == rhs.Info;
	}


	NS_INLINE bool operator!=(const nsType& rhs) noexcept
	{
		return !(*this == rhs);
	}

};



class nsProperty
{
	NS_DECLARE_NOCOPY(nsProperty)

private:
	nsName Name;
	nsType Type;
	int Offset;
	uint32 bIsSerializable : 1;
	

public:
	nsProperty(nsName name, nsType type, int offset, bool bSerializable) noexcept
		: Name(name)
		, Type(type)
		, Offset(offset)
		, bIsSerializable(bSerializable)
	{
	}


	template<typename T>
	NS_INLINE void SetValue(void* obj, const T& newValue)
	{
		NS_Assert(sizeof(T) == Type.Size);
		T& currentValue = *reinterpret_cast<T*>(static_cast<uint8*>(obj) + Offset);
		currentValue = newValue;
	}


	template<typename T>
	NS_NODISCARD_INLINE const T& GetValue(const void* obj) const
	{
		NS_Assert(sizeof(T) == Type.Size);
		return *reinterpret_cast<const T*>(static_cast<const uint8*>(obj) + Offset);
	}


	NS_NODISCARD_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE const nsType& GetType() const noexcept
	{
		return Type;
	}


	NS_NODISCARD_INLINE bool IsVoid() const noexcept
	{
		return Type.Info == nsEType::Void;
	}


	NS_NODISCARD_INLINE bool IsPointer() const noexcept
	{
		return Type.Info == nsEType::Pointer;
	}


	NS_NODISCARD_INLINE bool IsBool() const noexcept
	{
		return Type.Info == nsEType::Bool;
	}


	NS_NODISCARD_INLINE bool IsInt() const noexcept
	{
		return Type.Info == nsEType::Int;
	}


	NS_NODISCARD_INLINE bool IsFloat() const noexcept
	{
		return Type.Info == nsEType::Float;
	}


	NS_NODISCARD_INLINE bool IsString() const noexcept
	{
		return Type.Info == nsEType::String;
	}


	NS_NODISCARD_INLINE bool IsName() const noexcept
	{
		return Type.Info == nsEType::Name;
	}


	NS_NODISCARD_INLINE bool IsVector2() const noexcept
	{
		return Type.Info == nsEType::Vector2;
	}


	NS_NODISCARD_INLINE bool IsVector3() const noexcept
	{
		return Type.Info == nsEType::Vector3;
	}


	NS_NODISCARD_INLINE bool IsArray() const noexcept
	{
		return Type.Info == nsEType::Array;
	}


	NS_NODISCARD_INLINE bool IsSerializable() const noexcept
	{
		return bIsSerializable;
	}

};

typedef nsTArrayInline<nsProperty*, 32> nsPropertyList;



class nsObject;


class nsClass
{
private:
	nsName Name;
	const nsClass* ParentClass;
	nsObject* DefaultObject;

protected:
	nsPropertyList Properties;
	uint32 bIsAbstract : 1;


public:
	nsClass(nsName name, const nsClass* parentClass, nsObject* defaultObject) noexcept
		: Name(name)
		, ParentClass(parentClass)
		, DefaultObject(defaultObject)
		, bIsAbstract(false)
	{
	}


	bool IsSubclassOf(const nsClass* parentClass) const noexcept
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


	virtual nsObject* CreateInstance(nsMemory& memory) const
	{
		return memory.AllocateConstruct<nsObject>();
	}


	virtual void DestroyInstance(nsMemory& memory, nsObject* obj) const
	{
		return memory.DeallocateDestruct<nsObject>(obj);
	}


	template<typename T>
	NS_NODISCARD_INLINE T* CreateInstanceAs(nsMemory& memory) const
	{
		return static_cast<T*>(CreateInstance(memory));
	}


	NS_NODISCARD_INLINE const nsPropertyList& GetProperties() const noexcept
	{
		return Properties;
	}


	NS_NODISCARD_INLINE bool IsAbstract() const noexcept
	{
		return bIsAbstract;
	}


	NS_NODISCARD_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE const nsClass* GetParentClass() const noexcept
	{
		return ParentClass;
	}


	NS_NODISCARD_INLINE nsObject* GetDefaultObject() noexcept
	{
		return DefaultObject;
	}


	NS_NODISCARD_INLINE const nsObject* GetDefaultObject() const noexcept
	{
		return DefaultObject;
	}


	template<typename T>
	NS_NODISCARD_INLINE T* GetDefaultObjectAs() noexcept
	{
		return static_cast<T*>(GetDefaultObject());
	}


	template<typename T>
	NS_NODISCARD_INLINE const T* GetDefaultObjectAs() const noexcept
	{
		return static_cast<const T*>(GetDefaultObject());
	}

};



namespace nsReflection
{
	template<typename T>
	constexpr nsType CreateType() noexcept
	{
		if constexpr (std::is_pointer<T>::value) return nsType(sizeof(T*), nsEType::Pointer);
		if constexpr (std::is_same<T, bool>::value) return nsType(sizeof(bool), nsEType::Bool);
		if constexpr (std::is_same<T, int>::value) return nsType(sizeof(int), nsEType::Int);
		if constexpr (std::is_same<T, float>::value) return nsType(sizeof(float), nsEType::Float);
		if constexpr (std::is_same<T, nsString>::value) return nsType(sizeof(nsString), nsEType::String);
		if constexpr (std::is_same<T, nsName>::value) return nsType(sizeof(nsName), nsEType::Name);
		if constexpr (std::is_same<T, nsVector2>::value) return nsType(sizeof(nsVector2), nsEType::Vector2);
		if constexpr (std::is_same<T, nsVector3>::value) return nsType(sizeof(nsVector3), nsEType::Vector3);
		if constexpr (std::is_same<T, nsTArray<T>>::value) return nsType(sizeof(nsTArray<T>), nsEType::Array);

		return nsType(0, nsEType::Void);
	}


	extern NS_CORE_API nsMemory Memory;

	extern NS_CORE_API void RegisterObjectClass(const nsClass* objectClass);


	template<typename T>
	NS_NODISCARD_INLINE nsClass* CreateClass() noexcept
	{
		nsClass* newClass = Memory.AllocateConstruct<T>();
		RegisterObjectClass(newClass);

		return newClass;
	}

	NS_NODISCARD extern NS_CORE_API const nsClass* FindClass(const nsName& name);
	NS_NODISCARD extern NS_CORE_API nsTArray<const nsClass*> FindAllClasses(const nsClass* baseClass);


	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses(const nsName& name)
	{
		return FindAllClasses(FindClass(name));
	}


	template<typename TObject>
	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses()
	{
		return FindAllClasses(TObject::Class);
	}


	NS_NODISCARD extern NS_CORE_API nsProperty* CreateProperty(nsName name, const nsType& type, int offset, bool bSerializable) noexcept;

};



template<typename T>
class nsTClass : public nsClass
{
public:
	nsTClass(nsName name, const nsClass* parentClass)
		: nsClass(name, parentClass, nsReflection::Memory.AllocateConstruct<T>())
	{
		bIsAbstract = std::is_abstract<T>::value;
	}

};



#define NS_CreateProperty(classType, propertyType, propertyName, bSerializable) nsReflection::CreateProperty(#propertyName, nsReflection::CreateType<propertyType>(), offsetof(classType, propertyName), bSerializable)



#define NS_CLASS_BEGIN_NO_PARENT(classType)															\
class classType##__Class : public nsTClass<classType>												\
{																									\
public:																								\
	classType##__Class()																			\
		: nsTClass(#classType, nullptr)																\
	{


#define NS_CLASS_BEGIN(classType, parentClassType)																\
class classType##__Class : public nsTClass<classType>															\
{																												\
public:																											\
	classType##__Class()																						\
		: nsTClass(#classType, parentClassType::Class)															\
	{																	

#define NS_CLASS_AddProperty(classType, propertyType, propertyName, isSerializable) Properties.Add(NS_CreateProperty(classType, propertyType, propertyName, isSerializable));	

#define NS_CLASS_END(classType)																								\
	}																														\
	virtual nsObject* CreateInstance(nsMemory& memory) const override { return memory.AllocateConstruct<classType>(); }		\
	virtual void DestroyInstance(nsMemory& memory, nsObject* obj) const override											\
	{																														\
		classType* objThisType = static_cast<classType*>(obj);																\
		memory.DeallocateDestruct<classType>(objThisType);																	\
	}																														\
};																															\
const nsClass* classType::Class = nsReflection::CreateClass<classType##__Class>();


#define NS_DECLARE_OBJECT(type)											\
friend class type##__Class;												\
public:																	\
	static const nsClass* Class;										\
	virtual const nsClass* GetClass() const override { return Class; }
