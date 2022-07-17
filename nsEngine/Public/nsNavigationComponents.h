#pragma once

#include "nsPhysicsComponents.h"
#include "nsNavigationTypes.h"



class NS_ENGINE_API nsNavigationAgentComponent : public nsCapsuleCollisionComponent
{
	NS_DECLARE_OBJECT(nsNavigationAgentComponent)

private:
	nsNavigationAgentID NavAgentId;
	nsVector3 CurrentVelocity;
	nsVector3 DesiredVelocity;

public:
	float MaxAcceleration;
	float MaxSpeed;
	float MinDistanceToMove;


public:
	nsNavigationAgentComponent();
	virtual void OnDestroy() override;
	virtual void OnStartPlay() override;
	virtual void OnStopPlay() override;
	virtual void OnPhysicsTickUpdate(float deltaTime) override;
	void SetNavigationTarget(const nsVector3& worldPosition);
	void Internal_SyncWithDetourCrowdAgent(float deltaTime, const nsVector3& navigationPosition, const nsVector3& currentVelocity, const nsVector3& desiredVelocity);

private:
	void RegisterAgent();
	void UnregisterAgent();


public:
	NS_NODISCARD_INLINE const nsVector3& GetCurrentVelocity() const
	{
		return CurrentVelocity;
	}


	NS_NODISCARD_INLINE const nsVector3& GetDesiredVelocity() const
	{
		return DesiredVelocity;
	}

};
