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
	AbilityIndex = -1;

	Command = cstECharacterCommand::NONE;
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


bool cstCharacter::MoveToTargetToExecuteAbility()
{
	NS_Assert(AbilityIndex >= 0 && AbilityIndex < Abilities.GetCount());

	const cstAbilityExecutionTarget& executionTarget = Abilities[AbilityIndex].ExecutionTarget;

	const nsVector3 targetPosition = executionTarget.Character->GetWorldPosition();
	cstAbility* ability = Abilities[AbilityIndex].Instance;
	int level = Abilities[AbilityIndex].Level;
	MoveDistanceToTarget = nsVector3::DistanceXZ(GetWorldPosition(), targetPosition);
	
	if (MoveDistanceToTarget <= ability->GetCastingDistance(level))
	{
		NavigationAgentComponent->StopMovement();
		return true;
	}

	NavigationAgentComponent->SetNavigationTarget(targetPosition);

	return false;
}


void cstCharacter::UpdateActiveEffects(float deltaTime)
{
	const float currentTime = GetWorld()->GetCurrentTimeSeconds();
	TempAttributes = cstAttributes();

	if (!ActiveEffects.IsEmpty())
	{
		for (int i = 0; i < ActiveEffects.GetCount();)
		{
			cstEffectExecution* effectExecution = ActiveEffects[i];

			if (effectExecution->IsActive())
			{
				if (effectExecution->IsInstant())
				{
					effectExecution->UpdateEffect(deltaTime, currentTime, BaseAttributes);
				}
				else
				{
					effectExecution->UpdateEffect(deltaTime, currentTime, TempAttributes);
				}

				++i;
			}
			else
			{
				ActiveEffects.RemoveAt(i);
			}
		}
	}

	CurrentAttributes = BaseAttributes + TempAttributes;
}


void cstCharacter::UpdateActiveAbilities(float deltaTime)
{
	const float currentTime = GetWorld()->GetCurrentTimeSeconds();

	for (int i = 0; i < Abilities.GetCount(); ++i)
	{
		NS_Assert(Abilities[0].IsValid());
		Abilities[i].Instance->TickUpdate(deltaTime, currentTime);
	}
}


