#include "nsPhysicsComponents.h"
#include "nsPhysics_PhysX.h"
#include "nsWorld.h"
#include "nsMesh.h"



// ================================================================================================================================== //
// COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsCollisionComponent, nsTransformComponent);

nsCollisionComponent::nsCollisionComponent()
{
	PhysicsActor = nullptr;
	PhysicsMaterial = nullptr;
	PhysicsShape = nullptr;
	CollisionTest = nsEPhysicsCollisionTest::COLLISION_AND_QUERY;
	ObjectChannel = nsEPhysicsCollisionChannel::Default;
	CollisionChannels = nsEPhysicsCollisionChannel::Default;
	bIsTrigger = false;
	bSimulatePhysics = true;
	bEnableGravity = true;
}


void nsCollisionComponent::OnInitialize()
{
	nsTransformComponent::OnInitialize();

	PhysicsMaterial = nsPhysicsManager::Get().GetDefaultMaterial();

	UpdateCollisionVolume();
}


void nsCollisionComponent::OnStartPlay()
{
	nsTransformComponent::OnStartPlay();

	UpdateCollisionActorSimulation();
}


void nsCollisionComponent::OnStopPlay()
{
	if (PhysicsActor)
	{
		PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
	}

	nsTransformComponent::OnStopPlay();
}


void nsCollisionComponent::OnDestroy()
{
	if (PhysicsActor)
	{
		PhysicsActor->release();
		PhysicsActor = nullptr;
	}

	if (PhysicsShape)
	{
		PhysicsShape->release();
		PhysicsShape = nullptr;
	}

	PhysicsMaterial = nullptr;

	nsTransformComponent::OnDestroy();
}


void nsCollisionComponent::OnStaticChanged()
{
	UpdateCollisionVolume();
}


void nsCollisionComponent::OnAddedToLevel()
{
	nsTransformComponent::OnAddedToLevel();

	PxScene* physicsScene = GetWorld()->GetPhysicsScene();

	if (PhysicsActor && physicsScene)
	{
		physicsScene->addActor(*PhysicsActor);
	}

	UpdateCollisionVolume();
}


void nsCollisionComponent::OnRemovedFromLevel()
{
	PxScene* physicsScene = GetWorld()->GetPhysicsScene();

	if (PhysicsActor && physicsScene)
	{
		physicsScene->removeActor(*PhysicsActor);
	}

	nsTransformComponent::OnRemovedFromLevel();
}


void nsCollisionComponent::OnTransformChanged()
{
	if (PhysicsActor)
	{
		PhysicsActor->setGlobalPose(NS_ToPxTransform(GetWorldTransform()));
	}
}


void nsCollisionComponent::UpdateCollisionActor()
{
	const bool bIsStatic = Actor->IsStatic();

	// Recreate physics actor if:
	// - PhysicsActor is null
	// - OR, PhysicsActor was static and actor set to dynamic (not static)
	// - OR, PhysicsActor was dynamic and actor set to static 
	const bool bShouldRecreate = (PhysicsActor == nullptr) || (PhysicsActor->is<PxRigidStatic>() && !bIsStatic) || (PhysicsActor->is<PxRigidDynamic>() && bIsStatic);

	if (!bShouldRecreate)
	{
		return;
	}

	// Only destroy physics actor if it was added to scene
	nsWorld* world = GetWorld();
	PxScene* scene = world ? world->GetPhysicsScene() : nullptr;

	if (scene && bAddedToLevel)
	{
		scene->removeActor(*PhysicsActor);

		PhysicsActor->release();
		PhysicsActor = nullptr;

		PhysicsShape->release();
		PhysicsShape = nullptr;
	}

	NS_Assert(PhysicsActor == nullptr);
	PxPhysics* physics = nsPhysicsManager::Get().GetAPI_Physics();

	if (bIsStatic)
	{
		PhysicsActor = physics->createRigidStatic(NS_ToPxTransform(GetWorldTransform()));
	}
	else
	{
		PhysicsActor = physics->createRigidDynamic(NS_ToPxTransform(GetWorldTransform()));
	}

	PhysicsActor->userData = this;

	if (scene && bAddedToLevel)
	{
		scene->addActor(*PhysicsActor);
	}

	if (!bStartedPlay)
	{
		PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
	}
}


void nsCollisionComponent::UpdateCollisionActorSimulation()
{
	if (PhysicsActor && bAddedToLevel && bStartedPlay)
	{
		PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !bSimulatePhysics);
		PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !bEnableGravity);
		PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>();

		if (rigidDynamic && (bSimulatePhysics && bEnableGravity))
		{
			PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, 10.0f);
			rigidDynamic->wakeUp();
		}
	}
}


