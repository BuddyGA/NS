#pragma once

#include "nsActorComponents.h"
#include "nsPhysicsTypes.h"



// ================================================================================================================================== //
// COLLISION COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsCollisionComponent : public nsTransformComponent
{
	NS_DECLARE_OBJECT()

protected:
	physx::PxRigidActor* PhysicsActor;
	physx::PxMaterial* PhysicsMaterial;
	physx::PxShape* PhysicsShape;
	nsEPhysicsCollisionTest CollisionTest;
	nsEPhysicsCollisionChannel::Type ObjectChannel;
	nsPhysicsCollisionChannels CollisionChannels;
	bool bIsTrigger;
	bool bIsKinematic;
	bool bSimulatePhysics;
	bool bEnableGravity;


public:
	nsCollisionComponent();
	virtual void OnInitialize() override;
	virtual void OnStartPlay() override;
	virtual void OnStopPlay() override;
	virtual void OnDestroy() override;
	virtual void OnStaticChanged() override;
	virtual void OnAddedToLevel() override;
	virtual void OnRemovedFromLevel() override;
	virtual void OnTransformChanged() override;

protected:
	void UpdateCollisionActor();
	void UpdateCollisionActorSimulation();
	void UpdateCollisionShapeFlags();
	void UpdateCollisionShapeChannels();
	virtual void UpdateCollisionShape() = 0;

public:
	void UpdateCollisionVolume();
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) = 0;
	bool AdjustPositionIfOverlappedWith(nsActor* actorToTest);
	void SetKinematicTarget(nsTransform transform);


	NS_INLINE void SetAsTrigger(bool bTrigger)
	{
		if (bIsTrigger != bTrigger)
		{
			bIsTrigger = bTrigger;
			UpdateCollisionShapeFlags();
		}
	}


	NS_INLINE void SetAsKinematic(bool bKinematic)
	{
		if (bIsKinematic != bKinematic)
		{
			bIsKinematic = bKinematic;
			UpdateCollisionActorSimulation();
		}
	}


	NS_INLINE void SetSimulatePhysics(bool bSimulate)
	{
		if (bSimulatePhysics != bSimulate)
		{
			bSimulatePhysics = bSimulate;
			UpdateCollisionActorSimulation();
		}
	}


	NS_INLINE void SetEnableGravity(bool bEnabled)
	{
		if (bEnableGravity != bEnabled)
		{
			bEnableGravity = bEnabled;
			UpdateCollisionActorSimulation();
		}
	}


	NS_INLINE void SetCollisionTest(nsEPhysicsCollisionTest test)
	{
		if (CollisionTest != test)
		{
			CollisionTest = test;
			UpdateCollisionShapeFlags();
		}
	}


	NS_INLINE void SetObjectChannel(nsEPhysicsCollisionChannel::Type newObjectChannel)
	{
		if (ObjectChannel != newObjectChannel)
		{
			ObjectChannel = newObjectChannel;
			UpdateCollisionShapeChannels();
		}
	}


	NS_INLINE void SetCollisionChannels(nsPhysicsCollisionChannels newCollisionChannels)
	{
		if (CollisionChannels != newCollisionChannels)
		{
			CollisionChannels = newCollisionChannels;
			UpdateCollisionShapeChannels();
		}
	}


	NS_NODISCARD_INLINE nsEPhysicsCollisionTest GetCollisionTest() const
	{
		return CollisionTest;
	}


	NS_NODISCARD_INLINE nsEPhysicsCollisionChannel::Type GetObjectChannel() const
	{
		return ObjectChannel;
	}


	NS_NODISCARD_INLINE nsPhysicsCollisionChannels GetCollisionChannels() const
	{
		return CollisionChannels;
	}


	NS_NODISCARD_INLINE bool IsTrigger() const
	{
		return bIsTrigger;
	}


	NS_NODISCARD_INLINE physx::PxRigidActor* Internal_GetPhysicsActor() const
	{
		return PhysicsActor;
	}


	NS_NODISCARD_INLINE physx::PxShape* Internal_GetPhysicsShape() const
	{
		return PhysicsShape;
	}

};




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsBoxCollisionComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

public:
	nsVector3 HalfExtent;


public:
	nsBoxCollisionComponent();

protected:
	virtual void UpdateCollisionShape() override;

public:
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;

};




// ================================================================================================================================== //
// CAPSULE COLLISION COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsCapsuleCollisionComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

public:
	float Height;
	float Radius;


public:
	nsCapsuleCollisionComponent();

protected:
	virtual void UpdateCollisionShape() override;

public:
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;

};




// ================================================================================================================================== //
// CONVEX MESH COLLISION COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsConvexMeshCollisionComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

private:
	nsMeshID Mesh;


public:
	nsConvexMeshCollisionComponent();

protected:
	virtual void UpdateCollisionShape() override;

public:
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;


	NS_INLINE void SetMesh(nsMeshID newMesh)
	{
		if (Mesh != newMesh)
		{
			Mesh = newMesh;
			UpdateCollisionShape();
		}
	}

};




// ================================================================================================================================== //
// CHARACTER MOVEMENT COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsCharacterMovementComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

public:
	float CapsuleHeight;
	float CapsuleRadius;
	float ContactOffset;
	float Acceleration;
	float Deceleration;
	float MaxSpeed;
	float SlopeLimit;
	float StepHeight;

protected:
	nsVector3 Velocity;
	nsPhysicsHitResultMany MoveHitResultMany;

private:
	float CurrentSlopeAngle;
	bool bIsOnGround;


public:
	nsCharacterMovementComponent();
	virtual void OnInitialize() override;

protected:
	virtual void UpdateCollisionShape() override;

public:
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;


protected:
	bool SweepCapsule(const nsTransform& worldTransform, const nsVector3& moveDirection);
	nsPhysicsHitResult SweepCapsuleAndFindClosestHit(const nsTransform& worldTransform, const nsVector3& moveDirection);
	nsVector3 GetNewTargetPositionToSlideOnSurface(const nsVector3& currentPosition, const nsVector3& currentTargetPosition, const nsVector3& surfaceNormal);

public:
	virtual void Move(float deltaTime, const nsVector3& worldDirection);
	void SetupCapsule(float height, float radius);

};
