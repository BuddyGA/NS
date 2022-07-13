#pragma once

#include "cstTypes.h"



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT()

private:
	class nsNavigationAgentComponent* NavigationAgentComponent;
	class nsSkeletalMeshComponent* SkelMeshComponent;
	class cstAttributeComponent* AttributeComponent;

	nsSharedAnimationAsset AnimRunForwardLoop;


public:
	cstCharacter();
	virtual void OnInitialize() override;
	virtual void OnStartPlay() override;
	virtual void OnDestroy() override;
	void SetMoveTargetPosition(const nsVector3& worldPosition);

};
