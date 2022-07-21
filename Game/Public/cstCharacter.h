#pragma once

#include "cstAbility.h"
#include "cstItem.h"



enum class cstECharacterState : uint8
{
	NONE = 0,
	IDLE,
	MOVE,
	CHASE_TARGET,
	EXECUTE_ABILITY,
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

	cstAttributes BaseAttributes;
	cstAttributes CurrentAttributes;
	nsTArrayInline<cstAbility*, 16> Abilities;
	nsTArrayInline<cstEffectExecution*, 8> ActiveEffects;
	cstTags OwningTags;
	cstWeapon* EquippedWeapon;

	nsVector3 MoveTargetPosition;
	float MoveDistanceToTarget;

	cstAbilityExecutionTarget ExecutionTarget;
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


private:
	void UpdateActiveAbilities(float deltaTime);
	void UpdateActiveEffects(float deltaTime);
	void UpdateState(float deltaTime);
	void UpdateAnimation(float deltaTime);

public:
	void EquipWeapon(cstWeapon* weapon);
	void EquipArmor(cstArmor* armor);
	void ExecuteAbility(cstAbility* ability, const cstAbilityExecutionTarget& targetParams);
	void UseItem(cstItem* item, const cstAbilityExecutionTarget& targetParams);
	void Move(const nsVector3& worldPosition);
	void Attack(cstCharacter* targetCharacter);
	void Stop();


private:
	NS_INLINE void ValidateCharacterTags()
	{
		if ((OwningTags & (cstTag::Character_Player | cstTag::Character_Enemy | cstTag::Character_Neutral)) == 0)
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Character [%s] must have at least one character tag [Character_Player/Character_Enemy/Character_Neutral]!"), *Name);
		}

		if ((OwningTags & cstTag::Character_Player) && (OwningTags & cstTag::Character_Enemy))
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Character [%s] contains character tag [Character_Player] and [Character_Enemy] at the same time!"), *Name);
		}

		if ((OwningTags & cstTag::Character_Player) && (OwningTags & cstTag::Character_Neutral))
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Character [%s] contains character tag [Character_Player] and [Character_Neutral] at the same time!"), *Name);
		}

		if ((OwningTags & cstTag::Character_Enemy) && (OwningTags & cstTag::Character_Neutral))
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Character [%s] contains character tag [Character_Enemy] and [Character_Neutral] at the same time!"), *Name);
		}
	}


public:
	NS_INLINE void AddOwningTags(cstTags tags)
	{
		OwningTags |= tags;
		ValidateCharacterTags();
	}


	NS_INLINE void RemoveOwningTags(cstTags tags)
	{
		OwningTags &= ~tags;
		ValidateCharacterTags();
	}


	NS_NODISCARD_INLINE cstTags GetOwningTags() const
	{
		return OwningTags;
	}


	NS_NODISCARD_INLINE cstAttributes& GetBaseAttributes()
	{
		return BaseAttributes;
	}


	NS_NODISCARD_INLINE const cstAttributes& GetBaseAttributes() const
	{
		return BaseAttributes;
	}


	NS_NODISCARD_INLINE cstAttributes& GetCurrentAttributes()
	{
		return CurrentAttributes;
	}


	NS_NODISCARD_INLINE const cstAttributes& GetCurrentAttributes() const
	{
		return CurrentAttributes;
	}


	NS_NODISCARD_INLINE bool IsAlly(const cstCharacter* other) const
	{
		const cstTags otherOwningTags = other->OwningTags;

		if ( (OwningTags & cstTag::Character_Player) && (otherOwningTags & cstTag::Character_Player) )
		{
			return true;
		}

		if ( (OwningTags & cstTag::Character_Enemy) && (otherOwningTags & cstTag::Character_Enemy) )
		{
			return true;
		}

		if ( (OwningTags & cstTag::Character_Neutral) && (otherOwningTags & cstTag::Character_Neutral) )
		{
			return true;
		}

		return false;
	}


	NS_NODISCARD_INLINE bool IsEnemy(const cstCharacter* other) const
	{
		return !IsAlly(other);
	}


	NS_NODISCARD_INLINE bool IsAlive() const
	{
		return BaseAttributes[cstAttribute::CURRENT_HEALTH] > 0.0f;
	}


	NS_NODISCARD_INLINE bool CanMove() const
	{
		return IsAlive() && !(OwningTags & cstTag::DISABLE_ACTION);
	}


#ifdef CST_GAME_WITH_EDITOR
public:
	void DebugGUI(nsGUIContext& context);
	void DebugDraw(nsRenderer* renderer);
#endif // CST_GAME_WITH_EDITOR

};
