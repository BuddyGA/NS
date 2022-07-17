#include "cstAbility.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstAbility, nsObject)
NS_CLASS_END(cstAbility)

cstAbility::cstAbility()
{
	ExecutionType = cstEAbilityExecutionType::POINT_AND_CLICK_TARGET;
	TargetType = cstEAbilityTargetType::SELF_ONLY;
	RequiredWeaponClass = nullptr;
	ManaCost = 0;
	ExecuteDistance = 0.0f;
	ExecuteDuration = 0.0f;
	ChannelingDuration = 0.0f;
	CooldownDuration = 0.0f;
	LastExecutionTime = 0.0f;
	ExecuteTimer = 0.0f;
}


cstEAbilityExecutionResult cstAbility::CanExecute(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation) const
{
	if (ExecuteTimer > 0.0f)
	{
		return cstEAbilityExecutionResult::EXECUTING;
	}

	if (LastExecutionTime > 0.0f && currentTime < (LastExecutionTime + CooldownDuration))
	{
		return cstEAbilityExecutionResult::COOLDOWN;
	}

	if (characterExecutor == nullptr)
	{
		return cstEAbilityExecutionResult::INVALID_EXECUTOR;
	}


	if (ExecutionType == cstEAbilityExecutionType::POINT_AND_CLICK_TARGET)
	{
		if (characterTarget == nullptr)
		{
			return cstEAbilityExecutionResult::INVALID_TARGET_ANY;
		}

		switch (TargetType)
		{
			case cstEAbilityTargetType::SELF_ONLY:
			{
				if (characterExecutor != characterTarget)
				{
					return cstEAbilityExecutionResult::INVALID_TARGET_SELF;
				}

				break;
			}

			case cstEAbilityTargetType::ALLY_ONLY:
			{
				if (characterExecutor->GetTeam() != characterTarget->GetTeam())
				{
					return cstEAbilityExecutionResult::INVALID_TARGET_ALLY;
				}

				break;
			}

			case cstEAbilityTargetType::ENEMY_ONLY:
			{
				if (characterExecutor->GetTeam() == characterTarget->GetTeam())
				{
					return cstEAbilityExecutionResult::INVALID_TARGET_ENEMY;
				}

				break;
			}

			default: break;
		}
	}
	else if (ExecutionType == cstEAbilityExecutionType::POINT_AND_CLICK_GROUND)
	{

	}
	else if (ExecutionType == cstEAbilityExecutionType::POINT_AND_CLICK_DRAG)
	{

	}
	

	// TODO: Check required weapon

	const cstAttributes& executorAttr = characterExecutor->GetAttributes();
	const cstCharacterStatusFlags executorStatusFlags = characterExecutor->GetStatusFlags();

	if (executorAttr.Mana < ManaCost)
	{
		return cstEAbilityExecutionResult::NOT_ENOUGH_MANA;
	}

	if (executorAttr.Health <= 0.0f)
	{
		NS_Assert(executorStatusFlags & cstECharacterStatus::KO);
		return cstEAbilityExecutionResult::EXECUTOR_KOed;
	}

	if (executorStatusFlags & cstECharacterStatus::Silence)
	{
		return cstEAbilityExecutionResult::EXECUTOR_SILENCED;
	}

	if (executorStatusFlags & cstECharacterStatus::Stun)
	{
		return cstEAbilityExecutionResult::EXECUTOR_STUNNED;
	}


	return cstEAbilityExecutionResult::SUCCESS;
}


void cstAbility::UpdateExecution(float deltaTime)
{
	if (ExecuteTimer > 0.0f)
	{
		ExecuteTimer = nsMath::Clamp(ExecuteTimer - deltaTime, 0.0f, ExecuteDuration);
	}
}


cstEAbilityExecutionResult cstAbility::Execute(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation)
{
	const cstEAbilityExecutionResult result = CanExecute(currentTime, characterExecutor, characterTarget, groundLocation, dragStartLocation, dragEndLocation);

	if (result == cstEAbilityExecutionResult::SUCCESS)
	{
		LastExecutionTime = currentTime;
		ExecuteTimer = ExecuteDuration;
		Execute_Implementation(currentTime, characterExecutor, characterTarget, groundLocation, dragStartLocation, dragEndLocation);
	}

	return result;
}




NS_CLASS_BEGIN(cstAbility_Dummy, cstAbility)
NS_CLASS_END(cstAbility_Dummy)

cstAbility_Dummy::cstAbility_Dummy()
{
	Name = TEXT("Ability_Dummy");
	ExecuteDuration = 3.0f;
	CooldownDuration = 1.5f;
}


void cstAbility_Dummy::Execute_Implementation(float currentTime, cstCharacter* characterExecutor, cstCharacter* characterTarget, nsVector3 groundLocation, nsVector3 dragStartLocation, nsVector3 dragEndLocation)
{

}
