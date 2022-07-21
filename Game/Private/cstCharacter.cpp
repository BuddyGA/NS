#include "cstCharacter.h"
#include "nsAssetManager.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"
#include "nsNavigationComponents.h"
#include "nsWorld.h"



NS_CLASS_BEGIN(cstCharacter, nsActor)
NS_CLASS_END(cstCharacter)

cstCharacter::cstCharacter()
{
	Flags |= nsEActorFlag::CallPrePhysicsTickUpdate;

	NavigationAgentComponent = AddComponent<nsNavigationAgentComponent>("nav_agent");
	NavigationAgentComponent->MaxAcceleration = 1200.0f;
	NavigationAgentComponent->MaxSpeed = 300.0f;
	RootComponent = NavigationAgentComponent;

	SkelMeshComponent = AddComponent<nsSkeletalMeshComponent>("skeletal_mesh");
	SkelMeshComponent->bDebugDrawSkeleton = true;
	SkelMeshComponent->SetLocalPosition(nsVector3(0.0f, -90.0f, 0.0f));

	PendingChangeState = cstECharacterState::NONE;
	CurrentState = cstECharacterState::IDLE;

	OwningTags = cstTag::NONE;
	BaseAttributes = cstAttributes::InitializeCharacterBaseAttributes();
	Abilities.Resize(1);
	EquippedWeapon = nullptr;

	MoveDistanceToTarget = 0.0f;
	ExecutingAbility = nullptr;
}


void cstCharacter::OnInitialize()
{
	nsActor::OnInitialize();

	nsAssetManager& assetManager = nsAssetManager::Get();
	SkelMeshComponent->SetMesh(assetManager.LoadModelAsset("mdl_LowPolyChar"));
	SkelMeshComponent->SetSkeleton(assetManager.LoadSkeletonAsset("skl_LowPolyChar_Rig"));

	AnimIdle0 = assetManager.LoadAnimationAsset("anim_idle_0");
	AnimRunForwardLoop = assetManager.LoadAnimationAsset("anim_run_forward_loop");
}


void cstCharacter::OnDestroy()
{
	nsActor::OnDestroy();
}


void cstCharacter::OnStartPlay()
{
	nsActor::OnStartPlay();

}


void cstCharacter::OnStopPlay()
{

	nsActor::OnStopPlay();
}


void cstCharacter::OnTickUpdate(float deltaTime)
{
	nsActor::OnTickUpdate(deltaTime);

	UpdateActiveAbilities(deltaTime);
	UpdateActiveEffects(deltaTime);
	UpdateState(deltaTime);
	UpdateAnimation(deltaTime);
}


void cstCharacter::UpdateActiveAbilities(float deltaTime)
{
	const float currentTime = GetWorld()->GetCurrentTimeSeconds();

	for (int i = 0; i < Abilities.GetCount(); ++i)
	{
		Abilities[i]->TickUpdate(deltaTime, currentTime);
	}
}


void cstCharacter::UpdateActiveEffects(float deltaTime)
{
	if (ActiveEffects.IsEmpty())
	{
		return;
	}

	const float currentTime = GetWorld()->GetCurrentTimeSeconds();

	for (int i = 0; i < ActiveEffects.GetCount();)
	{
		if (ActiveEffects[i]->IsActive())
		{
			ActiveEffects[i]->UpdateEffect(deltaTime);
			++i;
		}
		else
		{
			ActiveEffects.RemoveAt(i);
		}
	}
}


