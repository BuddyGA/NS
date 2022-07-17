#pragma once

#include "cstTypes.h"



enum class cstECharacterTeam : uint8
{
	NONE = 0,
	PLAYER,
	NPC
};



enum class cstECharacterState : uint8
{
	NONE = 0,

	IDLE,
	MOVING,
	EXECUTING_ABILITY,
	KO,
};



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT(cstCharacter)

protected:
	class nsNavigationAgentComponent* NavigationAgentComponent;
	class nsSkeletalMeshComponent* SkelMeshComponent;

	cstECharacterState PendingChangeState;
	cstECharacterState CurrentState;

	cstAttributes Attributes;
	cstCharacterStatusFlags StatusFlags;
	cstECharacterTeam Team;

	nsSharedAnimationAsset AnimIdle0;
	nsSharedAnimationAsset AnimRunForwardLoop;

	nsVector3 MoveTargetPosition;
	cstCharacter* AttackTargetCharacter;
	float MoveDistanceToTarget;

	cstAbility* ExecutingAbility;


public:
	cstCharacter();


// Begin actor interfaces //
protected:
	virtual void OnInitialize() override;
	virtual void OnDestroy() override;
	virtual void OnStartPlay() override;
	virtual void OnStopPlay() override;
	virtual void OnTickUpdate(float deltaTime) override;
// End actor interfaces //

	virtual void UpdateAnimation();


public:
	void SetMoveTargetPosition(const nsVector3& worldPosition);
	void StartExecuteAbility(cstAbility* ability);
	void StopAction();


	NS_NODISCARD_INLINE cstAttributes& GetAttributes()
	{
		return Attributes;
	}


	NS_NODISCARD_INLINE const cstAttributes& GetAttributes() const
	{
		return Attributes;
	}


	NS_NODISCARD_INLINE cstCharacterStatusFlags GetStatusFlags() const
	{
		return StatusFlags;
	}


	NS_NODISCARD_INLINE cstECharacterTeam GetTeam() const
	{
		return Team;
	}


	NS_NODISCARD_INLINE bool IsAlive() const
	{
		return Attributes.Health > 0.0f;
	}


	NS_NODISCARD_INLINE bool CanMove() const
	{
		return IsAlive() && (ExecutingAbility == nullptr) && !(StatusFlags & cstECharacterStatus::Stun);
	}


#ifdef CST_GAME_WITH_EDITOR
public:
	void DebugGUI(nsGUIContext& context);
	void DebugDraw(nsRenderer* renderer);
#endif // CST_GAME_WITH_EDITOR

};




class cstPlayerCharacter : public cstCharacter
{
	NS_DECLARE_OBJECT(cstPlayerCharacter)

public:
	cstPlayerCharacter();

};
