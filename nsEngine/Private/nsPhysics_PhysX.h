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
	nsTArrayFreeList<nsPhysicsSceneSettings> SceneSettings;
	nsTArrayFreeList<PxScene*> SceneObjects;

	nsTArrayFreeList<nsName> ObjectNames;
	nsTArrayFreeList<nsPhysicsObjectSettings> ObjectSettings;
	nsTArrayFreeList<PxRigidActor*> ObjectRigidActors;


public:
	nsPhysicsManager_PhysX();
	virtual void Initialize() override;
	virtual void Update(float fixedDeltaTime) override;

	virtual nsPhysicsSceneID CreatePhysicsScene(nsName name, const nsPhysicsSceneSettings& settings = nsPhysicsSceneSettings()) override;
	virtual void DestroyPhysicsScene(nsPhysicsSceneID& scene) override;
	virtual void SyncPhysicsSceneTransforms(nsPhysicsSceneID scene) override;
	virtual bool IsPhysicsSceneValid(nsPhysicsSceneID scene) const override;
	virtual nsName GetPhysicsSceneName(nsPhysicsSceneID scene) const override;

	virtual nsPhysicsObjectID CreatePhysicsObjectRigidBody_Box(nsName name, nsActor* actor, const nsVector3& halfExtent, nsEPhysicsCollisionChannel collisionChannel = nsEPhysicsCollisionChannel::NONE, bool bIsStatic = true) override;
	virtual void UpdatePhysicsObjectShape_Box(nsPhysicsObjectID physicsObject, const nsVector3& halfExtent) override;
	virtual void SetPhysicsObjectCollisionChannel(nsPhysicsObjectID physicsObject, nsEPhysicsCollisionChannel collisionChannel) override;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) override;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) override;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const override;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const override;

	virtual void AddPhysicsObjectToScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;
	virtual void RemovePhysicsObjectFromScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;


#ifdef __NS_ENGINE_DEBUG_DRAW__
	virtual void DEBUG_Draw(nsRenderContextWorld& renderContextWorld) override;
#endif // __NS_ENGINE_DEBUG_DRAW__


private:
	NS_NODISCARD_INLINE int AllocatePhysicsObject(nsName name, nsActor* actor, nsEPhysicsCollisionChannel collisionChannel, nsEPhysicsShape shape, bool bIsStatic)
	{
		NS_Assert(actor);
		NS_Assert(shape != nsEPhysicsShape::NONE);

		const int nameId = ObjectNames.Add();
		const int settingsId = ObjectSettings.Add();
		const int rigidActorId = ObjectRigidActors.Add();
		NS_Assert(nameId == settingsId && settingsId == rigidActorId);

		ObjectNames[nameId] = name;

		nsPhysicsObjectSettings& settings = ObjectSettings[settingsId];
		settings.CollisionChannel = collisionChannel;
		settings.Shape = shape;
		settings.bIsStatic = bIsStatic;
		settings.UserData = actor;

		return nameId;
	}


	NS_INLINE void DeallocatePhysicsObject(nsPhysicsObjectID& physicsObject)
	{
		NS_Assert(IsPhysicsObjectValid(physicsObject));

	}


};
