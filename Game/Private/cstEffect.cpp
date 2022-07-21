#include "cstEffect.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstEffectExecution, nsObject)
NS_CLASS_END(cstEffectExecution)

cstEffectExecution::cstEffectExecution()
{
	RemainingTime = 0.0f;
	CurrentStack = 0;
}


bool cstEffectExecution::CanApply(const cstCharacter* character, const cstEffectContext& context) const
{
	const cstTags characterOwningTags = character->GetOwningTags();

	// Must have required tags to apply the effect
	if ((context.RequiredTags != cstTag::NONE) && !(context.RequiredTags & characterOwningTags))
	{
		return false;
	}

	// Must NOT have ignored tags to apply this effect
	if ((context.IgnoredTags != cstTag::NONE) && (context.IgnoredTags & characterOwningTags))
	{
		return false;
	}

	return true;
}


bool cstEffectExecution::ApplyEffect(cstCharacter* character, const cstEffectContext& context)
{
	if (CanApply(character, context))
	{
		ModifyAttributes(character->GetBaseAttributes(), character->GetCurrentAttributes(), context);

		return true;
	}

	return false;
}


void cstEffectExecution::UpdateEffect(float deltaTime)
{

}
