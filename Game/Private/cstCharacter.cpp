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

	MeshComponent = AddComponent<nsMeshComponent>("mesh");
	MeshComponent->SetLocalPosition(nsVector3(0.0f, -90.0f, 0.0f));
	MeshComponent->SetLocalRotation(nsQuaternion::FromRotation(0.0f, 180.0f, 0.0f));
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	MeshComponent->SetMesh(nsAssetManager::Get().LoadModelAsset("mdl_LowPolyChar"));
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::Move(float deltaTime, const nsVector3& worldDirection)
{
	MovementComponent->Move(deltaTime, worldDirection);
}
