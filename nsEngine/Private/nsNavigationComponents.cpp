#include "nsNavigationComponents.h"
#include "nsNavigationManager.h"



NS_CLASS_BEGIN(nsNavigationAgentComponent, nsCapsuleCollisionComponent)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, float, MaxAcceleration, true)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, float, MaxSpeed, true)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, nsVector3, CurrentVelocity, true)
	NS_CLASS_AddProperty(nsNavigationAgentComponent, nsVector3, DesiredVelocity, true)
NS_CLASS_END(nsNavigationAgentComponent)

nsNavigationAgentComponent::nsNavigationAgentComponent()
{
	bIsKinematic = true;
	NavAgentId = nsNavigationAgentID::INVALID;
	MaxAcceleration = 2400.0f;
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

	RegisterAgent();
}


void nsNavigationAgentComponent::OnStopPlay()
{
	UnregisterAgent();

	nsCapsuleCollisionComponent::OnStopPlay();
}


void nsNavigationAgentComponent::OnPhysicsTickUpdate(float deltaTime)
{
	nsCapsuleCollisionComponent::OnPhysicsTickUpdate(deltaTime);
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


void nsNavigationAgentComponent::StopMovement()
{
	if (!NavAgentId.IsValid())
	{
		return;
	}

	nsNavigationManager::Get().StopAgentMovement(NavAgentId);
}


void nsNavigationAgentComponent::Internal_SyncWithDetourCrowdAgent(float deltaTime, const nsVector3& navigationPosition, const nsVector3& currentVelocity, const nsVector3& desiredVelocity)
{
	nsTransform newTransform = GetWorldTransform();
	newTransform.Position = navigationPosition;
	newTransform.Position.Y += Radius + Height * 0.5f;

	const float speed = currentVelocity.GetMagnitude();

	if (speed > 10.0f)
	{
		const nsQuaternion actorRotation = nsQuaternion::FromVectors(nsVector3::FORWARD, newTransform.GetAxisForward());
		const nsQuaternion newRotation = nsQuaternion::FromVectors(nsVector3::FORWARD, currentVelocity.GetNormalized());
		newTransform.Rotation = nsQuaternion::Slerp(actorRotation, newRotation, deltaTime);
	}

	SetKinematicTarget(newTransform);

	CurrentVelocity = currentVelocity;
	DesiredVelocity = desiredVelocity;
}


void nsNavigationAgentComponent::RegisterAgent()
{
	if (!bAddedToLevel)
	{
		return;
	}

	nsNavigationManager& navigationManager = nsNavigationManager::Get();

	if (NavAgentId.IsValid())
	{
		navigationManager.UpdateAgentParams(NavAgentId);
	}
	else
	{
		NavAgentId = navigationManager.CreateAgent(this);
	}
}


void nsNavigationAgentComponent::UnregisterAgent()
{
	if (!NavAgentId.IsValid())
	{
		return;
	}

	nsNavigationManager::Get().DestroyAgent(NavAgentId);
}
