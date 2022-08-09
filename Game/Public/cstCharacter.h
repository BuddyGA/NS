#pragma once

#include "cstAbility.h"
#include "cstItem.h"



enum class cstECharacterState : uint8
{
	NONE = 0,
	IDLE,
	MOVE,
	ATTACK_TARGET,
	EXECUTE_ABILITY,
	USE_ITEM,
	INTERACT,
	DISABLED,
	KO,
};



enum class cstECharacterCommand : uint8
{
	NONE = 0,
	STOP,
	MOVE,
	ATTACK,
	ABILITY,
	ITEM,
	INTERACT
};



struct cstCharacterAbility
{
	const nsClass* Class;
	cstAbility* Instance;
	cstAbilityExecutionTarget ExecutionTarget;
	int Level;


public:
	cstCharacterAbility()
	{
		Class = nullptr;
		Instance = nullptr;
		Level = 1;
	}


	NS_NODISCARD_INLINE bool IsValid() const
	{
		return Class && Instance;
	}


	NS_INLINE bool operator==(const nsClass* rhs) const
	{
		return Class == rhs;
	}

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

	cstTags OwningTags;
	cstAttributes BaseAttributes;
	cstAttributes TempAttributes;
	cstAttributes CurrentAttributes;
	nsTArrayInline<cstEffectExecution*, 8> ActiveEffects;
	nsTArrayInline<cstCharacterAbility, 16> Abilities;
	cstWeapon* EquippedWeapon;

	nsVector3 MoveTargetPosition;
	float MoveDistanceToTarget;
	int AbilityIndex;

	cstECharacterCommand Command;


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
	bool MoveToTargetToExecuteAbility();
	void UpdateActiveEffects(float deltaTime);
	void UpdateActiveAbilities(float deltaTime);
	void UpdateState(float deltaTime);
	void UpdateAnimation(float deltaTime);
	cstEffectExecution* FindActiveEffect(cstTags effectTags, cstEffectExecution* effectExecution) const;

public:
	void EquipWeapon(cstWeapon* weapon);
	void EquipArmor(cstArmor* armor);
	void AddAbility(const nsClass* abilityClass);
	void RemoveAbility(const nsClass* abilityClass);
	void ApplyEffect(cstEffectExecution* effectExecution, const cstEffectContext& effectContext);
	void RemoveEffect(cstTags effectTags);
	void CommandStop();
	void CommandMove(const nsVector3& worldPosition);
	void CommandAttack(cstCharacter* character);
	void CommandExecuteAbility(const nsClass* abilityClass, const cstAbilityExecutionTarget& targetParams, int level = 1);
	void CommandUseItem(cstItem* item, const cstAbilityExecutionTarget& targetParams);
	void CommandInteract(nsActor* actor);


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
	NS_NODISCARD_INLINE nsNavigationAgentComponent* GetNavigationAgentComponent() const
	{
		return NavigationAgentComponent;
	}


	NS_NODISCARD_INLINE cstCharacterAbility& GetAbility(int index)
	{
		return Abilities[index];
	}


	NS_NODISCARD_INLINE const cstCharacterAbility& GetAbility(int index) const
	{
		return Abilities[index];
	}


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


	NS_NODISCARD_INLINE cstAttributes& GetCurrentAttributes()
	{
		return CurrentAttributes;
	}


	NS_NODISCARD_INLINE const cstAttributes& GetCurrentAttributes() const
	{
		return CurrentAttributes;
	}


	// Check if this character sees other character as ally
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


	// Check if this character sees other character as enemy
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
		return IsAlive() && !(OwningTags & cstTag::CHARACTER_DISABLED);
	}


#ifdef CST_GAME_WITH_EDITOR
public:
	void DebugGUI(nsGUIContext& context);
	void DebugDraw(nsRenderer* renderer);
#endif // CST_GAME_WITH_EDITOR

};
