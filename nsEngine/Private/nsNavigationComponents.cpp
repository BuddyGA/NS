#include "nsNavigationComponents.h"
#include "nsNavigationManager.h"



NS_CLASS_BEGIN(nsNavigationAgentComponent, nsCapsuleCollisionComponent)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, float, MaxAcceleration, true)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, float, MaxSpeed, true)
NS_CLASS_END(nsNavigationAgentComponent)

nsNavigationAgentComponent::nsNavigationAgentComponent()
{
	bIsKinematic = true;
	NavAgentId = nsNavigationAgentID::INVALID;
	MaxAcceleration = 3000.0f;
	MaxSpeed = 600.0f;
	MinDistanceToMove = 64.0f;
}


void nsNavigationAgentComponent::OnDestroy()
{
	UnregisterAgent();

	nsCapsuleCollisionComponent::OnDestroy();
}


void nsNavigationAgentComponent::OnStartPlay()
{
	nsCapsuleCollisionComponent::OnStartPlay();

	nsNavigationManager::Get().SetAgentActive(NavAgentId, true, this);
}


void nsNavigationAgentComponent::OnStopPlay()
{
	nsNavigationManager::Get().SetAgentActive(NavAgentId, false, nullptr);

	nsCapsuleCollisionComponent::OnStopPlay();
}


void nsNavigationAgentComponent::OnAddedToLevel()
{
	nsCapsuleCollisionComponent::OnAddedToLevel();

	RegisterAgent();
}


void nsNavigationAgentComponent::OnRemovedFromLevel()
{
	UnregisterAgent();

	nsCapsuleCollisionComponent::OnRemovedFromLevel();
}


void nsNavigationAgentComponent::SetNavigationTarget(const nsVector3& worldPosition)
{
	if (!NavAgentId.IsValid())
	{
		return;
	}

	const float distance = nsVector3::Distance(GetWorldPosition(), worldPosition);

	if (distance >= MinDistanceToMove)
	{
		nsNavigationManager::Get().SetAgentMoveTarget(NavAgentId, worldPosition);
	}
}


void nsNavigationAgentComponent::RegisterAgent()
{
	if (!bAddedToLevel)
	{
		return;
	}

	nsNavigationManager& navigationManager = nsNavigationManager::Get();

	if (NavAgentId == nsNavigationAgentID::INVALID)
	{
		NavAgentId = navigationManager.CreateAgent(Radius, Height, MaxAcceleration, MaxSpeed);
	}
	else
	{
		navigationManager.UpdateAgentParams(NavAgentId, Radius, Height, MaxAcceleration, MaxSpeed);
	}
}


void nsNavigationAgentComponent::UnregisterAgent()
{
	if (NavAgentId == nsNavigationAgentID::INVALID)
	{
		return;
	}

	nsNavigationManager::Get().DestroyAgent(NavAgentId);
}
