#pragma once

#include "cstTypes.h"
#include "nsAlgorithm.h"


#define CST_STATE_MACHINE_MAX_STATE		(8)



extern nsMemory g_StateMachineMemory;



class cstStateParameters
{
private:
	nsTMap<nsName, bool> Bools;
	nsTMap<nsName, int> Ints;
	nsTMap<nsName, float> Floats;
	nsTMap<nsName, nsVector3> Vectors;
	nsTMap<nsName, nsObject*> Objects;


public:
	cstStateParameters()
	{
	}


	NS_INLINE void SetBoolValue(nsName name, bool value)
	{
		Bools[name] = value;
	}


	NS_NODISCARD_INLINE bool GetBoolValue(nsName name) const
	{
		return Bools[name];
	}


	NS_INLINE void SetIntValue(nsName name, int value)
	{
		Ints[name] = value;
	}


	NS_NODISCARD_INLINE int GetIntValue(nsName name) const
	{
		return Ints[name];
	}


	NS_INLINE void SetFloatValue(nsName name, float value)
	{
		Floats[name] = value;
	}


	NS_NODISCARD_INLINE float GetFloatValue(nsName name) const
	{
		return Floats[name];
	}


	NS_INLINE void SetVectorValue(nsName name, nsVector3 value)
	{
		Vectors[name] = value;
	}


	NS_NODISCARD_INLINE nsVector3 GetVectorValue(nsName name) const
	{
		return Vectors[name];
	}


	NS_INLINE void SetObjectValue(nsName name, nsObject* value)
	{
		Objects[name] = value;
	}


	NS_NODISCARD_INLINE nsObject* GetObjectValue(nsName name) const
	{
		return Objects[name];
	}


	friend class cstStateMachine;

};




class cstIState
{
	NS_DECLARE_NOCOPY(cstIState)

public:
	cstIState() = default;
	virtual ~cstIState() = default;
	virtual void EnterState(const cstStateParameters& params) = 0;
	virtual void ExitState(const cstStateParameters& params) = 0;
	virtual bool UpdateState(float deltaTime, cstStateParameters& params) = 0;

};



class cstIStateTransition
{
	NS_DECLARE_NOCOPY(cstIStateTransition)

protected:
	cstIState* FromState;
	cstIState* ToState;
	bool bIsTwoWays;
	uint8 Priority;


public:
	cstIStateTransition(cstIState* from, cstIState* to, bool bTwoWays, uint8 priority = 255)
		: FromState(from)
		, ToState(to)
		, bIsTwoWays(bTwoWays)
		, Priority(priority)
	{
	}

	virtual ~cstIStateTransition() = default;
	virtual bool CanTransition(const cstStateParameters& params) const = 0;
	virtual bool CanTransitionReversed(const cstStateParameters& params) const = 0;


	friend class cstStateMachine;

};


template<typename TFromState, typename TToState>
class cstTStateTransition : public cstIStateTransition
{
public:
	cstTStateTransition(TFromState* from, TToState* to, bool bTwoWays, uint8 priority = 255)
		: cstIStateTransition(from, to, bTwoWays, priority)
	{
	}

};




class cstStateMachine
{
private:
	nsName Name;
	cstStateParameters Parameters;
	nsTArrayInline<cstIStateTransition*, 16> StateTransitions;
	cstIState* CurrentState;
	bool bHasStartedPlay;


public:
	cstStateMachine(nsName name);

private:
	cstIStateTransition* FindTransitionBetweenState(cstIState* from, cstIState* to) const;

public:
	void StartPlay(cstIState* entryState);
	void StopPlay();
	void TickUpdate(float deltaTime);


	NS_INLINE void SetParameterBoolValue(nsName name, bool value)
	{
		Parameters.SetBoolValue(name, value);
	}


	NS_NODISCARD_INLINE bool GetParameterBoolValue(nsName name) const
	{
		return Parameters.GetBoolValue(name);
	}


	NS_INLINE void SetParameterIntValue(nsName name, int value)
	{
		Parameters.SetIntValue(name, value);
	}


	NS_NODISCARD_INLINE int GetParameterIntValue(nsName name) const
	{
		return Parameters.GetIntValue(name);
	}


	NS_INLINE void SetParameterFloatValue(nsName name, float value)
	{
		Parameters.SetFloatValue(name, value);
	}


	NS_NODISCARD_INLINE float GetParameterFloatValue(nsName name) const
	{
		return Parameters.GetFloatValue(name);
	}


	NS_INLINE void SetParameterVectorValue(nsName name, nsVector3 value)
	{
		Parameters.SetVectorValue(name, value);
	}


	NS_NODISCARD_INLINE nsVector3 GetParameterVectorValue(nsName name) const
	{
		return Parameters.GetVectorValue(name);
	}


	NS_INLINE void SetParameterObjectValue(nsName name, nsObject* value)
	{
		Parameters.SetObjectValue(name, value);
	}


	template<typename T = nsObject>
	NS_NODISCARD_INLINE T* GetParameterObjectValue(nsName name) const
	{
		static_assert(std::is_base_of<nsObject, T>::value, "GetParameterObjectValue type of <T> must be derived from type <nsObject>!");
		return static_cast<T*>(Parameters.GetObjectValue(name));
	}


	template<typename TState, typename...TConstructorArgs>
	NS_INLINE TState* AddState(TConstructorArgs&&... args)
	{
		static_assert(std::is_base_of<cstIState, TState>::value, "AddState type of <TState> must be derived from type <cstIState>!");
		return g_StateMachineMemory.AllocateConstruct<TState>(std::forward<TConstructorArgs>(args)...);
	}


	template<typename TStateTransition, typename TFromState, typename TToState>
	NS_INLINE TStateTransition* AddTransition(TFromState* from, TToState* to, bool bTwoWays, uint8 priority = 255)
	{
		if (FindTransitionBetweenState(from, to))
		{
			return nullptr;
		}

		TStateTransition* transition = g_StateMachineMemory.AllocateConstruct<TStateTransition>(from, to, bTwoWays, priority);
		StateTransitions.Add(transition);
		
		nsAlgorithm::Sort(StateTransitions.GetData(), StateTransitions.GetCount(),
			[](cstIStateTransition* first, cstIStateTransition* second)
			{
				return first->Priority < second->Priority;
			}
		);

		return transition;
	}


public:
	static cstStateMachine* CreateStateMachine(nsName name)
	{
		return g_StateMachineMemory.AllocateConstruct<cstStateMachine>(name);
	}

};
