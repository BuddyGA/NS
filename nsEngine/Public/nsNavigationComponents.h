#pragma once

#include "nsPhysicsComponents.h"
#include "nsNavigationTypes.h"



class NS_ENGINE_API nsNavigationAgentComponent : public nsCapsuleCollisionComponent
{
	NS_DECLARE_OBJECT()

private:
	nsNavigationAgentID NavAgentId;

public:
	float MaxAcceleration;
	float MaxSpeed;
	float MinDistanceToMove;


public:
	nsNavigationAgentComponent();
	virtual void OnDestroy() override;
	virtual void OnStartPlay() override;
	virtual void OnStopPlay() override;
	virtual void OnAddedToLevel() override;
	virtual void OnRemovedFromLevel() override;
	void SetNavigationTarget(const nsVector3& worldPosition);

private:
	void RegisterAgent();
	void UnregisterAgent();


public:
	NS_INLINE void SyncWithNavigationPosition(const nsVector3& navigationPosition)
	{
		nsTransform newTransform = GetWorldTransform();
		newTransform.Position = navigationPosition;
		newTransform.Position.Y += Radius + Height * 0.5f;
		SetKinematicTarget(newTransform);
	}

};
