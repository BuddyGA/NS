#pragma once

#include "cstTypes.h"



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT(cstCharacter)

private:
	class nsNavigationAgentComponent* NavigationAgentComponent;
	class nsSkeletalMeshComponent* SkelMeshComponent;
	class cstAttributeComponent* AttributeComponent;

	nsSharedAnimationAsset AnimIdle0;
	nsSharedAnimationAsset AnimRunForwardLoop;


public:
	cstCharacter();


// Begin actor interfaces //
protected:
	virtual void OnInitialize() override;
	virtual void OnDestroy() override;
	virtual void OnStartPlay() override;
	virtual void OnTickUpdate(float deltaTime) override;
// End actor interfaces //


public:
	void SetMoveTargetPosition(const nsVector3& worldPosition);

};
