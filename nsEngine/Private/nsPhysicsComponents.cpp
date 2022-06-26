#include "nsPhysicsComponents.h"
#include "nsPhysics_PhysX.h"
#include "nsWorld.h"
#include "nsMesh.h"
#include "nsConsole.h"



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
	CollisionChannels = nsEPhysicsCollisionChannel::Default | nsEPhysicsCollisionChannel::Character;
	bIsTrigger = false;
	bIsKinematic = false;
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
		PxRigidDynamic* rigidDynamic = physics->createRigidDynamic(NS_ToPxTransform(GetWorldTransform()));
		PhysicsActor = rigidDynamic;
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
	if (PhysicsActor == nullptr || !bAddedToLevel || !bStartedPlay)
	{
		return;
	}

	PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !bSimulatePhysics);
	PhysicsActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !bEnableGravity);

	if (PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>())
	{
		rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, bIsKinematic);
		
		if (bSimulatePhysics && !bIsKinematic)
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

	const PxTransform shapeGlobalPose = PxShapeExt::getGlobalPose(*PhysicsShape, *PhysicsActor);
	const PxTransform testAgaintsGlobalPose = PxShapeExt::getGlobalPose(*testAgaintsPhysicsShape, *testAgaintsPhysicsActor);

	PxVec3 direction;
	float depth;
	const bool bIsPenetrating = PxGeometryQuery::computePenetration(direction, depth, PhysicsShape->getGeometry().any(), shapeGlobalPose, testAgaintsPhysicsShape->getGeometry().any(), testAgaintsGlobalPose);

	if (bIsPenetrating)
	{
		nsTransform newTransform = Actor->GetWorldTransform();
		newTransform.Position += NS_FromPxVec3(direction) * depth;
		Actor->SetWorldTransform(newTransform);
	}

	return bIsPenetrating;
}


void nsCollisionComponent::SetKinematicTarget(nsTransform transform)
{
	if (PhysicsActor == nullptr || !bIsKinematic)
	{
		return;
	}

	if (PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>())
	{
		rigidDynamic->setGlobalPose(NS_ToPxTransform(transform));
		SetWorldTransform(transform);
	}
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
	const PxVec3 pxHalfExtent = NS_ToPxVec3(HalfExtent);

	if (PhysicsShape == nullptr)
	{
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxBoxGeometry(pxHalfExtent), *PhysicsMaterial);
	}
	else
	{
		PhysicsShape->setGeometry(PxBoxGeometry(pxHalfExtent));
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
	const PxTransform boxGlobalPose = PxShapeExt::getGlobalPose(*PhysicsShape, *PhysicsActor);

	return nsPhysX::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, boxGeometry, boxGlobalPose, NS_ToPxVec3(direction), distance, params);
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

	// By default capsule will extend along the Y-axis (rotate local)
	PxTransform relativePose(PxQuat(NS_MATH_PI_2, PxVec3(0.0f, 0.0f, 1.0f)));
	PhysicsShape->setLocalPose(relativePose);
}


bool nsCapsuleCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsShape == nullptr)
	{
		return false;
	}

	PxCapsuleGeometry capsuleGeometry;
	PhysicsShape->getCapsuleGeometry(capsuleGeometry);
	const PxTransform capsuleGlobalPose = PxShapeExt::getGlobalPose(*PhysicsShape, *PhysicsActor);

	return nsPhysX::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, capsuleGeometry, capsuleGlobalPose, NS_ToPxVec3(direction), distance, params);
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
	const PxTransform convexGlobalPose = PxShapeExt::getGlobalPose(*PhysicsShape, *PhysicsActor);

	return nsPhysX::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, convexMeshGeometry, convexGlobalPose, NS_ToPxVec3(direction), distance, params);
}




// ================================================================================================================================== //
// CHARACTER MOVEMENT COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsCharacterMovementComponent, nsCollisionComponent);

nsCharacterMovementComponent::nsCharacterMovementComponent()
{
	ObjectChannel = nsEPhysicsCollisionChannel::Character;
	bIsKinematic = true;
	CapsuleHeight = 120.0f;
	CapsuleRadius = 36.0f;
	ContactOffset = 3.0f;
	Acceleration = 300.0f;
	Deceleration = 240.0f;
	MaxSpeed = 300.0f;
	SlopeLimit = 40.0f;
	StepHeight = 25.0f;
	Velocity = nsVector3::ZERO;
	bIsOnGround = false;
}


void nsCharacterMovementComponent::OnInitialize()
{
	nsCollisionComponent::OnInitialize();

	PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>();
	if (rigidDynamic == nullptr)
	{
		NS_CONSOLE_Error(nsComponentLog, "nsCharacterMovementComponent requires dynamic actor (not marked as static)!");
		return;
	}
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

	// By default capsule will extend along the Y-axis (rotate local)
	PxTransform relativePose(PxQuat(NS_MATH_PI_2, PxVec3(0.0f, 0.0f, 1.0f)));
	PhysicsShape->setLocalPose(relativePose);
}


