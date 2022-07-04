#pragma once

#include "cstTypes.h"



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT()

private:
	class nsCharacterMovementComponent* MovementComponent;
	class nsSkeletalMeshComponent* SkelMeshComponent;
	class cstAttributeComponent* AttributeComponent;

	nsSharedAnimationAsset AnimRunForwardLoop;


public:
	cstCharacter();
	virtual void OnInitialize() override;
	virtual void OnStartPlay() override;
	virtual void OnDestroy() override;
	void Move(float deltaTime, const nsVector3& worldDirection);


	NS_NODISCARD_INLINE cstAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

};
