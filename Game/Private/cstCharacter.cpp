#include "cstCharacter.h"
#include "nsAssetManager.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"
#include "cstAttributeComponent.h"



NS_CLASS_BEGIN(cstCharacter, nsActor)
NS_CLASS_END(cstCharacter)

cstCharacter::cstCharacter()
{
	MovementComponent = AddComponent<nsCharacterMovementComponent>("movement");
	MovementComponent->CapsuleHeight = 120.0f;
	MovementComponent->CapsuleRadius = 36.0f;
	RootComponent = MovementComponent;

	SkelMeshComponent = AddComponent<nsSkeletalMeshComponent>("skeletal_mesh");
	SkelMeshComponent->bDebugDrawSkeleton = true;
	SkelMeshComponent->SetLocalPosition(nsVector3(0.0f, -90.0f, 0.0f));

	AttributeComponent = AddComponent<cstAttributeComponent>("attribute");
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	nsAssetManager& assetManager = nsAssetManager::Get();
	SkelMeshComponent->SetMesh(assetManager.LoadModelAsset("mdl_LowPolyChar"));
	SkelMeshComponent->SetSkeleton(assetManager.LoadSkeletonAsset("skl_LowPolyChar_Rig"));

	AnimRunForwardLoop = assetManager.LoadAnimationAsset("anim_run_forward_loop");
}


void cstCharacter::OnStartPlay()
{
	nsActor::OnStartPlay();

	SkelMeshComponent->PlayAnimation(AnimRunForwardLoop, 1.0f, true);
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::Move(float deltaTime, const nsVector3& worldDirection)
{
	MovementComponent->Move(deltaTime, worldDirection);
}
