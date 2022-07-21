#pragma once

#include "nsActor.h"



class NS_ENGINE_API nsActorAsset : public nsObject
{
	NS_DECLARE_OBJECT(nsActorAsset)

private:
	const nsClass* ActorClass;
	nsTArray<const nsClass*> ComponentClasses;


public:
	nsActorAsset();
	void Serialize(nsStream& stream);
	nsActor* CreateInstance(nsMemory& memory);


	NS_NODISCARD_INLINE const nsClass* GetActorClass() const
	{
		return ActorClass;
	}


	NS_NODISCARD_INLINE const nsTArray<const nsClass*>& GetComponentClasses() const
	{
		return ComponentClasses;
	}

};
