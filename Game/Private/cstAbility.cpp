#include "cstAbility.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstAbility, nsObject)
NS_CLASS_END(cstAbility)

cstAbility::cstAbility()
{
	ActionType = cstEAbilityActionType::CLICK_SHORTCUT_ONLY;
	TargetType = cstEAbilityTargetType::SELF_ONLY;
	AbilityTags = cstTag::NONE;
	RequiredTags = cstTag::NONE;
	IgnoredTags = cstTag::NONE;

	LastCommitTime = 0.0f;
	CastingRemainingTime = 0.0f;
	CooldownRemainingTime = 0.0f;

	Level = 1;
	ExecutorCharacter = nullptr;
}


float cstAbility::CountTotalCostHP(const cstAttributes& charAttributes, int level) const
{
	const cstAbilityAttributes& abilityAttributes = Attributes[level - 1];

	return nsMath::Max(abilityAttributes.CostHP - (abilityAttributes.CostHP * charAttributes[cstAttribute::ABILITY_HP_COST_REDUCTION]), 0.0f);
}


float cstAbility::CountTotalCostMP(const cstAttributes& charAttributes, int level) const
{
	const cstAbilityAttributes& abilityAttributes = Attributes[level - 1];

	return nsMath::Max(abilityAttributes.CostMP - (abilityAttributes.CostMP * charAttributes[cstAttribute::ABILITY_MP_COST_REDUCTION]), 0.0f);
}


void cstAbility::CommitAbility(float currentTime)
{
	LastCommitTime = currentTime;
	const cstAttributes& executorAttributes = ExecutorCharacter->GetCurrentAttributes();
	const cstAbilityAttributes& abilityAttributes = Attributes[Level - 1];

	// Apply HP, MP cost
	if (abilityAttributes.CostHP > 0.0f || abilityAttributes.CostMP > 0.0f)
	{
		cstEffectContext effectContext;
		effectContext.AddAttributeModification(cstAttribute::CURRENT_HP, -CountTotalCostHP(executorAttributes, Level));
		effectContext.AddAttributeModification(cstAttribute::CURRENT_MP, -CountTotalCostMP(executorAttributes, Level));

		ExecutionTarget.Character->ApplyEffect(ns_GetDefaultObjectAs<cstEffectExecution_Add>(cstEffectExecution_Add::Class), effectContext);
	}

	// Apply cooldown
	CooldownRemainingTime = nsMath::Max(abilityAttributes.CooldownDuration - (abilityAttributes.CooldownDuration * executorAttributes[cstAttribute::ABILITY_COOLDOWN_REDUCTION]), 0.0f);
}


cstEAbilityExecutionResult cstAbility::CanExecute(float currentTime, cstCharacter* character, const cstAbilityExecutionTarget& target, int level) const
{
	const int levelIndex = level - 1;
	NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

	if (character == nullptr)
	{
		return cstEAbilityExecutionResult::INVALID_EXECUTOR;
	}

	if (CastingRemainingTime > 0.0f)
	{
		return cstEAbilityExecutionResult::CASTING;
	}

	if (LastCommitTime > 0.0f && currentTime < (LastCommitTime + Attributes[levelIndex].CooldownDuration))
	{
		return cstEAbilityExecutionResult::COOLDOWN;
	}


	switch (TargetType)
	{
		case cstEAbilityTargetType::SELF_ONLY:
		{
			if (target.Character != character)
			{
				return cstEAbilityExecutionResult::INVALID_TARGET_SELF;
			}

			break;
		}

		case cstEAbilityTargetType::ALLY_ONLY:
		{
			if (target.Character == nullptr || !target.Character->IsAlly(character))
			{
				return cstEAbilityExecutionResult::INVALID_TARGET_ALLY;
			}

			break;
		}

		case cstEAbilityTargetType::ENEMY_ONLY:
		{
			if (target.Character == nullptr || !target.Character->IsEnemy(character))
			{
				return cstEAbilityExecutionResult::INVALID_TARGET_ENEMY;
			}

			break;
		}

		case cstEAbilityTargetType::ANY:
		{
			if (target.Character == nullptr)
			{
				return cstEAbilityExecutionResult::INVALID_TARGET_ANY;
			}

			break;
		}

		default: break;
	}


	const cstAttributes& charAttributes = character->GetCurrentAttributes();

	if (charAttributes[cstAttribute::CURRENT_HP] < CountTotalCostHP(charAttributes, level))
	{
		return cstEAbilityExecutionResult::NOT_ENOUGH_HP;
	}

	if (charAttributes[cstAttribute::CURRENT_MP] < CountTotalCostMP(charAttributes, level))
	{
		return cstEAbilityExecutionResult::NOT_ENOUGH_MP;
	}


	const cstTags charOwningTags = character->GetOwningTags();

	// Must have required tags to execute this ability
	if ( (RequiredTags != cstTag::NONE) && !(RequiredTags & charOwningTags) )
	{
		return cstEAbilityExecutionResult::REQUIRED_TAGS_NOT_FOUND;
	}

	// Must NOT have ignored tags to execute this ability
	if ( (IgnoredTags != cstTag::NONE) && (IgnoredTags & charOwningTags) )
	{
		return cstEAbilityExecutionResult::IGNORED_TAGS_FOUND;
	}


	return cstEAbilityExecutionResult::SUCCESS;
}


