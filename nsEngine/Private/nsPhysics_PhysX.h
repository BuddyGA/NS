#pragma once

#include "nsPhysics.h"
#include "physx/PxPhysicsAPI.h"
#include "physx/foundation/PxAllocatorCallback.h"
#include "physx/foundation//PxErrorCallback.h"

using namespace physx;



class nsPhysX_AllocatorCallback : public PxAllocatorCallback
{
private:
	nsCriticalSection CriticalSection;
	nsMemory Memory;

public:
	nsPhysX_AllocatorCallback();
	virtual void* allocate(size_t size, const char*, const char*, int) override;
	virtual void deallocate(void* ptr) override;

};




class nsPhysX_ErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override;

};



class nsPhysicsManager_PhysX : public nsPhysicsManager
{
private:
	bool bInitialized;

	nsPhysX_AllocatorCallback AllocatorCallback;
	nsPhysX_ErrorCallback ErrorCallback;
	PxFoundation* Foundation;
	PxDefaultCpuDispatcher* DefaultCpuDispatcher;
	PxPhysics* Physics;

	nsTArrayFreeList<nsName> SceneNames;
	nsTArrayFreeList<nsPhysicsSceneSettings> SceneSettings;
	nsTArrayFreeList<PxScene*> SceneObjects;

	nsTArrayFreeList<nsName> ObjectNames;
	nsTArrayFreeList<nsPhysicsObjectSettings> ObjectSettings;


public:
	nsPhysicsManager_PhysX();
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

	virtual nsPhysicsSceneID CreatePhysicsScene(nsName name, const nsPhysicsSceneSettings& settings = nsPhysicsSceneSettings()) override;
	virtual void DestroyPhysicsScene(nsPhysicsSceneID& scene) override;
	virtual void SyncPhysicsSceneTransforms(nsPhysicsSceneID scene) override;
	virtual bool IsPhysicsSceneValid(nsPhysicsSceneID scene) const override;
	virtual nsName GetPhysicsSceneName(nsPhysicsSceneID scene) const override;

	virtual nsPhysicsObjectID CreatePhysicsObjectRigidBody_Box(nsName name, nsActor* actor, const nsVector3& halfExtent, nsPhysicsCollisionLayers collisionLayers = nsEPhysicsCollisionLayer::Default, bool bIsStatic = true) override;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) override;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) override;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const override;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const override;

};
