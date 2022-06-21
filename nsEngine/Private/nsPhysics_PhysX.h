#pragma once

#include "nsPhysics.h"
#include "nsRenderContextWorld.h"
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
	PxDefaultCpuDispatcher* CpuDispatcher;
	PxPhysics* Physics;
	PxMaterial* DefaultMaterial;

	nsTArrayFreeList<nsName> SceneNames;
	nsTArrayFreeList<PxScene*> SceneObjects;

	nsTArrayFreeList<nsName> ObjectNames;
	nsTArrayFreeList<PxRigidActor*> ObjectRigidActors;


public:
	nsPhysicsManager_PhysX();
	virtual void Initialize() override;
	virtual void Update(float fixedDeltaTime) override;

	virtual nsPhysicsSceneID CreatePhysicsScene(nsName name) override;
	virtual void DestroyPhysicsScene(nsPhysicsSceneID& scene) override;
	virtual bool IsPhysicsSceneValid(nsPhysicsSceneID scene) const override;
	virtual void SyncPhysicsSceneTransforms(nsPhysicsSceneID scene) override;
	virtual nsName GetPhysicsSceneName(nsPhysicsSceneID scene) const override;

	virtual nsPhysicsObjectID CreatePhysicsObject_Box(nsName name, const nsVector3& halfExtent, nsEPhysicsCollisionChannel::Type collisionChannel, bool bIsStatic, bool bIsTrigger, void* transformComponent) override;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) override;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const override;
	virtual void UpdatePhysicsObjectShape_Box(nsPhysicsObjectID physicsObject, const nsVector3& halfExtent) override;
	virtual void SetPhysicsObjectChannel(nsPhysicsObjectID physicsObject, nsEPhysicsCollisionChannel::Type objectChannel) override;
	virtual void SetPhysicsObjectCollisionChannels(nsPhysicsObjectID physicsObject, nsPhysicsCollisionChannels collisionChannels) override;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) override;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const override;

	virtual void AddPhysicsObjectToScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;
	virtual void RemovePhysicsObjectFromScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;


#ifdef __NS_ENGINE_DEBUG_DRAW__
	virtual void DEBUG_Draw(nsRenderContextWorld& renderContextWorld) override;
#endif // __NS_ENGINE_DEBUG_DRAW__


private:
	NS_NODISCARD_INLINE int AllocatePhysicsObject()
	{
		const int nameId = ObjectNames.Add();
		const int rigidActorId = ObjectRigidActors.Add();
		NS_Assert(nameId == rigidActorId);

		return nameId;
	}


	NS_INLINE void DeallocatePhysicsObject(nsPhysicsObjectID& physicsObject)
	{
		NS_Assert(IsPhysicsObjectValid(physicsObject));

	}


};
