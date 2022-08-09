#include "cstAttributesEffect.h"
#include "cstCharacter.h"



cstAttributes::cstAttributes()
	: Values()
{
}


cstAttributes cstAttributes::Add(const cstAttributes& other)
{
	cstAttributes attributes;

	for (int i = 0; i < cstAttribute::MAX_COUNT; ++i)
	{
		attributes.Values[i] = Values[i] + other.Values[i];
	}

	return attributes;
}





NS_CLASS_BEGIN(cstEffectExecution, nsObject)
NS_CLASS_END(cstEffectExecution)

cstEffectExecution::cstEffectExecution()
{
	LastHitTime = 0.0f;
	RemainingTime = 0.0f;
	CurrentStack = 0;
	HitCount = 0;
	TargetCharacter = nullptr;
	bIsActive = false;
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

	// For duration effect
	if (context.Duration > 0.0f && CurrentStack == context.MaxStack)
	{
		return false;
	}

	return true;
}


bool cstEffectExecution::ApplyEffect(float currentTime, cstCharacter* character, const cstEffectContext& context)
{
	if (CanApply(character, context))
	{
		Context = context;
		RemainingTime = Context.Duration;

		if (CurrentStack < Context.MaxStack)
		{
			CurrentStack++;
		}

		HitCount = 0;
		TargetCharacter = character;
		TargetCharacter->AddOwningTags(context.AddedTags);
		bIsActive = true;

		return true;
	}

	return false;
}


void cstEffectExecution::UpdateEffect(float deltaTime, float currentTime, cstAttributes& outputAttributes)
{
	if (!bIsActive)
	{
		return;
	}

	bool bApplyHit = false;

	if (HitCount == 0)
	{
		bApplyHit = true;
	}
	else
	{
		// If hit interval
		if (Context.ProcInterval > 0.0f)
		{
			bApplyHit = (Context.ProcInterval + LastHitTime) <= currentTime && CanApply(TargetCharacter, Context);
		}
		else
		{
			bApplyHit = CanApply(TargetCharacter, Context);
		}
	}
	
	if (bApplyHit)
	{
		LastHitTime = currentTime;
		HitCount++;
		ModifyAttributes(outputAttributes, Context.AttributeModifications);
	}

	RemainingTime = nsMath::Max(RemainingTime - deltaTime, 0.0f);

	// Instant effect or duration is over
	if (Context.Duration <= 0.0f || RemainingTime <= 0.0f)
	{
		TargetCharacter->RemoveOwningTags(Context.AddedTags);
		Reset();
	}
}





NS_CLASS_BEGIN(cstEffectExecution_Add, cstEffectExecution)
NS_CLASS_END(cstEffectExecution_Add)

cstEffectExecution_Add::cstEffectExecution_Add()
{
}

void cstEffectExecution_Add::ModifyAttributes(cstAttributes& outputAttributes, const nsTArrayInline<cstEffectAttributeModification, cstAttribute::MAX_COUNT>& attributeModifications)
{
	for (int i = 0; i < attributeModifications.GetCount(); ++i)
	{
		const cstEffectAttributeModification& modify = attributeModifications[i];
		NS_Assert(modify.Attribute >= 0 && modify.Attribute < cstAttribute::MAX_COUNT);

		outputAttributes[modify.Attribute] = nsMath::Max(outputAttributes[modify.Attribute] + modify.Value, 0.0f);
	}
}
