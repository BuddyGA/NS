#include "cstProjectile.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"



NS_CLASS_BEGIN(cstProjectile, nsActor)
NS_CLASS_END(cstProjectile)

cstProjectile::cstProjectile()
{
	Flags |= nsEActorFlag::CallPhysicsTickUpdate;

	SphereCollisionComponent = AddComponent<nsSphereCollisionComponent>(TEXT("sphere_collision"));
	RootComponent = SphereCollisionComponent;

	MeshComponent = AddComponent<nsMeshComponent>(TEXT("mesh"));
}


void cstProjectile::OnPhysicsTickUpdate(float deltaTime)
{
	nsActor::OnPhysicsTickUpdate(deltaTime);

}
