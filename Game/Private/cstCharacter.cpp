#include "cstCharacter.h"
#include "nsAssetManager.h"



NS_CLASS_BEGIN(cstCharacter, nsActor)
NS_CLASS_END(cstCharacter)

cstCharacter::cstCharacter()
{
	MovementComponent = AddComponent<nsCharacterMovementComponent>("movement");
	MovementComponent->CapsuleHeight = 120.0f;
	MovementComponent->CapsuleRadius = 36.0f;
	RootComponent = MovementComponent;

	MeshComponent = AddComponent<nsSkeletalMeshComponent>("skeletal_mesh");
	MeshComponent->SetLocalPosition(nsVector3(0.0f, -90.0f, 0.0f));
	//MeshComponent->SetLocalRotation(nsQuaternion::FromRotation(0.0f, 180.0f, 0.0f));
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	nsAssetManager& assetManager = nsAssetManager::Get();
	MeshComponent->SetMesh(assetManager.LoadModelAsset("mdl_LowPolyChar"));
	MeshComponent->SetSkeleton(assetManager.LoadSkeletonAsset("skl_LowPolyChar_Rig"));
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::Move(float deltaTime, const nsVector3& worldDirection)
{
	MovementComponent->Move(deltaTime, worldDirection);
}
