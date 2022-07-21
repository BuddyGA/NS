#include "cstAbility.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstAbility, nsObject)
NS_CLASS_END(cstAbility)

cstAbility::cstAbility()
{
	ActionType = cstEAbilityActionType::CLICK_SHORTCUT_ONLY;
	TargetType = cstEAbilityTargetType::SELF_ONLY;
	RequiredTags = cstTag::NONE;
	IgnoredTags = cstTag::NONE;

	Level = 1;
	ExecutorCharacter = nullptr;
	LastCommitTime = 0.0f;
	CastingRemainingTime = 0.0f;
	CooldownRemainingTime = 0.0f;
	bIsActive = false;
}


void cstAbility::CommitAbility(float currentTime)
{
	const cstAbilityAttributes& attributes = Attributes[Level - 1];

	LastCommitTime = currentTime;
	CastingRemainingTime = attributes.CastingDuration;


	// TODO: Apply mana cost
	

	// Apply cooldown
	const cstAttributes& executorAttributes = ExecutorCharacter->GetBaseAttributes();
	CooldownRemainingTime = attributes.CooldownDuration - (attributes.CooldownDuration * executorAttributes[cstAttribute::CURRENT_ABILITY_COOLDOWN_REDUCTION]);
	CooldownRemainingTime = nsMath::Max(CooldownRemainingTime, 0.0f);
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

	if (charAttributes[cstAttribute::CURRENT_MANA] < Attributes[levelIndex].ManaCost)
	{
		return cstEAbilityExecutionResult::NOT_ENOUGH_MANA;
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
	if (!bIsActive)
	{
		return;
	}

	if (CastingRemainingTime > 0.0f)
	{
		CastingRemainingTime = nsMath::Max(CastingRemainingTime - deltaTime, 0.0f);
	}
	else if (CooldownRemainingTime > 0.0f)
	{
		CooldownRemainingTime = nsMath::Max(CooldownRemainingTime - deltaTime, 0.0f);
	}
	else
	{
		StopExecute(false);
		bIsActive = false;
	}
}


cstEAbilityExecutionResult cstAbility::Execute(float currentTime, cstCharacter* executorCharacter, const cstAbilityExecutionTarget& target, int level)
{
	const cstEAbilityExecutionResult result = CanExecute(currentTime, executorCharacter, target);

	if (result == cstEAbilityExecutionResult::SUCCESS)
	{
		Level = level;
		const int levelIndex = Level - 1;
		NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

		ExecutorCharacter = executorCharacter;
		ExecutionTarget = target;
		StartExecute(currentTime);
	}

	return result;
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

	cstEffectContext& effect = attributes.Effect;
	effect.ModifiedAttributes[cstAttribute::CURRENT_HEALTH] = -15.0f;
}


void cstAbility_Attack::StartExecute(float currentTime)
{
	CommitAbility(currentTime);

	// TODO: Play animation

}




NS_CLASS_BEGIN(cstAbility_Stop, cstAbility)
NS_CLASS_END(cstAbility_Stop)

cstAbility_Stop::cstAbility_Stop()
{
	Name = TEXT("Ability_Stop");
}


void cstAbility_Stop::StartExecute(float currentTime)
{
	ExecutionTarget.Character->Stop();
}