void cstCharacter::UpdateState(float deltaTime)
{
	if (CurrentAttributes[cstAttribute::CURRENT_HEALTH] <= 0.0f)
	{
		PendingChangeState = cstECharacterState::KO;
	}
	else if (OwningTags & cstTag::CHARACTER_DISABLED)
	{
		PendingChangeState = cstECharacterState::DISABLED;
	}
	else
	{
		switch (Command)
		{
			case cstECharacterCommand::STOP: PendingChangeState = cstECharacterState::IDLE; break;
			case cstECharacterCommand::MOVE: PendingChangeState = cstECharacterState::MOVE; break;
			case cstECharacterCommand::ATTACK: PendingChangeState = cstECharacterState::ATTACK_TARGET; break;
			case cstECharacterCommand::ABILITY:	PendingChangeState = cstECharacterState::EXECUTE_ABILITY; break;
			case cstECharacterCommand::ITEM: PendingChangeState = cstECharacterState::USE_ITEM; break;
			case cstECharacterCommand::INTERACT: PendingChangeState = cstECharacterState::INTERACT; break;
			default: break;
		}

		Command = cstECharacterCommand::NONE;
	}


	if (PendingChangeState != cstECharacterState::NONE)
	{
		bool bCanTransitionState = false;

		switch (CurrentState)
		{
			case cstECharacterState::IDLE:
			case cstECharacterState::MOVE:
			{
				bCanTransitionState = true;
				break;
			}

			case cstECharacterState::ATTACK_TARGET:
			case cstECharacterState::EXECUTE_ABILITY:
			{
				NS_Assert(AbilityIndex >= 0 && AbilityIndex < Abilities.GetCount());
				bCanTransitionState = Abilities[AbilityIndex].Instance->CanBeCancelled(Abilities[AbilityIndex].Level);
				break;
			}

			case cstECharacterState::USE_ITEM:
				break;
			case cstECharacterState::INTERACT:
				break;
			case cstECharacterState::DISABLED:
				break;
			case cstECharacterState::KO:
				break;
			default:
				break;
		}


		if (bCanTransitionState)
		{
			// Exit
			switch (CurrentState)
			{
				case cstECharacterState::IDLE:
				{
					break;
				}

				case cstECharacterState::MOVE:
				{
					if (PendingChangeState != cstECharacterState::MOVE)
					{
						NavigationAgentComponent->StopMovement();
					}

					break;
				}

				case cstECharacterState::ATTACK_TARGET:
				{
					break;
				}

				case cstECharacterState::EXECUTE_ABILITY:
				{
					break;
				}

				case cstECharacterState::USE_ITEM:
				{
					break;
				}

				case cstECharacterState::INTERACT:
				{
					break;
				}

				case cstECharacterState::DISABLED:
				case cstECharacterState::KO:
				{
					break;
				}

				default: break;
			}


			CurrentState = PendingChangeState;


			// Enter
			switch (CurrentState)
			{
				case cstECharacterState::IDLE:
				{
					break;
				}

				case cstECharacterState::MOVE:
				{
					NavigationAgentComponent->SetNavigationTarget(MoveTargetPosition);
					break;
				}

				case cstECharacterState::ATTACK_TARGET:
				{
					break;
				}

				case cstECharacterState::EXECUTE_ABILITY:
				{
					break;
				}

				case cstECharacterState::USE_ITEM:
				{
					break;
				}

				case cstECharacterState::INTERACT:
				{
					break;
				}

				case cstECharacterState::DISABLED:
				{
					break;
				}

				case cstECharacterState::KO:
				{
					break;
				}

				default: break;
			}

			PendingChangeState = cstECharacterState::NONE;
		}
	}


	// Update
	switch (CurrentState)
	{
		case cstECharacterState::IDLE:
		{
			// Nothing to do
			break;
		}


		case cstECharacterState::MOVE:
		{
			MoveDistanceToTarget = nsVector3::DistanceXZ(GetWorldPosition(), MoveTargetPosition);

			if (MoveDistanceToTarget < 64.0f)
			{
				PendingChangeState = cstECharacterState::IDLE;
			}

			break;
		}


		case cstECharacterState::ATTACK_TARGET:
		{
			if (MoveToTargetToExecuteAbility())
			{
				cstAbility* ability = Abilities[0].Instance;
				const cstAbilityExecutionTarget& executionTarget = Abilities[0].ExecutionTarget;
				const int level = Abilities[0].Level;

				ability->Execute(GetWorld()->GetCurrentTimeSeconds(), this, executionTarget, level);
			}

			break;
		}


		case cstECharacterState::EXECUTE_ABILITY:
		{
			break;
		}


		case cstECharacterState::USE_ITEM:
		{
			break;
		}


		case cstECharacterState::INTERACT:
		{
			break;
		}


		case cstECharacterState::DISABLED:
		case cstECharacterState::KO:
		{
			break;
		}

		default: break;
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


cstEffectExecution* cstCharacter::FindActiveEffect(cstTags effectTags, cstEffectExecution* effectExecution) const
{
	for (int i = 0; i < ActiveEffects.GetCount(); ++i)
	{
		cstEffectExecution* check = ActiveEffects[i];

		if (check->GetEffectTags() == effectTags && check == effectExecution)
		{
			return check;
		}
	}

	return nullptr;
}


void cstCharacter::EquipWeapon(cstWeapon* weapon)
{
	if (EquippedWeapon == weapon)
	{
		return;
	}

	EquippedWeapon = weapon;

	if (Abilities[0].Instance)
	{
		ns_DestroyObjectByClass(Abilities[0].Instance);
		Abilities[0] = cstCharacterAbility();
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToComponent(SkelMeshComponent, nsETransformAttachmentMode::RESET_TRANSFORM, EquippedWeapon->AttachSocketName);

		if (EquippedWeapon->AttackAbilityClass)
		{
			cstCharacterAbility& ability = Abilities[0];
			ability.Class = EquippedWeapon->AttackAbilityClass;
			ability.Instance = ns_CreateObjectByClass<cstAbility>(ability.Class);
		}
		else
		{
			NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to create attack ability from weapon [%s]. AttackAbilityClass is NULL!"), *EquippedWeapon->Name);
		}
	}
}


void cstCharacter::EquipArmor(cstArmor* armor)
{
}


void cstCharacter::AddAbility(const nsClass* abilityClass)
{
	if (abilityClass == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Fail to add ability. <abilityClass> is NULL!"));
		return;
	}

	NS_Validate(abilityClass->IsSubclassOf(cstAbility::Class));

	if (Abilities.Find(abilityClass) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Ignore add ability. Ability type [%s] already exists!"), *abilityClass->GetName().ToString());
		return;
	}

	cstCharacterAbility& newAbility = Abilities.Add();
	newAbility.Class = abilityClass;
	newAbility.Instance = ns_CreateObjectByClass<cstAbility>(abilityClass);
	newAbility.ExecutionTarget = cstAbilityExecutionTarget();
	newAbility.Level = 1;
}


void cstCharacter::RemoveAbility(const nsClass* abilityClass)
{
	if (abilityClass == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Fail to remove ability. <abilityClass> is NULL!"));
		return;
	}

	NS_Validate(abilityClass->IsSubclassOf(cstAbility::Class));

	const int index = Abilities.Find(abilityClass);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Ignore remove ability. Ability type [%s] not found!"), *abilityClass->GetName().ToString());
		return;
	}

	cstCharacterAbility& ability = Abilities[index];
	NS_Assert(ability.Instance);

	ns_DestroyObjectByClass(ability.Instance);
	ability.Class = nullptr;
	ability.Instance = nullptr;

	Abilities.RemoveAt(index);
}


void cstCharacter::ApplyEffect(cstEffectExecution* effectExecution, const cstEffectContext& effectContext)
{
	const float currentTime = GetWorld()->GetCurrentTimeSeconds();
	cstEffectExecution* activeEffect = FindActiveEffect(effectContext.EffectTags, effectExecution);

	if (activeEffect)
	{
		activeEffect->ApplyEffect(currentTime, this, effectContext);
		return;
	}

	if (effectExecution->ApplyEffect(currentTime, this, effectContext))
	{
		ActiveEffects.Add(effectExecution);
	}
}


void cstCharacter::RemoveEffect(cstTags effectTags)
{
	NS_ValidateV(0, TEXT("Not implemented yet!"));
}


void cstCharacter::CommandStop()
{
	Command = cstECharacterCommand::STOP;
}


void cstCharacter::CommandMove(const nsVector3& worldPosition)
{
	MoveTargetPosition = worldPosition;
	Command = cstECharacterCommand::MOVE;
}


void cstCharacter::CommandAttack(cstCharacter* character)
{
	if (character == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Ignore attack command. <character> is NULL!"));
		return;
	}

	if (EquippedWeapon == nullptr)
	{
		NS_CONSOLE_Log(cstCharacterLog, TEXT("Cannot attack. No weapon equipped!"));
		return;
	}

	if (!Abilities[0].IsValid())
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to attack by using weapon [%s]. Abilities[0] (Attack ability) is invalid!"), *EquippedWeapon->Name);
		return;
	}

	NS_CONSOLE_Log(cstCharacterLog, TEXT("Character [%s] attacking [%s]"), *Name, *character->Name);

	AbilityIndex = 0;
	Abilities[AbilityIndex].ExecutionTarget = cstAbilityExecutionTarget(character);
	Abilities[AbilityIndex].Level = 1;
	Command = cstECharacterCommand::ATTACK;
}


