#pragma once

#include "cstTypes.h"



#define CST_EFFECT_MAX_DURATION		(300.0f)
#define CST_EFFECT_MAX_STACK		(8)


struct cstEffectContext
{
	// Unique identifier tags for this effect. Used to apply stacking
	cstTags EffectTags;

	// Target must have this tags to apply this effect
	cstTags RequiredTags;

	// Target must NOT have this tags to apply this effect
	cstTags IgnoredTags;

	// Effect duration in seconds
	float Duration;

	// Interval in seconds to process this effect (usually every 1 sec). Only works for duration > 0.0f
	float ProcInterval;

	// Maximum stack to apply this effect. Modified attributes will be accumulative. Only works if EffectTags provided
	int MaxStack;

	// Attributes modification when process this effect
	cstAttributes ModifiedAttributes;


public:
	cstEffectContext()
	{
		EffectTags = cstTag::NONE;
		RequiredTags = cstTag::NONE;
		IgnoredTags = cstTag::NONE;
		Duration = 0.0f;
		ProcInterval = 0.0f;
		MaxStack = 1;
	}

};



class cstEffectExecution : public nsObject
{
	NS_DECLARE_OBJECT(cstEffectExecution)

private:
	cstEffectContext Context;
	float RemainingTime;
	int CurrentStack;


public:
	cstEffectExecution();
	bool CanApply(const cstCharacter* character, const cstEffectContext& context) const;
	bool ApplyEffect(cstCharacter* character, const cstEffectContext& context);
	void UpdateEffect(float deltaTime);

protected:
	virtual void ModifyAttributes(cstAttributes& baseAttributes, cstAttributes& currentAttributes, const cstEffectContext& context) {}


public:
	NS_NODISCARD_INLINE bool IsActive() const
	{
		return RemainingTime > 0.0f;
	}


	NS_NODISCARD_INLINE float GetRemainingTime() const
	{
		return RemainingTime;
	}

};
