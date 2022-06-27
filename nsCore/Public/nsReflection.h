#pragma once

#include "nsString.h"
#include "nsMath.h"
#include "nsMemory.h"



struct nsType
{
	const char* Name;
	int Size;
};


constexpr nsType nsTypeVoid({ "void", 0 });
constexpr nsType nsTypeBool({ "bool", sizeof(bool) });
constexpr nsType nsTypeInt({ "int", sizeof(int) });
constexpr nsType nsTypeFloat({ "float", sizeof(float) });
constexpr nsType nsTypeString({ "nsString", sizeof(nsString) });
constexpr nsType nsTypeName({ "nsName", sizeof(nsName) });
constexpr nsType nsTypeVector2({ "nsVector2", sizeof(nsVector2) });
constexpr nsType nsTypeVector3({ "nsVector3", sizeof(nsVector3) });
constexpr nsType nsTypePointer({ "pointer", sizeof(void*) });



class NS_CORE_API nsProperty
{
	NS_DECLARE_NOCOPY(nsProperty)

private:
	nsName Name;
	nsType Type;
	int Offset;
	bool bIsSerializable;


public:
	nsProperty(nsName name, const nsType& type, int offset, bool bSerializable) noexcept
		: Name(name)
		, Type(type)
		, Offset(offset)
		, bIsSerializable(bSerializable)
	{
	}

};

typedef nsTArrayInline<nsProperty*, 32> nsPropertyList;



class NS_CORE_API nsClass
{
private:
	nsName Name;
	const nsClass* ParentClass;

protected:
	nsPropertyList Properties;
	uint32 bIsAbstract : 1;


public:
	nsClass(nsName name, const nsClass* parentClass) noexcept;
	NS_NODISCARD bool IsSubclassOf(const nsClass* parentClass) const noexcept;
	NS_NODISCARD const nsPropertyList& GetProperties() const noexcept;


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

};


template<typename T>
class nsTClass : public nsClass
{
public:
	nsTClass(nsName name, const nsClass* parentClass)
		: nsClass(name, parentClass)
	{
		bIsAbstract = std::is_abstract<T>::value;
	}

};



namespace nsReflection
{
	template<typename T>
	constexpr const nsType& GetType() noexcept
	{
		if constexpr (std::is_same<T, bool>::value) return nsTypeBool;
		if constexpr (std::is_same<T, int>::value) return nsTypeInt;
		if constexpr (std::is_same<T, float>::value) return nsTypeFloat;
		if constexpr (std::is_same<T, nsString>::value) return nsTypeString;
		if constexpr (std::is_same<T, nsName>::value) return nsTypeName;
		if constexpr (std::is_same<T, nsVector2>::value) return nsTypeVector2;
		if constexpr (std::is_same<T, nsVector3>::value) return nsTypeVector3;
		if constexpr (std::is_pointer<T>::value) return nsTypePointer;

		return nsTypeVoid;
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

	extern NS_NODISCARD NS_CORE_API const nsClass* FindClass(const nsName& name);
	extern NS_NODISCARD NS_CORE_API nsTArray<const nsClass*> FindAllClasses(const nsClass* baseClass);


	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses(const nsName& name)
	{
		return FindAllClasses(FindClass(name));
	}


	template<typename TObject>
	NS_NODISCARD_INLINE nsTArray<const nsClass*> FindAllClasses()
	{
		return FindAllClasses(TObject::Class);
	}


	extern NS_CORE_API nsProperty* CreateProperty(nsName name, const nsType& type, int offset, bool bSerializable) noexcept;

};


#define NS_CreateProperty(name, classType, propertyType, bSerializable) nsReflection::CreateProperty(#name, nsReflection::GetType<propertyType>(), offsetof(classType, name), bSerializable)


#define NS_PROPERTY(...)
#define NS_CLASS(...)



#define NS_CLASS_BEGIN_NO_PARENT(classType)												\
class classType##__Class : public nsTClass<classType>									\
{																						\
public:																					\
	classType##__Class()																\
		: nsTClass(#classType, nullptr)													\
	{


#define NS_CLASS_BEGIN(classType, parentClassType)										\
class classType##__Class : public nsTClass<classType>									\
{																						\
public:																					\
	classType##__Class()																\
		: nsTClass(#classType, parentClassType::Class)									\
	{																	

#define NS_CLASS_Property(classType, propertyName, propertyType, serializable)			\
		Properties.Add(NS_CreateProperty(propertyName, classType, propertyType, true));	\

#define NS_CLASS_END(classType)															\
	}																					\
};																						\
const nsClass* classType::Class = nsReflection::CreateClass<classType##__Class>();


#define NS_DECLARE_OBJECT() \
public: \
	static const nsClass* Class; \
	virtual const nsClass* GetClass() const override { return Class; }
