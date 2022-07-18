#pragma once

#include "cstTypes.h"


#define CST_ABILITY_MAX_LEVEL		(3)



enum class cstEAbilityTargetType : uint8
{
	// Target self only
	SELF_ONLY = 0,

	// Target self or ally
	ALLY_ONLY,

	// Target enemy only 
	ENEMY_ONLY,

	// Target self, ally, or enemy
	ANY,
};



struct cstAbilityAttributes
{
	// Amount of mana required to execute this ability
	float ManaCost;

	// Distance to target to execute this ability. Used to approaching target before actually execute it
	float CastingDistance;

	// How long to perform/execute this ability (seconds)
	float CastingDuration;

	// Channeling duration while executing this ability (seconds)
	float ChannelingDuration;

	// Cooldown duration (seconds)
	float CooldownDuration;

	// AoE radius
	float AreaEffectRadius;

	// Effect
	cstAttributes Effect;


public:
	cstAbilityAttributes()
	{
		ManaCost = 0.0f;
		CastingDistance = 0.0f;
		CastingDuration = 0.0f;
		ChannelingDuration = 0.0f;
		CooldownDuration = 0.0f;
		AreaEffectRadius = 0.0f;
	}

};



class cstAbility : public nsObject
{
	NS_DECLARE_OBJECT(cstAbility)

public:
	// Execute action type
	cstExecute::EActionType ExecuteActionType;

	// Target type 
	cstEAbilityTargetType TargetType;

	// Required weapon class to execute this ability
	const nsClass* RequiredWeaponClass;

	// Attributes
	nsTArrayInline<cstAbilityAttributes, CST_ABILITY_MAX_LEVEL> Attributes;

	// Current level
	int CurrentLevel;

	// UI: Ability icon
	nsSharedTextureAsset DisplayIcon;

private:
	float LastExecutionTime;
	float ExecutionRemainingTime;


public:
	cstAbility();
	cstExecute::EResult CanExecute(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams) const;
	cstExecute::EResult Execute(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams);
	void UpdateExecution(float deltaTime);

protected:
	virtual void Execute_Implementation(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams) = 0;

public:
	NS_NODISCARD_INLINE float GetExecutionRemainingTime() const
	{
		return ExecutionRemainingTime;
	}


	NS_NODISCARD_INLINE bool IsExecuting() const
	{
		return ExecutionRemainingTime > 0.0f;
	}


	NS_NODISCARD_INLINE float GetCastingDistance() const
	{
		return Attributes[CurrentLevel - 1].CastingDistance;
	}

};



class cstAbility_Dummy : public cstAbility
{
	NS_DECLARE_OBJECT(cstAbility_Dummy)

public:
	cstAbility_Dummy();
	virtual void Execute_Implementation(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams) override;

};



class cstAbility_StopAction : public cstAbility
{
	NS_DECLARE_OBJECT(cstAbility_StopAction)

public:
	cstAbility_StopAction();
	virtual void Execute_Implementation(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams) override;

};