bool nsCharacterMovementComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsShape == nullptr)
	{
		return false;
	}

	PxCapsuleGeometry capsuleGeometry;
	PhysicsShape->getCapsuleGeometry(capsuleGeometry);
	const PxTransform capsuleGlobalPose = PxShapeExt::getGlobalPose(*PhysicsShape, *PhysicsActor);

	return nsPhysX::SceneQuerySweep(GetWorld()->GetPhysicsScene(), hitResult, capsuleGeometry, capsuleGlobalPose, NS_ToPxVec3(direction), distance, params);
}


void nsCharacterMovementComponent::SetupCapsule(float height, float radius)
{
	CapsuleHeight = height;
	CapsuleRadius = radius;
	UpdateCollisionShape();
}


bool nsCharacterMovementComponent::SweepCapsuleMovement(const nsTransform& worldTransform, const nsVector3& movement)
{
	nsVector3 direction = movement - worldTransform.Position;
	const float distance = direction.GetMagnitude();
	direction.Normalize();

	const PxCapsuleGeometry capsuleGeometry(CapsuleRadius, CapsuleHeight * 0.5f);
	const PxTransform capsuleGlobalPose = NS_ToPxTransform(worldTransform) * PhysicsShape->getLocalPose();

	nsPhysicsQueryParams queryParams;
	queryParams.Channel = nsEPhysicsCollisionChannel::Character;
	queryParams.IgnoredActors.Add(Actor);

	MoveHitResultMany.Clear();

	return nsPhysX::SceneQuerySweepMany(GetWorld()->GetPhysicsScene(), MoveHitResultMany, capsuleGeometry, capsuleGlobalPose, NS_ToPxVec3(direction), distance, queryParams);
}


void nsCharacterMovementComponent::ResolveCollision(nsTransform& outTransform)
{
	const PxCapsuleGeometry capsuleGeometryInflated(CapsuleRadius + ContactOffset, CapsuleHeight * 0.5f);

	for (int i = 0; i < MoveHitResultMany.GetCount(); ++i)
	{
		const PxTransform capsuleGlobalPose = NS_ToPxTransform(outTransform) * PhysicsShape->getLocalPose();
		const nsPhysicsHitResult& hit = MoveHitResultMany[i];

		nsCollisionComponent* collisionComp = ns_Cast<nsCollisionComponent>(hit.Component);
		NS_Assert(collisionComp);

		//NS_CONSOLE_Debug(nsComponentLog, "Hit actor [%s] [%s]", *collisionComp->GetActor()->Name, hit.bIsBlock ? "BLOCK" : "TOUCH");

		PxRigidActor* otherActor = collisionComp->Internal_GetPhysicsActor();
		NS_Assert(otherActor);

		PxShape* otherShape = collisionComp->Internal_GetPhysicsShape();
		NS_Assert(otherShape);

		const PxTransform otherShapeTransform = PxShapeExt::getGlobalPose(*otherShape, *otherActor);
		PxVec3 direction;
		float depth;

		if (PxGeometryQuery::computePenetration(direction, depth, capsuleGeometryInflated, capsuleGlobalPose, otherShape->getGeometry().any(), otherShapeTransform))
		{
			outTransform.Position += NS_FromPxVec3(direction) * (depth - ContactOffset);
		}
	}
}


void nsCharacterMovementComponent::Move(float deltaTime, const nsVector3& worldDirection)
{
	nsTransform actorTransform = Actor->GetWorldTransform();
	PxTransform rigidBodyTransform = PhysicsActor->getGlobalPose();
	
	nsPhysicsQueryParams queryParams;
	queryParams.Channel = nsEPhysicsCollisionChannel::Character;
	queryParams.IgnoredActors.Add(Actor);


	// Apply forward/right movement
	if (!worldDirection.IsZero())
	{
		const nsVector3 forwardRightMovement = worldDirection * MaxSpeed * deltaTime;

		if (SweepCapsuleMovement(actorTransform, forwardRightMovement))
		{
			actorTransform.Position += forwardRightMovement;
			ResolveCollision(actorTransform);
		}
		else
		{
			actorTransform.Position += forwardRightMovement;
		}
	}


	// apply down (gravity) movement
	if (bEnableGravity)
	{
		const nsVector3 gravityMovement = -nsVector3::UP * 980.0f * deltaTime;

		if (SweepCapsuleMovement(actorTransform, gravityMovement))
		{
			actorTransform.Position += gravityMovement;
			ResolveCollision(actorTransform);
		}
		else
		{
			actorTransform.Position += gravityMovement;
		}
	}

	
	if (PxRigidDynamic* rigidDynamic = PhysicsActor->is<PxRigidDynamic>())
	{
		rigidDynamic->setKinematicTarget(NS_ToPxTransform(actorTransform));
		SetWorldTransform(actorTransform);
	}
}