void nsCollisionComponent::UpdateCollisionShapeFlags()
{
	if (PhysicsShape == nullptr)
	{
		return;
	}

	PxShapeFlags shapeFlags{};

#if __NS_ENGINE_DEBUG_DRAW__
	shapeFlags = PxShapeFlag::eVISUALIZATION;
#endif // __NS_ENGINE_DEBUG_DRAW__

	switch (CollisionTest)
	{
		case nsEPhysicsCollisionTest::COLLISION_ONLY:		shapeFlags |= PxShapeFlag::eSIMULATION_SHAPE; break;
		case nsEPhysicsCollisionTest::QUERY_ONLY:			shapeFlags |= PxShapeFlag::eSCENE_QUERY_SHAPE; break;
		case nsEPhysicsCollisionTest::COLLISION_AND_QUERY:	shapeFlags |= PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE; break;
		default: break;
	}

	if (bIsTrigger)
	{
		shapeFlags &= ~PxShapeFlag::eSIMULATION_SHAPE;
		shapeFlags |= PxShapeFlag::eTRIGGER_SHAPE;
	}

	PhysicsShape->setFlags(shapeFlags);
}


void nsCollisionComponent::UpdateCollisionShapeChannels()
{
	if (PhysicsShape == nullptr)
	{
		return;
	}

	PxFilterData simulationFilterData{};
	simulationFilterData.word0 = static_cast<PxU32>(ObjectChannel);
	simulationFilterData.word1 = static_cast<PxU32>(CollisionChannels);
	PhysicsShape->setSimulationFilterData(simulationFilterData);

	PxFilterData queryFilterData{};
	queryFilterData.word0 = static_cast<PxU32>(ObjectChannel);
	queryFilterData.word1 = static_cast<PxU32>(CollisionChannels);
	PhysicsShape->setQueryFilterData(queryFilterData);
}


void nsCollisionComponent::UpdateCollisionVolume()
{
	UpdateCollisionActor();
	UpdateCollisionActorSimulation();
	UpdateCollisionShape();
	UpdateCollisionShapeFlags();
	UpdateCollisionShapeChannels();
}


bool nsCollisionComponent::AdjustPositionIfOverlappedWith(nsActor* actorToTest)
{
	if (PhysicsActor == nullptr || PhysicsShape == nullptr)
	{
		return false;
	}

	nsCollisionComponent* testAgaintsCollisionComponent = actorToTest ? actorToTest->GetComponent<nsCollisionComponent>() : nullptr;
	PxRigidActor* testAgaintsPhysicsActor = testAgaintsCollisionComponent ? testAgaintsCollisionComponent->PhysicsActor : nullptr;
	PxShape* testAgaintsPhysicsShape = testAgaintsCollisionComponent ? testAgaintsCollisionComponent->PhysicsShape : nullptr;

	if (testAgaintsPhysicsShape == nullptr)
	{
		return false;
	}

	PxVec3 direction;
	float depth;
	const bool bIsPenetrating = PxGeometryQuery::computePenetration(direction, depth, PhysicsShape->getGeometry().any(), PhysicsActor->getGlobalPose(), testAgaintsPhysicsShape->getGeometry().any(), testAgaintsPhysicsActor->getGlobalPose());

	if (bIsPenetrating)
	{
		nsTransform newTransform = Actor->GetWorldTransform();
		newTransform.Position = newTransform.Position + NS_FromPxVec3(direction) * depth;
		Actor->SetWorldTransform(newTransform);
	}

	return bIsPenetrating;
}




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsBoxCollisionComponent, nsCollisionComponent);

nsBoxCollisionComponent::nsBoxCollisionComponent()
{
	HalfExtent = nsVector3(50.0f);
}


void nsBoxCollisionComponent::UpdateCollisionShape()
{
	if (PhysicsShape == nullptr)
	{
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxBoxGeometry(NS_ToPxVec3(HalfExtent)), *PhysicsMaterial);
	}
	else
	{
		PhysicsShape->setGeometry(PxBoxGeometry(NS_ToPxVec3(HalfExtent)));
	}
}


bool nsBoxCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsShape == nullptr)
	{
		return false;
	}

	PxBoxGeometry boxGeometry;
	PhysicsShape->getBoxGeometry(boxGeometry);

	return nsPhysXHelper::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, boxGeometry, GetWorldTransform(), direction, distance, params);
}




