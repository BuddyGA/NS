#pragma once

#include "nsEngineTypes.h"
#include "physx/PxPhysicsAPI.h"


using namespace physx;


#define NS_ToPxVec3(nsVec3) physx::PxVec3(nsVec3.X, nsVec3.Y, nsVec3.Z)
#define NS_FromPxVec3(pxVec3) nsVector3(pxVec3.x, pxVec3.y, pxVec3.z)
#define NS_ToPxQuat(nsQuat) PxQuat(nsQuat.X, nsQuat.Y, nsQuat.Z, nsQuat.W)
#define NS_FromPxQuat(pxQuat) nsQuaternion(pxQuat.x, pxQuat.y, pxQuat.z, pxQuat.w)
#define NS_ToPxTransform(nsTrans) PxTransform(NS_ToPxVec3(nsTrans.Position), NS_ToPxQuat(nsTrans.Rotation))
#define NS_FromPxTransform(pxTrans) nsTransform(NS_FromPxVec3(pxTrans.p), NS_FromPxQuat(pxTrans.q))



namespace nsPhysX
{
	extern bool SceneQueryRayCast(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const PxVec3& origin, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params);
	extern bool SceneQueryRayCastMany(physx::PxScene* scene, nsPhysicsHitResultMany& outHitResultMany, const PxVec3& origin, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params);
	extern bool SceneQuerySweep(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const PxGeometry& geometry, const PxTransform& transform, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params);
	extern bool SceneQuerySweepMany(physx::PxScene* scene, nsPhysicsHitResultMany& outHitResultMany, const PxGeometry& geometry, const PxTransform& transform, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params);

};
