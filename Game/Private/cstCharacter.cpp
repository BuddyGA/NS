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
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	MeshComponent->SetMesh(nsAssetManager::Get().LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME));
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::Move(float deltaTime, const nsVector3& worldDirection)
{
	MovementComponent->Move(deltaTime, worldDirection);
}