// ================================================================================================================================== //
// CAPSULE COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsCapsuleCollisionComponent, nsCollisionComponent);

nsCapsuleCollisionComponent::nsCapsuleCollisionComponent()
{
	Height = 120.0f;
	Radius = 36.0f;
}


void nsCapsuleCollisionComponent::UpdateCollisionShape()
{
	if (PhysicsShape == nullptr)
	{
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxCapsuleGeometry(Radius, Height * 0.5f), *PhysicsMaterial);
	}
	else
	{
		PhysicsShape->setGeometry(PxCapsuleGeometry(Radius, Height * 0.5f));
	}

	PhysicsShape->setLocalPose(PxTransformFromSegment(PxVec3(0.0f), PxVec3(0.0f, 1.0f, 0.0f)));
}


bool nsCapsuleCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsShape == nullptr)
	{
		return false;
	}

	PxCapsuleGeometry capsuleGeometry;
	PhysicsShape->getCapsuleGeometry(capsuleGeometry);

	return nsPhysXHelper::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, capsuleGeometry, GetWorldTransform(), direction, distance, params);
}




// ================================================================================================================================== //
// CONVEX MESH COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsConvexMeshCollisionComponent, nsCollisionComponent);

nsConvexMeshCollisionComponent::nsConvexMeshCollisionComponent()
{
	Mesh = nsMeshID::INVALID;
}


void nsConvexMeshCollisionComponent::UpdateCollisionShape()
{
	if (Mesh == nsMeshID::INVALID)
	{
		return;
	}

	if (PhysicsShape == nullptr)
	{
		const nsTArray<nsVertexMeshPosition>& vertices = nsMeshManager::Get().GetMeshVertexData(Mesh, 0).Positions;

		PxConvexMeshDesc convexMeshDesc{};
		convexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		convexMeshDesc.points.stride = sizeof(nsVertexMeshPosition);
		convexMeshDesc.points.data = vertices.GetData();
		convexMeshDesc.points.count = static_cast<PxU32>(vertices.GetCount());
		convexMeshDesc.vertexLimit = 16;

		PxDefaultMemoryOutputStream buffer;
		PxConvexMeshCookingResult::Enum result;

		PxCooking* cooking = nsPhysicsManager::Get().GetAPI_Cooking();
		const bool bSuccess = cooking->cookConvexMesh(convexMeshDesc, buffer, &result);
		NS_ValidateV(bSuccess, "Cooking convex mesh failed!");

		PxDefaultMemoryInputData input(buffer.getData(), buffer.getSize());
		PxConvexMesh* convexMesh = nsPhysicsManager::Get().GetAPI_Physics()->createConvexMesh(input);
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxConvexMeshGeometry(convexMesh), *PhysicsMaterial);
		convexMesh->release();
	}
}


bool nsConvexMeshCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsShape == nullptr)
	{
		return false;
	}

	PxConvexMeshGeometry convexMeshGeometry;
	PhysicsShape->getConvexMeshGeometry(convexMeshGeometry);

	return nsPhysXHelper::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, convexMeshGeometry, GetWorldTransform(), direction, distance, params);
}




// ================================================================================================================================== //
// CHARACTER MOVEMENT COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsCharacterMovementComponent, nsCollisionComponent);

nsCharacterMovementComponent::nsCharacterMovementComponent()
{
	ObjectChannel = nsEPhysicsCollisionChannel::Character;
	
	CapsuleHeight = 120.0f;
	CapsuleRadius = 36.0f;
	ContactOffset = 3.0f;
	WalkSlopeLimit = 50.0f;
	WalkStepHeight = 50.0f;
}


void nsCharacterMovementComponent::OnInitialize()
{
	nsCollisionComponent::OnInitialize();

	PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>();
	NS_AssertV(rigidDynamic, "Character movement component requires dynamic (not static) actor!");

	rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
}


void nsCharacterMovementComponent::UpdateCollisionShape()
{
	if (PhysicsShape == nullptr)
	{
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxCapsuleGeometry(CapsuleRadius, CapsuleHeight * 0.5f), *PhysicsMaterial);
	}
	else
	{
		PhysicsShape->setGeometry(PxCapsuleGeometry(CapsuleRadius, CapsuleHeight * 0.5f));
	}

	PhysicsShape->setLocalPose(PxTransformFromSegment(PxVec3(0.0f), PxVec3(0.0f, 1.0f, 0.0f)));
}


bool nsCharacterMovementComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return false;
}


void nsCharacterMovementComponent::Move(float deltaTime, const nsVector3& worldDirection)
{

}
