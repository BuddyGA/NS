#include "nsPhysicsComponents.h"
#include "nsPhysicsManager.h"
#include "nsPhysics_PhysX.h"
#include "nsWorld.h"
#include "nsMesh.h"
#include "nsConsole.h"



// ================================================================================================================================== //
// COLLISION COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsCollisionComponent, nsTransformComponent)
NS_CLASS_END(nsCollisionComponent)

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

	if (Actor->IsStatic() && bIsKinematic)
	{
		NS_CONSOLE_Warning(nsComponentLog, "nsCollisionComponent: Kinematic collision type requires dynamic actor! [Actor:%s]", *Actor->Name);
	}

	UpdateCollisionVolume();
}


void nsCollisionComponent::OnDestroy()
{
	if (PhysicsActor)
	{
		PhysicsActor->release();
		PhysicsActor = nullptr;
		PhysicsShape = nullptr;
	}

	PhysicsMaterial = nullptr;

	nsTransformComponent::OnDestroy();
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


void nsCollisionComponent::OnTransformChanged(bool bPhysicsSync)
{
	if (PhysicsActor == nullptr || bPhysicsSync)
	{
		return;
	}

	PhysicsActor->setGlobalPose(NS_ToPxTransform(GetWorldTransform()));
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

#ifdef NS_ENGINE_DEBUG_DRAW
	shapeFlags = PxShapeFlag::eVISUALIZATION;
#endif // NS_ENGINE_DEBUG_DRAW

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
		rigidDynamic->setKinematicTarget(NS_ToPxTransform(transform));
		SetWorldTransform(transform);
	}
}




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsBoxCollisionComponent, nsCollisionComponent)
NS_CLASS_END(nsBoxCollisionComponent)

nsBoxCollisionComponent::nsBoxCollisionComponent()
{
	HalfExtents = nsVector3(50.0f);
}