void cstCharacter::CommandExecuteAbility(const nsClass* abilityClass, const cstAbilityExecutionTarget& targetParams, int level)
{
	if (abilityClass == nullptr)
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to execute ability. <abilityClass> is NULL!"));
		return;
	}

	NS_Validate(abilityClass->IsSubclassOf(cstAbility::Class));

}


void cstCharacter::CommandUseItem(cstItem* item, const cstAbilityExecutionTarget& targetParams)
{
	if (item == nullptr)
	{
		NS_CONSOLE_Warning(cstCharacterLog, TEXT("Fail to use item. <item> is NULL!"));
		return;
	}


}


void cstCharacter::CommandInteract(nsActor* actor)
{
}





#ifdef CST_GAME_WITH_EDITOR

constexpr const wchar_t* CharacterStateNames[9] =
{
	TEXT("None"),
	TEXT("Idle"),
	TEXT("Move"),
	TEXT("Attack_Target"),
	TEXT("Execute_Ability"),
	TEXT("Use_Item"),
	TEXT("Interact"),
	TEXT("Disabled"),
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

		debugText = nsString::Format(TEXT("Health: %.3f/%.3f"), BaseAttributes[cstAttribute::CURRENT_HEALTH], BaseAttributes[cstAttribute::HEALTH]);
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("Status: %s"), IsAlive() ? TEXT("-") : TEXT("[KO]"));
		context.AddControlText(*debugText);

		debugText = nsString::Format(TEXT("\nPendingChangeState: %s"), CharacterStateNames[static_cast<uint8>(PendingChangeState)]);
		context.AddControlText(*debugText);
		debugText = nsString::Format(TEXT("CurrentState: %s"), CharacterStateNames[static_cast<uint8>(CurrentState)]);
		context.AddControlText(*debugText);

		switch (CurrentState)
		{
			case cstECharacterState::MOVE:
			{
				debugText = nsString::Format(TEXT("Location: %.3f, %.3f, %.3f"), MoveTargetPosition.X, MoveTargetPosition.Y, MoveTargetPosition.Z);
				context.AddControlText(*debugText);
				debugText = nsString::Format(TEXT("Distance: %.3f"), MoveDistanceToTarget);
				context.AddControlText(*debugText);

				break;
			}

			case cstECharacterState::ATTACK_TARGET:
			case cstECharacterState::EXECUTE_ABILITY:
			{
				if (AbilityIndex >= 0 && AbilityIndex < Abilities.GetCount())
				{
					const cstAbilityExecutionTarget& target = Abilities[AbilityIndex].ExecutionTarget;
					debugText = nsString::Format(TEXT("Target: %s"), target.Character ? *target.Character->Name : TEXT("-"));
					context.AddControlText(*debugText);
					debugText = nsString::Format(TEXT("Distance: %.3f"), MoveDistanceToTarget);
					context.AddControlText(*debugText);

					cstAbility* ability = Abilities[AbilityIndex].Instance;
					debugText = nsString::Format(TEXT("Ability: %s"), *ability->Name);
					context.AddControlText(*debugText);
					debugText = nsString::Format(TEXT("Casting: [Distance: %.3f] [RemainingTime: %.3f]"), ability->GetCastingDistance(Abilities[AbilityIndex].Level), ability->GetCastingRemainingTime());
					context.AddControlText(*debugText);
				}

				break;
			}

			default: break;
		}

		debugText = TEXT("\nAbilities:");
		context.AddControlText(*debugText);

		for (int i = 0; i < Abilities.GetCount(); ++i)
		{
			cstAbility* ability = Abilities[i].Instance;
			debugText = nsString::Format(TEXT("%s: [%s] [Cooldown: %.3f]"), *ability->Name, ability->IsActive() ? TEXT("Active") : TEXT("Inactive"), ability->GetCooldownRemainingTime());
			context.AddControlText(*debugText);
		}
	}
	context.EndRegion();
}


void cstCharacter::DebugDraw(nsRenderer* renderer)
{

}

#endif // CST_GAME_WITH_EDITOR
