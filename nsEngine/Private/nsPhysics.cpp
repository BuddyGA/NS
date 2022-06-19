#include "nsPhysics_PhysX.h"


NS_ENGINE_DEFINE_HANDLE(nsPhysicsObjectID);
NS_ENGINE_DEFINE_HANDLE(nsPhysicsSceneID);



nsPhysicsManager& nsPhysicsManager::Get()
{
#if NS_ENGINE_PHYSICS_USE_PHYSX
	static nsPhysicsManager_PhysX instance;
#else
	static nsPhysicsManager instance;
#endif // NS_PHYSICS_ENGINE_USE_PHYSX

	return instance;
}
