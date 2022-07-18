#pragma once

#include "cstAbility.h"
#include "cstInput.h"



enum class cstECharacterTeam : uint8
{
	NEUTRAL = 0,
	PLAYER,
	ENEMY
};



enum class cstECharacterState : uint8
{
	NONE = 0,
	IDLE,
	MOVING,
	EXECUTING,
	KO,
};



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT(cstCharacter)

private:
	class nsNavigationAgentComponent* NavigationAgentComponent;
	class nsSkeletalMeshComponent* SkelMeshComponent;

	cstECharacterState PendingChangeState;
	cstECharacterState CurrentState;

	nsSharedAnimationAsset AnimIdle0;
	nsSharedAnimationAsset AnimRunForwardLoop;

	nsVector3 MoveTargetPosition;
	cstCharacter* AttackTargetCharacter;
	float MoveDistanceToTarget;

	cstAbility* ExecutingAbility;

protected:
	cstAttributes Attributes;
	cstStatusEffects StatusEffects;
	cstECharacterTeam Team;


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
	void ExecuteAbility(cstAbility* ability, const cstExecute::TargetParams& targetParams);
	void UseItem(cstItem* item, const cstExecute::TargetParams& targetParams);
	void StopAction();
	void ApplyKO();


	NS_NODISCARD_INLINE cstAttributes& GetAttributes()
	{
		return Attributes;
	}


	NS_NODISCARD_INLINE const cstAttributes& GetAttributes() const
	{
		return Attributes;
	}


	NS_NODISCARD_INLINE cstStatusEffects GetStatusEffects() const
	{
		return StatusEffects;
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
		return IsAlive() && (ExecutingAbility == nullptr) && !(StatusEffects & cstEStatusEffect::Stun);
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

private:
	nsTArrayInline<cstAbility*, cstInputAction::ABILITY_SLOT_MAX_COUNT> AbilitySlots;
	nsTArrayInline<cstItem*, cstInputAction::ITEM_SLOT_MAX_COUNT> ItemSlots;


public:
	cstPlayerCharacter();

private:
	NS_NODISCARD_INLINE int MapToAbilitySlotIndex(cstInputAction::EType inputType) const
	{
		NS_Assert(inputType >= cstInputAction::ABILITY_SLOT_ATTACK && inputType <= cstInputAction::ABILITY_SLOT_7);
		return cstInputAction::ABILITY_SLOT_MAX_COUNT - (cstInputAction::ABILITY_SLOT_7 - inputType);
	}


	NS_NODISCARD_INLINE int MapToItemSlotIndex(cstInputAction::EType inputType) const
	{
		NS_Assert(inputType >= cstInputAction::ITEM_SLOT_0 && inputType <= cstInputAction::ITEM_SLOT_7);
		return cstInputAction::ITEM_SLOT_MAX_COUNT - (cstInputAction::ITEM_SLOT_7 - inputType);
	}


public:
	NS_INLINE void ExecuteAbilityAtSlot(cstInputAction::EType abilitySlot, const cstExecute::TargetParams& targetParams)
	{
		NS_CONSOLE_Debug(cstPlayerLog, TEXT("Execute ability at slot [%s]"), cstInputAction::NAMES[abilitySlot]);
		const int index = MapToAbilitySlotIndex(abilitySlot);
		ExecuteAbility(AbilitySlots[index], targetParams);
	}


	NS_INLINE void UseItemAtSlot(cstInputAction::EType itemSlot, const cstExecute::TargetParams& targetParams)
	{
		NS_CONSOLE_Debug(cstPlayerLog, TEXT("Use item at slot [%s]"), cstInputAction::NAMES[itemSlot]);
		const int index = MapToItemSlotIndex(itemSlot);
		UseItem(ItemSlots[index], targetParams);
	}

};




class cstEnemyCharacter : public cstCharacter
{
	NS_DECLARE_OBJECT(cstEnemyCharacter)

public:
	cstEnemyCharacter();

};
