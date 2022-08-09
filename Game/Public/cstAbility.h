#pragma once

#include "cstAttributesEffect.h"


#define CST_ABILITY_MAX_LEVEL		(3)



enum class cstEAbilityActionType : uint8
{
	// Only click the shortcut to execute
	CLICK_SHORTCUT_ONLY = 0,

	// Click shortcut and click on target to execute
	CLICK_SHORTCUT_AND_CLICK_TARGET,

	// Click shortcut and click on any point to execute
	CLICK_SHORTCUT_AND_CLICK_ANY_POINT,

	// Click shortcut, drag mouse on ground, and release to execute
	CLICK_SHORTCUT_AND_DRAG_RELEASE
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
	CASTING,
	EXECUTING,
	COOLDOWN,
	NOT_ENOUGH_MANA,
	INVALID_EXECUTOR,
	INVALID_TARGET_ANY,
	INVALID_TARGET_SELF,
	INVALID_TARGET_ALLY,
	INVALID_TARGET_ENEMY,
	REQUIRED_TAGS_NOT_FOUND,
	IGNORED_TAGS_FOUND
};



struct cstAbilityExecutionTarget
{
	cstCharacter* Character;
	nsVector3 Location;
	nsVector3 DragStartLocation;
	nsVector3 DragEndLocation;


public:
	cstAbilityExecutionTarget()
		: Character(nullptr)
	{
	}


	cstAbilityExecutionTarget(cstCharacter* targetCharacter)
		: Character(targetCharacter)
	{
	}

};



struct cstAbilityAttributes
{
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

	// Mana cost effect 
	float ManaCost;

	// Effect to target
	cstEffectContext Effect;


public:
	cstAbilityAttributes()
	{
		CastingDistance = 0.0f;
		CastingDuration = 0.0f;
		ChannelingDuration = 0.0f;
		CooldownDuration = 0.0f;
		AreaEffectRadius = 0.0f;
		ManaCost = 0.0f;
	}

};



class cstAbility : public nsObject
{
	NS_DECLARE_OBJECT(cstAbility)

public:
	// Action type. Used to determine targeting system for player
	cstEAbilityActionType ActionType;

	// Target type 
	cstEAbilityTargetType TargetType;

	// Ability tags
	cstTags AbilityTags;

	// Executor must have this tags to execute this ability
	cstTags RequiredTags;

	// Executor must NOT have this tags to execute this ability
	cstTags IgnoredTags;

	// Attributes
	cstAbilityAttributes Attributes[CST_ABILITY_MAX_LEVEL];

	// Animation to play when execute this ability (random)
	nsTArrayInline<nsSharedAnimationAsset, 3> Animations;

private:
	float LastCommitTime;
	float CastingRemainingTime;
	float CooldownRemainingTime;
	cstEffectExecution_Add* EffectExecutionManaCost;

protected:
	int Level;
	cstCharacter* ExecutorCharacter;
	cstAbilityExecutionTarget ExecutionTarget;


public:
	cstAbility();

protected:
	void CommitAbility(float currentTime);

public:
	cstEAbilityExecutionResult CanExecute(float currentTime, cstCharacter* character, const cstAbilityExecutionTarget& target, int level = 1) const;
	cstEAbilityExecutionResult Execute(float currentTime, cstCharacter* character, const cstAbilityExecutionTarget& target, int level = 1);
	void TickUpdate(float deltaTime, float currentTime);
	

	NS_NODISCARD_INLINE bool IsActive() const
	{
		return CastingRemainingTime > 0.0f || CooldownRemainingTime > 0.0f;
	}


	NS_NODISCARD_INLINE bool CanBeCancelled(int level = 1) const
	{
		return CastingRemainingTime <= 0.0f || CastingRemainingTime > Attributes[level - 1].CastingDuration * 0.5f;
	}


	NS_NODISCARD_INLINE float GetCastingDistance(int level = 1) const
	{
		return Attributes[level - 1].CastingDistance;
	}


	NS_NODISCARD_INLINE float GetCastingRemainingTime() const
	{
		return CastingRemainingTime;
	}


	NS_NODISCARD_INLINE float GetCooldownRemainingTime() const
	{
		return CooldownRemainingTime;
	}


protected:
	virtual void StartExecute(float currentTime) {}
	virtual void StopExecute(bool bWasCancelled) {}

};



class cstAbility_Stop : public cstAbility
{
	NS_DECLARE_OBJECT(cstAbility_Stop)

public:
	cstAbility_Stop();
	virtual void StartExecute(float currentTime) override;

};



class cstAbility_Attack : public cstAbility
{
	NS_DECLARE_OBJECT(cstAbility_Attack)

public:
	cstAbility_Attack();
	virtual void StartExecute(float currentTime) override;

};
