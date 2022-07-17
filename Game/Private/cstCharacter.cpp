#include "cstCharacter.h"
#include "nsAssetManager.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"
#include "nsNavigationComponents.h"
#include "cstAttributeComponent.h"



NS_CLASS_BEGIN(cstCharacter, nsActor)
NS_CLASS_END(cstCharacter)

cstCharacter::cstCharacter()
{
	Flags |= nsEActorFlag::CallPrePhysicsTickUpdate;

	NavigationAgentComponent = AddComponent<nsNavigationAgentComponent>("nav_agent");
	NavigationAgentComponent->MaxAcceleration = 1200.0f;
	NavigationAgentComponent->MaxSpeed = 300.0f;
	RootComponent = NavigationAgentComponent;

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

	AnimIdle0 = assetManager.LoadAnimationAsset("anim_idle_0");
	AnimRunForwardLoop = assetManager.LoadAnimationAsset("anim_run_forward_loop");
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::OnStartPlay()
{
	nsActor::OnStartPlay();

}


void cstCharacter::OnTickUpdate(float deltaTime)
{
	nsActor::OnTickUpdate(deltaTime);
	
	const float currentSpeed = NavigationAgentComponent->GetCurrentVelocity().GetMagnitude();
	
	if (currentSpeed > 30.0f)
	{
		SkelMeshComponent->PlayAnimation(AnimRunForwardLoop, 1.0f, true);
	}
	else
	{
		SkelMeshComponent->PlayAnimation(AnimIdle0, 1.0f, true);
	}
}


void cstCharacter::SetMoveTargetPosition(const nsVector3& worldPosition)
{
	NavigationAgentComponent->SetNavigationTarget(worldPosition);
}
