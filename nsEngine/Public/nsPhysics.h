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



namespace nsEPhysicsCollisionLayer
{
	enum Flags : uint16
	{
		None			= (0),

		Default			= (1 << 0),
		Character		= (1 << 1),
		Water			= (1 << 2),

		// ...

		ALL				= (UINT16_MAX)
	};
};

typedef uint16 nsPhysicsCollisionLayers;



struct nsPhysicsSceneSettings
{
	nsVector3 Gravity = nsVector3(0.0f, -980.0f, 0.0f);
	int MaxRigidBodies = 1024;
};



struct nsPhysicsObjectSettings
{
	nsEPhysicsShape Shape = nsEPhysicsShape::NONE;
	nsPhysicsCollisionLayers CollisionLayers = 0;
	void* UserData = nullptr;
};



class NS_ENGINE_API nsPhysicsManager
{
	NS_DECLARE_NOCOPY_NOMOVE(nsPhysicsManager)


public:
	static nsPhysicsManager& Get();

	nsPhysicsManager() = default;
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;

	virtual nsPhysicsSceneID CreatePhysicsScene(nsName name, const nsPhysicsSceneSettings& settings = nsPhysicsSceneSettings()) = 0;
	virtual void DestroyPhysicsScene(nsPhysicsSceneID& scene) = 0;
	virtual void SyncPhysicsSceneTransforms(nsPhysicsSceneID scene) = 0;
	virtual bool IsPhysicsSceneValid(nsPhysicsSceneID scene) const = 0;
	virtual nsName GetPhysicsSceneName(nsPhysicsSceneID scene) const = 0;

	virtual nsPhysicsObjectID CreatePhysicsObjectRigidBody_Box(nsName name, nsActor* actor, const nsVector3& halfExtent, nsPhysicsCollisionLayers collisionLayers = nsEPhysicsCollisionLayer::Default, bool bIsStatic = true) = 0;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) = 0;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) = 0;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const = 0;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const = 0;

};
