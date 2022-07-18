#include "cstAbility.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstAbility, nsObject)
NS_CLASS_END(cstAbility)

cstAbility::cstAbility()
{
	ExecuteActionType = cstExecute::ACTION_CLICK_SHORTCUT_ONLY;
	TargetType = cstEAbilityTargetType::SELF_ONLY;
	RequiredWeaponClass = nullptr;
	LastExecutionTime = 0.0f;
	ExecutionRemainingTime = 0.0f;
	Attributes.Resize(CST_ABILITY_MAX_LEVEL);
	CurrentLevel = 1;
}


cstExecute::EResult cstAbility::CanExecute(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams) const
{
	const int levelIndex = CurrentLevel - 1;
	NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

	if (ExecutionRemainingTime > 0.0f)
	{
		return cstExecute::RESULT_EXECUTING;
	}

	if (LastExecutionTime > 0.0f && currentTime < (LastExecutionTime + Attributes[levelIndex].CooldownDuration))
	{
		return cstExecute::RESULT_COOLDOWN;
	}

	if (executorCharacter == nullptr)
	{
		return cstExecute::RESULT_INVALID_EXECUTOR;
	}

	// TODO: Check required weapon

	const cstAttributes& executorAttr = executorCharacter->GetAttributes();
	const cstStatusEffects executorStatusEffects = executorCharacter->GetStatusEffects();

	if (executorAttr.Mana < Attributes[levelIndex].ManaCost)
	{
		return cstExecute::RESULT_NOT_ENOUGH_MANA;
	}

	if (executorAttr.Health <= 0.0f)
	{
		NS_Assert(executorStatusEffects & cstEStatusEffect::KO);
		return cstExecute::RESULT_EXECUTOR_KOed;
	}

	if (executorStatusEffects & cstEStatusEffect::Silence)
	{
		return cstExecute::RESULT_EXECUTOR_SILENCED;
	}

	if (executorStatusEffects & cstEStatusEffect::Stun)
	{
		return cstExecute::RESULT_EXECUTOR_STUNNED;
	}


	switch (TargetType)
	{
		case cstEAbilityTargetType::SELF_ONLY:
		{
			if (targetParams.TargetCharacter != executorCharacter)
			{
				return cstExecute::RESULT_INVALID_TARGET_SELF;
			}

			break;
		}

		case cstEAbilityTargetType::ALLY_ONLY:
		{
			if (targetParams.TargetCharacter == nullptr || targetParams.TargetCharacter->GetTeam() != executorCharacter->GetTeam())
			{
				return cstExecute::RESULT_INVALID_TARGET_ALLY;
			}

			break;
		}

		case cstEAbilityTargetType::ENEMY_ONLY:
		{
			if (targetParams.TargetCharacter == nullptr || targetParams.TargetCharacter->GetTeam() == executorCharacter->GetTeam())
			{
				return cstExecute::RESULT_INVALID_TARGET_ENEMY;
			}

			break;
		}

		case cstEAbilityTargetType::ANY:
		{
			if (targetParams.TargetCharacter == nullptr)
			{
				return cstExecute::RESULT_INVALID_TARGET_ANY;
			}

			break;
		}

		default: break;
	}
	

	return cstExecute::RESULT_SUCCESS;
}


void cstAbility::UpdateExecution(float deltaTime)
{
	if (ExecutionRemainingTime > 0.0f)
	{
		const int levelIndex = CurrentLevel - 1;
		NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

		ExecutionRemainingTime = nsMath::Clamp(ExecutionRemainingTime - deltaTime, 0.0f, Attributes[levelIndex].CastingDuration);
	}
}


cstExecute::EResult cstAbility::Execute(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams)
{
	const cstExecute::EResult result = CanExecute(currentTime, executorCharacter, targetParams);

	if (result == cstExecute::RESULT_SUCCESS)
	{
		const int levelIndex = CurrentLevel - 1;
		NS_Assert(levelIndex >= 0 && levelIndex < CST_ABILITY_MAX_LEVEL);

		LastExecutionTime = currentTime;
		ExecutionRemainingTime = Attributes[levelIndex].CastingDuration;
		Execute_Implementation(currentTime, executorCharacter, targetParams);
	}

	return result;
}




NS_CLASS_BEGIN(cstAbility_Dummy, cstAbility)
NS_CLASS_END(cstAbility_Dummy)

cstAbility_Dummy::cstAbility_Dummy()
{
	Name = TEXT("Ability_Dummy");
}


void cstAbility_Dummy::Execute_Implementation(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams)
{

}




NS_CLASS_BEGIN(cstAbility_StopAction, cstAbility)
NS_CLASS_END(cstAbility_StopAction)

cstAbility_StopAction::cstAbility_StopAction()
{
	Name = TEXT("Ability_StopAction");
}


void cstAbility_StopAction::Execute_Implementation(float currentTime, const cstCharacter* executorCharacter, const cstExecute::TargetParams& targetParams)
{
	targetParams.TargetCharacter->StopAction();
}