void nsBoxCollisionComponent::UpdateCollisionShape()
{
	PxVec3 pxHalfExtents = NS_ToPxVec3(HalfExtents);
	pxHalfExtents.x += 1.0f;
	pxHalfExtents.y += 1.0f;
	pxHalfExtents.z += 1.0f;

	if (PhysicsShape == nullptr)
	{
		PhysicsShape = PxRigidActorExt::createExclusiveShape(*PhysicsActor, PxBoxGeometry(pxHalfExtents), *PhysicsMaterial);
	}
	else
	{
		PhysicsShape->setGeometry(PxBoxGeometry(pxHalfExtents));
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
NS_CLASS_BEGIN(nsCapsuleCollisionComponent, nsCollisionComponent)
NS_CLASS_END(nsCapsuleCollisionComponent)

nsCapsuleCollisionComponent::nsCapsuleCollisionComponent()
{
	Radius = 36.0f;
	Height = 120.0f;
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


void nsCapsuleCollisionComponent::SetupCapsule(float radius, float height)
{
	Radius = radius;
	Height = height;
	UpdateCollisionShape();
}




// ================================================================================================================================== //
// CONVEX MESH COLLISION COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsConvexMeshCollisionComponent, nsCollisionComponent)
NS_CLASS_END(nsConvexMeshCollisionComponent)

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
		convexMeshDesc.vertexLimit = 32;

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
NS_CLASS_BEGIN(nsCharacterMovementComponent, nsCapsuleCollisionComponent)
NS_CLASS_END(nsCharacterMovementComponent)

nsCharacterMovementComponent::nsCharacterMovementComponent()
{
	ObjectChannel = nsEPhysicsCollisionChannel::Character;
	bIsKinematic = true;
	Radius = 36.0f;
	Height = 120.0f;
	ContactOffset = 3.0f;
	Acceleration = 300.0f;
	Deceleration = 240.0f;
	MaxSpeed = 300.0f;
	SlopeLimit = 40.0f;
	StepHeight = 25.0f;
	Velocity = nsVector3::ZERO;
	CurrentSlopeAngle = 0.0f;
	bIsOnGround = false;
}


bool nsCharacterMovementComponent::SweepMove(const nsTransform& worldTransform, const nsVector3& moveDirection)
{
	NS_Assert(!moveDirection.IsZero());

	const PxCapsuleGeometry capsuleGeometry(Radius, Height * 0.5f);
	const PxTransform capsuleGlobalPose = NS_ToPxTransform(worldTransform) * PhysicsShape->getLocalPose();

	nsPhysicsQueryParams queryParams;
	queryParams.Channel = nsEPhysicsCollisionChannel::Character;
	queryParams.IgnoredActors.Add(Actor);

	MoveHitResultMany.Clear();
	nsPhysX::SceneQuerySweepMany(GetWorld()->GetPhysicsScene(), MoveHitResultMany, capsuleGeometry, capsuleGlobalPose, NS_ToPxVec3(moveDirection.GetNormalized()), moveDirection.GetMagnitude() + ContactOffset, queryParams);

	return MoveHitResultMany.GetCount() > 0;
}


nsPhysicsHitResult nsCharacterMovementComponent::SweepMoveAndGetClosestHit(const nsTransform& worldTransform, const nsVector3& moveDirection)
{
	float distance = moveDirection.GetMagnitude();

	if (distance == 0.0f)
	{
		return nsPhysicsHitResult();
	}

	SweepMove(worldTransform, moveDirection);
	nsPhysicsHitResult closestHit;

	for (int hitIndex = 0; hitIndex < MoveHitResultMany.GetCount(); ++hitIndex)
	{
		const nsPhysicsHitResult& hit = MoveHitResultMany[hitIndex];

		if ((hit.Distance - ContactOffset) < distance)
		{
			distance = hit.Distance - ContactOffset;
			closestHit = hit;
		}
	}

	closestHit.Distance = distance;

	return closestHit;
}


nsVector3 nsCharacterMovementComponent::GetNewTargetPositionToSlideOnSurface(const nsVector3& currentPosition, const nsVector3& currentTargetPosition, const nsVector3& surfaceNormal)
{
	const nsVector3 direction = currentTargetPosition - currentPosition;
	const nsVector3 reflect = nsVector3::Reflect(direction, surfaceNormal);
	const nsVector3 project = nsVector3::Project(reflect, surfaceNormal);
	const nsVector3 tangent = reflect - project;

	return currentPosition + tangent.GetNormalized() * direction.GetMagnitude();
}


void nsCharacterMovementComponent::Move(float deltaTime, const nsVector3& worldDirection)
{
	nsTransform actorTransform = Actor->GetWorldTransform();
	

	// Apply side (forward/right) movement
	if (!worldDirection.IsZero())
	{
		nsVector3 currentMoveDirection = worldDirection * MaxSpeed * deltaTime;

		if (!SweepMove(actorTransform, currentMoveDirection))
		{
			actorTransform.Position += currentMoveDirection;
		}
		else
		{
			nsVector3 currentPosition = actorTransform.Position;
			nsVector3 currentTargetPosition = actorTransform.Position + currentMoveDirection;
			int iteration = 0;

			while (iteration < 4)
			{
				const nsPhysicsHitResult hit = SweepMoveAndGetClosestHit(actorTransform, currentMoveDirection);

				if (hit.Distance > 0.0f)
				{
					currentPosition += currentMoveDirection.GetNormalized() * hit.Distance;
					const float dot = nsVector3::DotProduct(currentMoveDirection.GetNormalized(), hit.WorldNormal);
					const float angle = nsMath::RadToDeg(nsMath::ACos(dot));
					//NS_CONSOLE_Debug(nsComponentLog, "Collide [dot: %f, angleDegree: %f]", dot, angle);
				}

				currentTargetPosition = GetNewTargetPositionToSlideOnSurface(currentPosition, currentTargetPosition, hit.WorldNormal);
				currentMoveDirection = currentTargetPosition - currentPosition;
				actorTransform.Position = currentPosition;

				++iteration;

				if (currentMoveDirection.GetMagnitudeSqr() < NS_MATH_EPS_LOW_P * NS_MATH_EPS_LOW_P)
				{
					break;
				} 
			}
		}
	}


	// apply down (gravity) movement
	if (bEnableGravity)
	{
		nsVector3 currentMoveDirection = -nsVector3::UP * 980.0f * deltaTime;

		if (!SweepMove(actorTransform, currentMoveDirection))
		{
			actorTransform.Position += currentMoveDirection;
		}
		else
		{
			nsVector3 currentPosition = actorTransform.Position;
			nsVector3 currentTargetPosition = actorTransform.Position + currentMoveDirection;
			int iteration = 0;

			while (iteration < 4)
			{
				const nsPhysicsHitResult hit = SweepMoveAndGetClosestHit(actorTransform, currentMoveDirection);

				if (hit.Distance > 0.0f)
				{
					currentPosition += currentMoveDirection.GetNormalized() * hit.Distance;
				}

				const float dotNormalUp = nsVector3::DotProduct(hit.WorldNormal, nsVector3::UP);
				const nsVector3 vertDir = dotNormalUp >= 0.0f ? nsVector3::UP : -nsVector3::UP;
				const float slopeAngleDegree = nsMath::RadToDeg(nsVector3::AngleBetween(hit.WorldNormal, vertDir));
				//NS_CONSOLE_Debug(nsComponentLog, "CharacterMovement: Down pass. [dotNormalUp: %f][slopeAngleDegree: %f]", dotNormalUp, slopeAngleDegree);

				if (dotNormalUp >= 0.0f)
				{
					currentTargetPosition = slopeAngleDegree > SlopeLimit ? GetNewTargetPositionToSlideOnSurface(currentPosition, currentTargetPosition, hit.WorldNormal) : currentPosition;
				}
				else
				{
					currentTargetPosition = GetNewTargetPositionToSlideOnSurface(currentPosition, currentTargetPosition, hit.WorldNormal);
				}

				currentMoveDirection = currentTargetPosition - currentPosition;
				actorTransform.Position = currentPosition;

				++iteration;

				if (currentMoveDirection.GetMagnitudeSqr() < NS_MATH_EPS_LOW_P * NS_MATH_EPS_LOW_P)
				{
					break;
				}
			}
		}
	}

	SetKinematicTarget(actorTransform);
}
