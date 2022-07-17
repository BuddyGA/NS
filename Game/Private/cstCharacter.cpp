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

	Attributes.STR = 5.0f;
	Attributes.VIT = 5.0f;
	Attributes.INT = 5.0f;
	Attributes.MEN = 5.0f;
	Attributes.DEX = 5.0f;
	Attributes.AGI = 5.0f;
	Attributes.LUK = 5.0f;
	Attributes.MaxHealth = 100.0f;
	Attributes.MaxMana = 100.0f;
	Attributes.PATK = 10.0f;
	Attributes.PDEF = 5.0f;
	Attributes.MATK = 10.0f;
	Attributes.MDEF = 5.0f;
	Attributes.ASPD = 100.0f;
	Attributes.CSPD = 100.0f;
	Attributes.MSPD = 300.0f;
	Attributes.CritRate = 5.0f;
	Attributes.FireResistance = 5.0f;
	Attributes.WaterResistance = 5.0f;
	Attributes.WindResistance = 5.0f;
	Attributes.EarthResistance = 5.0f;
	Attributes.LightResistance = 5.0f;
	Attributes.DarkResistance = 5.0f;
	Attributes.PoisonResistance = 50.0f;
	Attributes.BurnResistance = 50.0f;
	Attributes.StunResistance = 50.0f;
	Attributes.SlowResistance = 50.0f;
	Attributes.Health = Attributes.MaxHealth;
	Attributes.HealthRegenPerSecond = 0.0f;
	Attributes.Mana = Attributes.MaxMana;
	Attributes.ManaRegenPerSecond = 1.0f;
	Attributes.AbilityManaCostReduction = 0.0f;
	Attributes.AbilityCooldownReduction = 0.0f;

	StatusFlags = 0;
	Team = cstECharacterTeam::NONE;

	AttackTargetCharacter = nullptr;
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


	switch (CurrentState)
	{
		case cstECharacterState::MOVING:
		{
			// Approaching target
			if (AttackTargetCharacter)
			{
				NS_Assert(ExecutingAbility);

				MoveDistanceToTarget = nsVector3::Distance(GetWorldPosition(), AttackTargetCharacter->GetWorldPosition());

				if (MoveDistanceToTarget <= ExecutingAbility->ExecuteDistance)
				{
					PendingChangeState = cstECharacterState::EXECUTING_ABILITY;
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

		case cstECharacterState::EXECUTING_ABILITY:
		{
			NS_Assert(ExecutingAbility);

			ExecutingAbility->UpdateExecution(deltaTime);

			if (ExecutingAbility->GetExecuteTimer() <= 0.1f)
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

			case cstECharacterState::EXECUTING_ABILITY:
			{
				NS_Assert(ExecutingAbility);

				if (ExecutingAbility->GetExecuteTimer() <= 0.0f)
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

		if (bCanTransitionState && PendingChangeState == cstECharacterState::MOVING)
		{
			if (CanMove())
			{
				NavigationAgentComponent->SetNavigationTarget(MoveTargetPosition);
			}
			else
			{
				bCanTransitionState = false;
			}
		}

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


void cstCharacter::StartExecuteAbility(cstAbility* ability)
{
	ExecutingAbility = ability;
	PendingChangeState = cstECharacterState::EXECUTING_ABILITY;
}




#ifdef CST_GAME_WITH_EDITOR

static const nsString CharacterStateNames[5] =
{
	TEXT("None"),
	TEXT("Idle"),
	TEXT("Moving"),
	TEXT("Executing_Ability"),
	TEXT("KO")
};


void cstCharacter::StopAction()
{
	AttackTargetCharacter = nullptr;
	PendingChangeState = cstECharacterState::IDLE;
}


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

			case cstECharacterState::EXECUTING_ABILITY:
			{
				if (ExecutingAbility)
				{
					static nsString abilityNameText;
					abilityNameText = nsString::Format(TEXT("Ability: %s"), *ExecutingAbility->Name);
					context.AddControlText(*abilityNameText);

					static nsString abilityExecuteTimerText;
					abilityExecuteTimerText = nsString::Format(TEXT("ExecuteTimer: %.3f"), ExecutingAbility->GetExecuteTimer());
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
	Team = cstECharacterTeam::PLAYER;
}
