#pragma once

#include "nsEngineTypes.h"



class NS_ENGINE_API nsPhysicsManager
{
	NS_DECLARE_SINGLETON(nsPhysicsManager)

private:
	bool bInitialized;

	physx::PxPhysics* Physics;
	physx::PxMaterial* DefaultMaterial;

#ifndef __NS_ENGINE_SHIPPING__
	physx::PxCooking* Cooking;
#endif // !__NS_ENGINE_SHIPPING__

	nsTArray<nsName> SceneNames;
	nsTArray<physx::PxScene*> SceneObjects;


public:
	bool bGlobalSimulate;


public:
	void Initialize();
	void Simulate(float fixedTimeSteps);

	NS_NODISCARD physx::PxScene* CreateScene(nsName name);
	void DestroyScene(physx::PxScene*& scene);
	bool SceneQueryRayCast(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams());
	bool SceneQuerySweepBox(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& halfExtent, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams());
	bool SceneQuerySweepSphere(physx::PxScene* scene, nsPhysicsHitResult& hitResult, float sphereRadius, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams());
	bool SceneQueryMousePicking(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const nsVector2& mousePosition, nsViewport* viewport);


	NS_NODISCARD_INLINE physx::PxMaterial* GetDefaultMaterial() const
	{
		return DefaultMaterial;
	}


	NS_NODISCARD_INLINE physx::PxPhysics* GetAPI_Physics() const
	{
		return Physics;
	}


	NS_NODISCARD_INLINE physx::PxCooking* GetAPI_Cooking() const
	{
		return Cooking;
	}

	

#ifdef __NS_ENGINE_DEBUG_DRAW__
public:
	void DebugDraw(class nsRenderContextWorld& renderContextWorld);
#endif // __NS_ENGINE_DEBUG_DRAW__

};
