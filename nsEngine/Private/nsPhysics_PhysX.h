#pragma once

#include "nsPhysics.h"
#include "nsRenderContextWorld.h"
#include "physx/PxPhysicsAPI.h"
#include "physx/foundation/PxAllocatorCallback.h"
#include "physx/foundation//PxErrorCallback.h"

using namespace physx;


#define NS_ToPxVec3(nsVec3) PxVec3(nsVec3.X, nsVec3.Y, nsVec3.Z)
#define NS_FromPxVec3(pxVec3) nsVector3(pxVec3.x, pxVec3.y, pxVec3.z)
#define NS_ToPxQuat(nsQuat) PxQuat(nsQuat.X, nsQuat.Y, nsQuat.Z, nsQuat.W)
#define NS_FromPxQuat(pxQuat) nsQuaternion(pxQuat.x, pxQuat.y, pxQuat.z, pxQuat.w)
#define NS_ToPxTransform(nsTrans) PxTransform(NS_ToPxVec3(nsTrans.Position), NS_ToPxQuat(nsTrans.Rotation))
#define NS_FromPxTransform(pxTrans) nsTransform(NS_FromPxVec3(pxTrans.p), NS_FromPxQuat(pxTrans.q))



class nsPhysicsManager_PhysX : public nsPhysicsManager
{
private:
	bool bInitialized;

	PxFoundation* Foundation;
	PxDefaultCpuDispatcher* CpuDispatcher;
	PxPhysics* Physics;
	PxMaterial* DefaultMaterial;

#ifndef __NS_ENGINE_SHIPPING__
	PxCooking* Cooking;
#endif // !__NS_ENGINE_SHIPPING__


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


private:
	NS_NODISCARD int CreatePhysicsObject(nsName name, bool bIsStatic, nsTransformComponent* transformComponent);


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


public:
	virtual nsPhysicsObjectID CreatePhysicsObject_Box(nsName name, const nsVector3& halfExtent, bool bIsStatic, bool bIsTrigger, nsTransformComponent* transformComponent) override;
	virtual nsPhysicsObjectID CreatePhysicsObject_ConvexMesh(nsName name, const nsTArray<nsVertexMeshPosition>& vertices, bool bIsStatic, nsTransformComponent* transformComponent) override;
	virtual void DestroyPhysicsObject(nsPhysicsObjectID& physicsObject) override;
	virtual bool IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const override;
	virtual void UpdatePhysicsObjectShape_Box(nsPhysicsObjectID physicsObject, const nsVector3& halfExtent) override;
	virtual void SetPhysicsObjectChannel(nsPhysicsObjectID physicsObject, nsEPhysicsCollisionChannel::Type objectChannel) override;
	virtual void SetPhysicsObjectCollisionChannels(nsPhysicsObjectID physicsObject, nsPhysicsCollisionChannels collisionChannels) override;
	virtual void SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation) override;
	virtual nsName GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const override;

	virtual void AddPhysicsObjectToScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;
	virtual void RemovePhysicsObjectFromScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene) override;
	virtual bool SceneQueryRayCast(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;

private:
	bool SceneQuerySweep(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, const PxGeometry& geometry, const nsTransform& transform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params);

public:
	virtual bool SceneQuerySweepBox(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, const nsVector3& halfExtent, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override
	{
		return SceneQuerySweep(scene, hitResult, PxBoxGeometry(NS_ToPxVec3(halfExtent)), worldTransform, direction, distance, params);
	}


	virtual bool SceneQuerySweepSphere(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, float sphereRadius, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override
	{
		return SceneQuerySweep(scene, hitResult, PxSphereGeometry(sphereRadius), worldTransform, direction, distance, params);
	}


	virtual bool SceneQuerySweepConvexMesh(nsPhysicsSceneID scene, nsPhysicsObjectID physicsObject, nsPhysicsHitResult& hitResult, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override
	{
		NS_Assert(IsPhysicsObjectValid(physicsObject));

		PxRigidActor* pxRigidActor = ObjectRigidActors[physicsObject.Id];
		NS_Assert(pxRigidActor);

		PxShape* shape = nullptr;
		pxRigidActor->getShapes(&shape, 1);
		NS_Assert(shape);

		PxConvexMeshGeometry convexMeshGeometry;
		bool bIsConvexMeshShape = shape->getConvexMeshGeometry(convexMeshGeometry);
		NS_ValidateV(bIsConvexMeshShape, "SceneQuerySweepConvexMesh() requires physics object with convex mesh geometry shape!");

		return SceneQuerySweep(scene, hitResult, convexMeshGeometry, worldTransform, direction, distance, params);
	}


	virtual bool AdjustPhysicsObjectPosition(nsPhysicsObjectID physicsObjectToAdjust, nsPhysicsObjectID physicsObjectAgaints) override;


#ifdef __NS_ENGINE_DEBUG_DRAW__
	virtual void DEBUG_Draw(nsRenderContextWorld& renderContextWorld) override;
#endif // __NS_ENGINE_DEBUG_DRAW__

};
