#include "nsActorAsset.h"



NS_CLASS_BEGIN(nsActorAsset, nsObject)
NS_CLASS_END(nsActorAsset)

nsActorAsset::nsActorAsset()
	: ActorClass(nullptr)
{
}


void nsActorAsset::Serialize(nsStream& stream)
{

}


nsActor* nsActorAsset::CreateInstance(nsMemory& memory)
{
	return nullptr;
}
