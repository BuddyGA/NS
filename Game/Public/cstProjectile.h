#pragma once

#include "cstTypes.h"



class cstProjectile : public nsActor
{
	NS_DECLARE_OBJECT(cstProjectile)

private:
	class nsSphereCollisionComponent* SphereCollisionComponent;
	class nsMeshComponent* MeshComponent;


public:
	cstProjectile();

protected:
	virtual void OnPhysicsTickUpdate(float deltaTime) override;

};