void cstCharacter::UpdateState(float deltaTime)
{
	switch (CurrentState)
	{
		case cstECharacterState::MOVE:
		{
			MoveDistanceToTarget = nsVector3::Distance(GetWorldPosition(), MoveTargetPosition);

			if (MoveDistanceToTarget < 120.0f)
			{
				PendingChangeState = cstECharacterState::IDLE;
			}

			break;
		}


		case cstECharacterState::CHASE_TARGET:
		{
			NS_Assert(ExecutingAbility);

			const nsVector3 targetPosition = ExecutionTarget.Character->GetWorldPosition();
			const float distanceToTarget = nsVector3::Distance(GetWorldPosition(), targetPosition);

			if (distanceToTarget > ExecutingAbility->GetCastingDistance())
			{
				if (CanMove())
				{
					NavigationAgentComponent->SetNavigationTarget(targetPosition);
				}
			}
			else
			{
				if (ExecutingAbility->Execute(GetWorld()->GetCurrentTimeSeconds(), this, ExecutionTarget) == cstEAbilityExecutionResult::SUCCESS)
				{
					Abilities.Add(ExecutingAbility);
					PendingChangeState = cstECharacterState::EXECUTE_ABILITY;
				}
				else
				{
					PendingChangeState = cstECharacterState::IDLE;
				}
			}

			break;
		}


		case cstECharacterState::EXECUTE_ABILITY:
		{
			NS_Assert(ExecutingAbility);

			break;
		}


		case cstECharacterState::KO:
		{
			break;
		}

		default: break;
	}


	if (PendingChangeState != cstECharacterState::NONE)
	{
		bool bCanTransitionState = false;

		switch (CurrentState)
		{
			case cstECharacterState::IDLE:
			{
				bCanTransitionState = true;
				break;
			}


			case cstECharacterState::MOVE:
			case cstECharacterState::CHASE_TARGET:
			{
				NavigationAgentComponent->StopMovement();
				bCanTransitionState = true;
				break;
			}


			case cstECharacterState::EXECUTE_ABILITY:
			{
				NS_Assert(ExecutingAbility);

				break;
			}


			case cstECharacterState::KO:
			{
				break;
			}

			default: break;
		}


		if (bCanTransitionState)
		{
			if (PendingChangeState == cstECharacterState::MOVE || PendingChangeState == cstECharacterState::CHASE_TARGET)
			{
				if (CanMove())
				{
					NavigationAgentComponent->SetNavigationTarget(MoveTargetPosition);
				}
				else
				{
					// Cancel transition state but don't reset PendingChangeState for next frame checking
					bCanTransitionState = false;
				}
			}
			else if (PendingChangeState == cstECharacterState::KO)
			{
				NS_CONSOLE_Log(cstCharacterLog, TEXT("Character [%s] KO!"), *Name);
				// TODO: Play dead animation (non-loop)
			}
		}


		// If transition state is not canceled, change CurrentState and reset PendingChangeState
		if (bCanTransitionState)
		{
			CurrentState = PendingChangeState;
			PendingChangeState = cstECharacterState::NONE;
		}
	}
}


void cstCharacter::UpdateAnimation(float deltaTime)
{
	const float currentSpeed = NavigationAgentComponent->GetCurrentVelocity().GetMagnitude();

	if (currentSpeed > 30.0f)
	{
		SkelMeshComponent->PlayAnimation(AnimRunForwardLoop, 1.0f, true);
	}
	else
	{
		SkelMeshComponent->PlayAnimation(AnimIdle0, 1.0f, true);
	}
}


void cstCharacter::EquipWeapon(cstWeapon* weapon)
{
	if (EquippedWeapon == weapon)
	{
		return;
	}

	EquippedWeapon = weapon;

	if (Abilities[0])
	{
		ns_DestroyObjectByClass(Abilities[0]);
		Abilities[0] = nullptr;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToComponent(SkelMeshComponent, nsETransformAttachmentMode::RESET_TRANSFORM, EquippedWeapon->AttachSocketName);

		if (EquippedWeapon->AttackAbilityClass)
		{
			Abilities[0] = ns_CreateObjectByClass<cstAbility>(EquippedWeapon->AttackAbilityClass);
		}
		else
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to create attack ability object from weapon [%s]. AttackAbilityClass is NULL!"), *EquippedWeapon->Name);
		}
	}
}


void cstCharacter::EquipArmor(cstArmor* armor)
{
}


void cstCharacter::ExecuteAbility(cstAbility* ability, const cstAbilityExecutionTarget& targetParams)
{
	if (ability == nullptr)
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to execute ability. <ability> is NULL!"));
		return;
	}

	ExecutionTarget = targetParams;
	ExecutingAbility = ability;
	PendingChangeState = cstECharacterState::CHASE_TARGET;
}


void cstCharacter::UseItem(cstItem* item, const cstAbilityExecutionTarget& targetParams)
{
	if (item == nullptr)
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to use item. <item> is NULL!"));
		return;
	}


}


