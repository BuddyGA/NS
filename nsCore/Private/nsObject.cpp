#include "nsObject.h"



nsClass::nsClass(nsName name, const nsClass* parentClass) noexcept
	: Name(name)
	, ParentClass(parentClass)
{

}



const nsClass* nsObject::Class = new nsClass("Object", nullptr);

nsObject::nsObject() noexcept
	: Flags(nsEObject::Default)
{
}


nsObject::nsObject(nsName name) noexcept
	: Flags(nsEObject::Default)
	, Name(name)
{
}


const nsClass* nsObject::GetClass() const noexcept
{
	return Class;
}
