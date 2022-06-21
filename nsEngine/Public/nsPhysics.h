#pragma once

#include "nsEngineTypes.h"


#if NS_ENGINE_PHYSICS_USE_PHYSX
NS_ENGINE_DECLARE_HANDLE(nsPhysicsObjectID, nsPhysicsManager_PhysX)
NS_ENGINE_DECLARE_HANDLE(nsPhysicsSceneID, nsPhysicsManager_PhysX)
#else
NS_ENGINE_DECLARE_HANDLE(nsPhysicsObjectID, nsPhysicsManager)
NS_ENGINE_DECLARE_HANDLE(nsPhysicsSceneID, nsPhysicsManager)
#endif // NS_ENGINE_PHYSICS_USE_PHYSX



enum class nsEPhysicsShape : uint8
{
	NONE = 0,
	BOX,
	SPHERE,
	CAPSULE,
	CONVEX_MESH,
	TRIANGLE_MESH,
	HEIGHTFIELD
};



namespace nsEPhysicsCollisionChannel
{
	enum Type : uint32
	{
		Default			= (1 << 0),
		Character		= (1 << 1),
		Camera			= (1 << 2),
		Water			= (1 << 3),

		// ...
	};
};

typedef uint32 nsPhysicsCollisionChannels;




class NS_ENGINE_API nsPhysicsManager
{
	NS_DECLARE_NOCOPY_NOMOVE(nsPhysicsManager)


public:
	static nsPhysicsManager& Get();

	nsPhysicsManager() = default;
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;

	virtual nsPhysicsSceneID CreatePhysicsScene(nsName name) = 0;
	virtual void DestroyPhysicsScene(nsPhysicsSceneID& scene) = 0;
	virtual void SyncPhysicsSceneTransforms(nsPhysicsSceneID scene) = 0;
	virtual bool IsPhysicsSceneValid(nsPhysicsSceneID scene) const = 0;
	virtual nsName GetPhysicsSceneName(nsPhysicsSceneID scene) const = 0;

	virtual nsPhysicsObjectID CreatePhysicsObject_Box(nsName name, const nsVector3& halfExtent, nsEPhysicsCollisionChannel::Type collisionChannel, bool bIsStatic, bool bIsTrigger, void* transformComponent) = 0;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) = 0;
	virtual void UpdatePhysicsObjectShape_Box(nsPhysicsObjectID physicsObject, const nsVector3& halfExtent) = 0;
	virtual void SetPhysicsObjectChannel(nsPhysicsObjectID physicsObject, nsEPhysicsCollisionChannel::Type objectChannel) = 0;
	virtual void SetPhysicsObjectCollisionChannels(nsPhysicsObjectID physicsObject, nsPhysicsCollisionChannels collisionChannels) = 0;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) = 0;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const = 0;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const = 0;

	virtual void AddPhysicsObjectToScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) = 0;
	virtual void RemovePhysicsObjectFromScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) = 0;


#ifdef __NS_ENGINE_DEBUG_DRAW__
	virtual void DEBUG_Draw(class nsRenderContextWorld& renderContextWorld) = 0;
#else
	NS_INLINE void DEBUG_Draw(class nsRenderContextWorld&) {}
#endif // __NS_ENGINE_DEBUG_DRAW__

};
