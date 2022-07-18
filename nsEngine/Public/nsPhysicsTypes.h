#pragma once

#include "nsEngineTypes.h"



namespace physx
{
	class PxPhysics;
	class PxMaterial;
	class PxCooking;
	class PxScene;
	class PxAggregate;
	class PxRigidActor;
	class PxShape;
};




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
		MousePicking	= (1 << 3),
	};
};

typedef uint32 nsPhysicsCollisionChannels;



enum class nsEPhysicsCollisionTest : uint8
{
	NONE = 0,
	COLLISION_ONLY,
	QUERY_ONLY,
	COLLISION_AND_QUERY
};


typedef nsTArrayInline<nsActor*, 8> nsPhysicsQueryIgnoredActors;



struct nsPhysicsQueryParams
{
	nsEPhysicsCollisionChannel::Type Channel;
	nsPhysicsQueryIgnoredActors IgnoredActors;


public:
	nsPhysicsQueryParams()
		: Channel(nsEPhysicsCollisionChannel::Default)
		, IgnoredActors()
	{
	}

};



struct nsPhysicsHitResult
{
	nsActor* Actor;
	nsActorComponent* Component;
	nsVector3 WorldPosition;
	nsVector3 WorldNormal;
	float Distance;
	bool bIsBlock;


public:
	nsPhysicsHitResult()
		: Actor(nullptr)
		, Component(nullptr)
		, WorldPosition()
		, WorldNormal()
		, Distance(0.0f)
		, bIsBlock(false)
	{
	}

};

typedef nsTArrayInline<nsPhysicsHitResult, NS_ENGINE_PHYSICS_MAX_HIT_RESULT> nsPhysicsHitResultMany;
