#include "cstCharacter.h"
#include "nsAssetManager.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"
#include "nsNavigationComponents.h"
#include "cstAbility.h"



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
	AttackTargetCharacter = nullptr;
	MoveDistanceToTarget = 0.0f;
	ExecutingAbility = nullptr;

	Attributes = cstAttributes::GetCharacterBaseAttributes();
	StatusEffects = cstEStatusEffect::Normal;
	Team = cstECharacterTeam::NEUTRAL;
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


	switch (CurrentState)
	{
		case cstECharacterState::MOVING:
		{
			// Approaching target
			if (AttackTargetCharacter)
			{
				NS_Assert(ExecutingAbility);

				MoveDistanceToTarget = nsVector3::Distance(GetWorldPosition(), AttackTargetCharacter->GetWorldPosition());

				if (MoveDistanceToTarget <= ExecutingAbility->GetCastingDistance())
				{
					PendingChangeState = cstECharacterState::EXECUTING;
				}
			}

			// Default movement 
			else
			{
				MoveDistanceToTarget = nsVector3::Distance(GetWorldPosition(), MoveTargetPosition);

				if (MoveDistanceToTarget < 120.0f)
				{
					PendingChangeState = cstECharacterState::IDLE;
				}
			}

			break;
		}

		case cstECharacterState::EXECUTING:
		{
			NS_Assert(ExecutingAbility);

			ExecutingAbility->UpdateExecution(deltaTime);

			if (ExecutingAbility->GetExecutionRemainingTime() <= 0.1f)
			{
				if (PendingChangeState == cstECharacterState::NONE)
				{
					PendingChangeState = cstECharacterState::IDLE;
				}
			}

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

			case cstECharacterState::MOVING:
			{
				NavigationAgentComponent->StopMovement();
				bCanTransitionState = true;
				break;
			}

			case cstECharacterState::EXECUTING:
			{
				NS_Assert(ExecutingAbility);

				if (ExecutingAbility->GetExecutionRemainingTime() <= 0.0f)
				{
					ExecutingAbility = nullptr;
					bCanTransitionState = true;
				}

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
			if (PendingChangeState == cstECharacterState::MOVING)
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


	UpdateAnimation();
}


void cstCharacter::UpdateAnimation()
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


void cstCharacter::SetMoveTargetPosition(const nsVector3& worldPosition)
{
	AttackTargetCharacter = nullptr;
	MoveTargetPosition = worldPosition;
	PendingChangeState = cstECharacterState::MOVING;
}


void cstCharacter::ExecuteAbility(cstAbility* ability, const cstExecute::TargetParams& targetParams)
{
	if (ability == nullptr)
	{
		NS_CONSOLE_Warning(cstPlayerLog, TEXT("Fail to execute ability. <ability> is NULL!"));
		return;
	}

	ExecutingAbility = ability;
	PendingChangeState = cstECharacterState::EXECUTING;
}


void cstCharacter::UseItem(cstItem* item, const cstExecute::TargetParams& targetParams)
{
	if (item == nullptr)
	{
		NS_CONSOLE_Warning(cstPlayerLog, TEXT("Fail to use item. <item> is NULL!"));
		return;
	}


}


void cstCharacter::StopAction()
{
	AttackTargetCharacter = nullptr;
	PendingChangeState = cstECharacterState::IDLE;
}


void cstCharacter::ApplyKO()
{
	Attributes.Health = 0.0f;
	PendingChangeState = cstECharacterState::KO;
}



#ifdef CST_GAME_WITH_EDITOR

static const nsString CharacterStateNames[5] =
{
	TEXT("None"),
	TEXT("Idle"),
	TEXT("Moving"),
	TEXT("Executing"),
	TEXT("KO")
};


void cstCharacter::DebugGUI(nsGUIContext& context)
{
	const nsGUIRect canvasRect = context.GetCanvasRect();

	nsGUIRect regionRect;
	regionRect.Left = 8.0f;
	regionRect.Top = canvasRect.GetHeight() * 0.5f;
	regionRect.Right = regionRect.Left + 256.0f;
	regionRect.Bottom = regionRect.Top + 256.0f;

	context.BeginRegion(nullptr, regionRect, nsPointFloat(), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false, "char_debug_region");
	{
		context.AddControlText(*Name);

		static nsString pendingChangeStateText;
		pendingChangeStateText = nsString::Format(TEXT("PendingChangeState: %s"), *CharacterStateNames[static_cast<uint8>(PendingChangeState)]);
		context.AddControlText(*pendingChangeStateText);

		static nsString currentStateText;
		currentStateText = nsString::Format(TEXT("CurrentState: %s"), *CharacterStateNames[static_cast<uint8>(CurrentState)]);
		context.AddControlText(*currentStateText);

		switch (CurrentState)
		{
			case cstECharacterState::MOVING:
			{
				static nsString distanceText;
				distanceText = nsString::Format(TEXT("Distance: %.3f"), MoveDistanceToTarget);
				context.AddControlText(*distanceText);

				break;
			}

			case cstECharacterState::EXECUTING:
			{
				if (ExecutingAbility)
				{
					static nsString abilityNameText;
					abilityNameText = nsString::Format(TEXT("Ability: %s"), *ExecutingAbility->Name);
					context.AddControlText(*abilityNameText);

					static nsString abilityExecuteTimerText;
					abilityExecuteTimerText = nsString::Format(TEXT("ExecutionRemainingTime: %.3f"), ExecutingAbility->GetExecutionRemainingTime());
					context.AddControlText(*abilityExecuteTimerText);
				}

				break;
			}

			case cstECharacterState::KO:
			{
				break;
			}

			default: break;
		}
	}
	context.EndRegion();
}


void cstCharacter::DebugDraw(nsRenderer* renderer)
{

}

#endif // CST_GAME_WITH_EDITOR




NS_CLASS_BEGIN(cstPlayerCharacter, cstCharacter)
NS_CLASS_END(cstPlayerCharacter)

cstPlayerCharacter::cstPlayerCharacter()
{
	Attributes.FireResistance = 5.0f;
	Attributes.WaterResistance = 5.0f;
	Attributes.WindResistance = 5.0f;
	Attributes.EarthResistance = 5.0f;

	Team = cstECharacterTeam::PLAYER;

	AbilitySlots.Resize(cstInputAction::ABILITY_SLOT_MAX_COUNT);
	AbilitySlots[ MapToAbilitySlotIndex(cstInputAction::ABILITY_SLOT_STOP) ] = ns_CreateObject<cstAbility_StopAction>();

	ItemSlots.Resize(cstInputAction::ITEM_SLOT_MAX_COUNT);
}




NS_CLASS_BEGIN(cstEnemyCharacter, cstCharacter)
NS_CLASS_END(cstEnemyCharacter)

cstEnemyCharacter::cstEnemyCharacter()
{
	Team = cstECharacterTeam::ENEMY;
}
