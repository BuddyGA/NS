#include "cstStateMachine.h"



cstStateMachine::cstStateMachine(nsName name)
{
	Name = name;
	CurrentState = nullptr;
	bHasStartedPlay = false;
}


cstIStateTransition* cstStateMachine::FindTransitionBetweenState(cstIState* from, cstIState* to) const
{
	for (int i = 0; i < StateTransitions.GetCount(); ++i)
	{
		cstIStateTransition* check = StateTransitions[i];

		if (check->FromState == from && check->ToState == to)
		{
			return check;
		}
	}

	return nullptr;
}


void cstStateMachine::StartPlay(cstIState* entryState)
{
	if (!bHasStartedPlay)
	{
		CurrentState = entryState;
		bHasStartedPlay = true;
	}
}


void cstStateMachine::StopPlay()
{
	if (bHasStartedPlay)
	{
		bHasStartedPlay = false;
		CurrentState = nullptr;
	}
}


void cstStateMachine::TickUpdate(float deltaTime)
{
	if (!bHasStartedPlay)
	{
		return;
	}

	const bool bCanTransition = CurrentState->UpdateState(deltaTime, Parameters);

	if (!bCanTransition)
	{
		return;
	}

	cstIState* nextState = nullptr;

	for (int i = 0; i < StateTransitions.GetCount(); ++i)
	{
		cstIStateTransition* transition = StateTransitions[i];

		if (CurrentState != transition->FromState && !transition->bIsTwoWays)
		{
			continue;
		}

		if (CurrentState == transition->FromState && transition->CanTransition(Parameters))
		{
			nextState = transition->ToState;
			break;
		}

		if (transition->bIsTwoWays && transition->CanTransitionReversed(Parameters))
		{
			nextState = transition->FromState;
			break;
		}
	}

	if (CurrentState != nextState)
	{
		CurrentState->ExitState(Parameters);
		CurrentState = nextState;
		CurrentState->EnterState(Parameters);
	}
}
