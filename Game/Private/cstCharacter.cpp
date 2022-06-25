#include "cstCharacter.h"
#include "nsAssetManager.h"



NS_DEFINE_OBJECT(cstCharacter, nsActor);

cstCharacter::cstCharacter()
{
	/*
	CapsuleComponent = AddComponent<nsCapsuleCollisionComponent>("capsule_collision");
	CapsuleComponent->Height = 120.0f;
	CapsuleComponent->Radius = 36.0f;
	CapsuleComponent->SetObjectChannel(nsEPhysicsCollisionChannel::Character);
	CapsuleComponent->SetCollisionChannels(nsEPhysicsCollisionChannel::Default | nsEPhysicsCollisionChannel::Character);
	CapsuleComponent->SetAsKinematic(true);
	RootComponent = CapsuleComponent;
	*/

	MovementComponent = AddComponent<nsCharacterMovementComponent>("movement");
	MovementComponent->CapsuleHeight = 120.0f;
	MovementComponent->CapsuleRadius = 36.0f;
	RootComponent = MovementComponent;

	MeshComponent = AddComponent<nsMeshComponent>("mesh");
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	MeshComponent->SetMesh(nsAssetManager::Get().LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME));
}


void cstCharacter::OnDestroy()
{
	MoveHitResults.Clear(true);

	nsActor::OnDestroy();
}


void cstCharacter::Move(float deltaTime, const nsVector3& worldDirection)
{
	MovementComponent->Move(deltaTime, worldDirection);

	/*
	const nsVector3 GRAVITY(0.0f, -980.0f, 0.0f);

	const float moveSpeed = static_cast<float>(Attributes.MSPD);
	


	nsTransform transform = GetWorldTransform();

	// apply gravity
	transform.Position += GRAVITY * deltaTime;

	nsPhysicsQueryParams queryParams;
	queryParams.IgnoredActors.Add(this);
	MoveHitResults.Clear();

	if (CapsuleComponent->SweepTestMany(MoveHitResults, -nsVector3::UP, 980.0f * deltaTime, queryParams))
	{
		for (int i = 0; i < MoveHitResults.GetCount(); ++i)
		{
		}
	}

	MoveHitResults.Clear();

	if (CapsuleComponent->SweepTestMany(MoveHitResults, worldDirection, moveSpeed * deltaTime, queryParams))
	{
		NS_CONSOLE_Debug(cstCharacterLog, "Collide with [%s]", *hitResult.Actor->Name);
	}

	// apply move
	transform.Position += worldDirection * moveSpeed * deltaTime;
	
	CapsuleComponent->SetKinematicTarget(transform);
	*/
}
