#pragma once

#include "cstStateMachine.h"
#include "cstCharacter.h"



#define CHARACTER_STATE_PARAM_OBJECT_TargetCharacter		"TargetCharacter"
#define CHARACTER_STATE_PARAM_VECTOR_TargetGroundPosition	"TargetGroundPosition"
#define CHARACTER_STATE_PARAM_VECTOR_MovetargetPosition		"MoveTargetPosition"
#define CHARACTER_STATE_PARAM_FLOAT_DistanceToTarget		"DistanceToTarget"
#define CHARACTER_STATE_PARAM_FLOAT_StopDistance			"StopDistance"
#define CHARACTER_STATE_PARAM_INT_AbilityIndex				"AbilityIndex"
#define CHARACTER_STATE_PARAM_BOOL_bHasMoveRequest			"bHasMoveRequest"



class cstCharacterState_Idle : public cstIState
{
private:
	cstCharacter* Character;

public:
	cstCharacterState_Idle(cstCharacter* character)
		: Character(character)
	{
	}

	void EnterState(const cstStateParameters& params) {}
	void ExitState(const cstStateParameters& params) {}
	bool UpdateState(float deltaTime, cstStateParameters& params) { return true; }

};



class cstCharacterState_Move : public cstIState
{
private:
	cstCharacter* Character;


public:
	cstCharacterState_Move(cstCharacter* character)
		: Character(character)
	{
	}


	void EnterState(const cstStateParameters& params)
	{
		Character->GetNavigationAgentComponent()->SetNavigationTarget(params.GetVectorValue(CHARACTER_STATE_PARAM_VECTOR_MovetargetPosition));
	}


	void ExitState(const cstStateParameters& params)
	{
		Character->GetNavigationAgentComponent()->StopMovement();
	}


	bool UpdateState(float deltaTime, cstStateParameters& params)
	{
		const float distance = nsVector3::DistanceXZ(Character->GetWorldPosition(), params.GetVectorValue(CHARACTER_STATE_PARAM_VECTOR_MovetargetPosition));
		params.SetFloatValue(CHARACTER_STATE_PARAM_FLOAT_DistanceToTarget, distance);

		return true;
	}

};



class cstCharacterState_Attack : public cstIState
{
private:
	cstCharacter* Character;


public:
	cstCharacterState_Attack(cstCharacter* character)
		: Character(character)
	{
	}


	void EnterState(const cstStateParameters& params)
	{

	}


	void ExitState(const cstStateParameters& params)
	{
	}


	bool UpdateState(float deltaTime, cstStateParameters& params)
	{
		return true;
	}

};





class cstCharacterStateTransition_Idle_Move : public cstTStateTransition<cstCharacterState_Idle, cstCharacterState_Move>
{
public:
	cstCharacter* Character;


public:
	cstCharacterStateTransition_Idle_Move(cstCharacterState_Idle* idleState, cstCharacterState_Move* moveState, bool bTwoWays, uint8 priority = 0)
		: cstTStateTransition(idleState, moveState, bTwoWays, priority)
		, Character(nullptr)
	{
	}


	bool CanTransition(const cstStateParameters& params) const
	{
		return params.GetBoolValue(CHARACTER_STATE_PARAM_BOOL_bHasMoveRequest);
	}


	bool CanTransitionReversed(const cstStateParameters& params) const
	{
		return params.GetFloatValue(CHARACTER_STATE_PARAM_FLOAT_DistanceToTarget) <= 64.0f;
	}

};



class cstCharacterStateTransition_Move_Attack : public cstTStateTransition<cstCharacterState_Move, cstCharacterState_Attack>
{
public:
	cstCharacter* Character;


public:
	cstCharacterStateTransition_Move_Attack(cstCharacterState_Move* moveState, cstCharacterState_Attack* attackState, bool bTwoWays, uint8 priority = 0)
		: cstTStateTransition(moveState, attackState, bTwoWays, priority)
		, Character(nullptr)
	{
	}


	bool CanTransition(const cstStateParameters& params) const
	{
		const cstCharacterAbility& ability = Character->GetAbility(params.GetIntValue(CHARACTER_STATE_PARAM_INT_AbilityIndex));
		const int level = ability.Level;
		const float distanceToTarget = params.GetFloatValue(CHARACTER_STATE_PARAM_FLOAT_DistanceToTarget);

		return distanceToTarget <= ability.Instance->GetCastingDistance(level);
	}


	bool CanTransitionReversed(const cstStateParameters& params) const
	{
		const cstCharacterAbility& ability = Character->GetAbility(params.GetIntValue(CHARACTER_STATE_PARAM_INT_AbilityIndex));
		const int level = ability.Level;

		if (ability.Instance->CanBeCancelled(level))
		{
			return true;
		}

		return !CanTransition(params);
	}

};