void cstCharacter::Move(const nsVector3& worldPosition)
{
	MoveTargetPosition = worldPosition;
	PendingChangeState = cstECharacterState::MOVE;
}


void cstCharacter::Attack(cstCharacter* targetCharacter)
{
	if (targetCharacter == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Ignore attack command. targetCharacter is NULL!"));
		return;
	}

	if (EquippedWeapon == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Cannot attack. No weapon equipped!"));
		return;
	}

	if (Abilities[0] == nullptr)
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to attack by using weapon [%s]. Abilities[0] (Attack ability) is NULL!"), *EquippedWeapon->Name);
		return;
	}

	NS_CONSOLE_Log(cstCharacterLog, TEXT("Character [%s] attacking [%s]"), *Name, *targetCharacter->Name);

	ExecuteAbility(Abilities[0], cstAbilityExecutionTarget(targetCharacter));
}


void cstCharacter::Stop()
{
	PendingChangeState = cstECharacterState::IDLE;
}



#ifdef CST_GAME_WITH_EDITOR

constexpr const wchar_t* CharacterStateNames[6] =
{
	TEXT("None"),
	TEXT("Idle"),
	TEXT("Move"),
	TEXT("Chase_Target"),
	TEXT("Execute_Ability"),
	TEXT("KO")
};


void cstCharacter::DebugGUI(nsGUIContext& context)
{
	const nsGUIRect canvasRect = context.GetCanvasRect();

	nsGUIRect regionRect;
	regionRect.Left = 8.0f;
	regionRect.Top = canvasRect.GetHeight() * 0.5f;
	regionRect.Right = regionRect.Left + 512.0f;
	regionRect.Bottom = regionRect.Top + 512.0f;

	context.BeginRegion(nullptr, regionRect, nsPointFloat(), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false, "char_debug_region");
	{
		context.AddControlText(*Name);

		static nsString debugText;

		debugText = nsString::Format(TEXT("Health: %.3f/%.3f"), BaseAttributes[cstAttribute::CURRENT_HEALTH], BaseAttributes[cstAttribute::MAX_HEALTH]);
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("DamageHealth: %.3f"), BaseAttributes[cstAttribute::DAMAGE_HEALTH]);
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("Status: %s"), IsAlive() ? TEXT("-") : TEXT("[KO]"));
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("PendingChangeState: %s"), CharacterStateNames[static_cast<uint8>(PendingChangeState)]);
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("CurrentState: %s"), CharacterStateNames[static_cast<uint8>(CurrentState)]);
		context.AddControlText(*debugText);

		switch (CurrentState)
		{
			case cstECharacterState::MOVE:
			case cstECharacterState::CHASE_TARGET:
			{
				if (CurrentState == cstECharacterState::CHASE_TARGET)
				{
					debugText = nsString::Format(TEXT("Chase: %s"), *ExecutionTarget.Character->Name);
					context.AddControlText(*debugText);
				}

				debugText = nsString::Format(TEXT("Distance: %.3f"), MoveDistanceToTarget);
				context.AddControlText(*debugText);

				break;
			}

			case cstECharacterState::EXECUTE_ABILITY:
			{
				if (ExecutingAbility)
				{
					debugText = nsString::Format(TEXT("Ability: %s"), *ExecutingAbility->Name);
					context.AddControlText(*debugText);
					debugText = nsString::Format(TEXT("CastingRemainingTime: %.3f"), ExecutingAbility->GetCastingRemainingTime());
					context.AddControlText(*debugText);
					debugText = nsString::Format(TEXT("CooldownRemainingTime: %.3f"), ExecutingAbility->GetCooldownRemainingTime());
					context.AddControlText(*debugText);
				}

				break;
			}

			default: break;
		}

		debugText = TEXT("\nActive Abilities:");
		context.AddControlText(*debugText);

		for (int i = 0; i < Abilities.GetCount(); ++i)
		{
			cstAbility* ability = Abilities[i];
			debugText = nsString::Format(TEXT("Ability: %s, CooldownRemainingTime: %.3f"), *ability->Name, ability->GetCooldownRemainingTime());
			context.AddControlText(*debugText);
		}
	}
	context.EndRegion();
}


void cstCharacter::DebugDraw(nsRenderer* renderer)
{

}

#endif // CST_GAME_WITH_EDITOR