void cstAbility::TickUpdate(float deltaTime, float currentTime)
{
	if (!IsActive())
	{
		return;
	}

	if (CastingRemainingTime > 0.0f)
	{
		CastingRemainingTime = nsMath::Max(CastingRemainingTime - deltaTime, 0.0f);

		if (CastingRemainingTime <= 0.0f)
		{
			CommitAbility(currentTime);
			StartExecute(currentTime);
		}
	}
	else if (CooldownRemainingTime > 0.0f)
	{
		CooldownRemainingTime = nsMath::Max(CooldownRemainingTime - deltaTime, 0.0f);
	}
}


cstEAbilityExecutionResult cstAbility::Execute(float currentTime, cstCharacter* character, const cstAbilityExecutionTarget& target, int level)
{
	const cstEAbilityExecutionResult result = CanExecute(currentTime, character, target);

	if (result == cstEAbilityExecutionResult::SUCCESS)
	{
		Level = level;
		const int levelIndex = Level - 1;
		NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

		ExecutorCharacter = character;
		ExecutionTarget = target;
		CastingRemainingTime = Attributes[levelIndex].CastingDuration;
	}

	return result;
}




NS_CLASS_BEGIN(cstAbility_Stop, cstAbility)
NS_CLASS_END(cstAbility_Stop)

cstAbility_Stop::cstAbility_Stop()
{
	Name = TEXT("Ability_Stop");
	ActionType = cstEAbilityActionType::CLICK_SHORTCUT_ONLY;
	TargetType = cstEAbilityTargetType::SELF_ONLY;
}


void cstAbility_Stop::StartExecute(float currentTime)
{
	ExecutionTarget.Character->CommandStop();
}




NS_CLASS_BEGIN(cstAbility_Attack, cstAbility)
NS_CLASS_END(cstAbility_Attack)

cstAbility_Attack::cstAbility_Attack()
{
	Name = TEXT("Ability_Attack");
	ActionType = cstEAbilityActionType::CLICK_SHORTCUT_AND_CLICK_TARGET;
	TargetType = cstEAbilityTargetType::ENEMY_ONLY;

	cstAbilityAttributes& attributes = Attributes[0];
	attributes.CastingDistance = 150.0f;
	attributes.CastingDuration = 1.0f;
	attributes.CooldownDuration = 5.0f;
	attributes.Effect.AddAttributeModification(cstAttribute::CURRENT_HP, -15.0f);
}


void cstAbility_Attack::StartExecute(float currentTime)
{
	// TODO: Play animation

	cstEffectExecution_Add* effectExecution = ns_GetDefaultObjectAs<cstEffectExecution_Add>(cstEffectExecution_Add::Class);
	ExecutionTarget.Character->ApplyEffect(effectExecution, Attributes[Level - 1].Effect);
}
