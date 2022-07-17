#pragma once

#include "cstTypes.h"



enum class cstEAbilityExecutionType : uint8
{
	POINT_AND_CLICK_TARGET = 0,
	POINT_AND_CLICK_GROUND,
	POINT_AND_CLICK_DRAG
};



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



enum class cstEAbilityExecutionResult : uint8
{
	SUCCESS = 0,
	EXECUTING,
	COOLDOWN,
	INVALID_EXECUTOR,
	INVALID_TARGET_ANY,
	INVALID_TARGET_SELF,
	INVALID_TARGET_ALLY,
	INVALID_TARGET_ENEMY,
	WEAPON_REQUIRED,
	NOT_ENOUGH_MANA,
	EXECUTOR_KOed,
	EXECUTOR_SILENCED,
	EXECUTOR_STUNNED,
};



class cstAbility : public nsObject
{
	NS_DECLARE_OBJECT(cstAbility)

public:
	// Execution type
	cstEAbilityExecutionType ExecutionType;

	// Target type 
	cstEAbilityTargetType TargetType;

	// Required weapon class to execute this ability
	const nsClass* RequiredWeaponClass;

	// Mana cost to execute this ability
	float ManaCost;

	// Distance to target to execute this ability. Used to approaching target before actually execute it
	float ExecuteDistance;

	// How long to perform/execute this ability (seconds)
	float ExecuteDuration;

	// Channeling duration while executing this ability (seconds)
	float ChannelingDuration;

	// Cooldown duration (seconds)
	float CooldownDuration;


private:
	float LastExecutionTime;
	float ExecuteTimer;


public:
	cstAbility();
	cstEAbilityExecutionResult CanExecute(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation) const;
	cstEAbilityExecutionResult Execute(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation);
	void UpdateExecution(float deltaTime);

protected:
	virtual void Execute_Implementation(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation) = 0;

public:
	NS_NODISCARD_INLINE float GetExecuteTimer() const
	{
		return ExecuteTimer;
	}


	NS_NODISCARD_INLINE bool IsExecuting() const
	{
		return ExecuteTimer > 0.0f;
	}

};



class cstAbility_Dummy : public cstAbility
{
	NS_DECLARE_OBJECT(cstAbility_Dummy)

public:
	cstAbility_Dummy();
	virtual void Execute_Implementation(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation) override;

};
