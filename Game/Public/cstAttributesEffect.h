#pragma once

#include "cstTypes.h"



namespace cstAttribute
{
	enum EType : uint8
	{
		STR = 0,									// Strength (PATK)
		VIT,										// Vitality (Health, PDEF)
		INT,										// Intelligence (Mana, MATK)
		MEN,										// Mentality (MDEF, CSPD)
		DEX,										// Dexterity (ASPD)
		AGI,										// Agility (MSPD)
		CRT,										// Physical/Magical critical rate (%)
		HEALTH,										// Maximum health
		MANA,										// Maximum mana
		PATK,										// Physical attack
		PDEF,										// Physical defense
		MATK,										// Magical attack
		MDEF,										// Magical defense
		ASPD,										// Attack speed
		CSPD,										// Casting speed
		MSPD,										// Movement speed
		RESIST_ELEMENT_FIRE,						// Fire element resistance
		RESIST_ELEMENT_WATER,						// Water element resistance
		RESIST_ELEMENT_WIND,						// Wind element resistance
		RESIST_ELEMENT_EARTH,						// Earth element resistance
		RESIST_ELEMENT_LIGHT,						// Light element resistance
		RESIST_ELEMENT_DARK,						// Dark element resistance
		RESIST_STATUS_POISON,						// Poison status resistance
		RESIST_STATUS_BURN,							// Burn status resistance
		RESIST_STATUS_FREEZE,						// Freeze status resistance
		RESIST_STATUS_STUN,							// Stun status resistance
		RESIST_STATUS_SLOW,							// Slow status resistance
		RESIST_STATUS_SILENCE,						// Silence status resistance
		RESIST_STATUS_BLIND,						// Blind status resistance
		RESIST_STATUS_HATE,							// Hate status resistance

		CURRENT_HEALTH,								// Current health
		CURRENT_HEALTH_REGEN,						// Current health regeneration per second
		CURRENT_MANA,								// Current mana
		CURRENT_MANA_REGEN,							// Current mana regeneration per second
		CURRENT_STATUS_POISON,						// Current status meter: Poison 
		CURRENT_STATUS_BURN,						// Current status meter: Burn 
		CURRENT_STATUS_FREEZE,						// Current status meter: Freeze
		CURRENT_STATUS_STUN,						// Current status meter: Stun
		CURRENT_STATUS_SLOW,						// Current status meter: Slow
		CURRENT_STATUS_SILENCE,						// Current status meter: Silence
		CURRENT_STATUS_BLIND,						// Current status meter: Blind
		CURRENT_STATUS_HATE,						// Current status meter: Hate

		ABILITY_MANA_COST_REDUCTION,				// Ability mana cost reduction (%)
		ABILITY_COOLDOWN_REDUCTION,					// Ability cooldown reduction (%)

		MAX_COUNT
	};
};


class cstAttributes
{
private:
	float Values[cstAttribute::MAX_COUNT];


public:
	cstAttributes();
	cstAttributes Add(const cstAttributes& other);


	NS_INLINE cstAttributes operator+(const cstAttributes& rhs)
	{
		return Add(rhs);
	}


	NS_INLINE float& operator[](uint8 type)
	{
		NS_Validate(type >= 0 && type < cstAttribute::MAX_COUNT);
		return Values[type];
	}


	NS_INLINE const float& operator[](uint8 type) const
	{
		NS_Validate(type >= 0 && type < cstAttribute::MAX_COUNT);
		return Values[type];
	}


	static NS_INLINE cstAttributes InitializeCharacterBaseAttributes()
	{
		cstAttributes attributes;

		attributes[cstAttribute::STR]						= 5.0f;
		attributes[cstAttribute::VIT]						= 5.0f;
		attributes[cstAttribute::INT]						= 5.0f;
		attributes[cstAttribute::MEN]						= 5.0f;
		attributes[cstAttribute::DEX]						= 5.0f;
		attributes[cstAttribute::AGI]						= 5.0f;
		attributes[cstAttribute::CRT]						= 5.0f;
		attributes[cstAttribute::HEALTH]					= 100.0f;
		attributes[cstAttribute::MANA]						= 100.0f;
		attributes[cstAttribute::PATK]						= 10.0f;
		attributes[cstAttribute::PDEF]						= 5.0f;
		attributes[cstAttribute::MATK]						= 10.0f;
		attributes[cstAttribute::MDEF]						= 5.0f;
		attributes[cstAttribute::ASPD]						= 100.0f;
		attributes[cstAttribute::CSPD]						= 100.0f;
		attributes[cstAttribute::MSPD]						= 300.0f;
		attributes[cstAttribute::RESIST_ELEMENT_FIRE]		= 0.0f;
		attributes[cstAttribute::RESIST_ELEMENT_WATER]		= 0.0f;
		attributes[cstAttribute::RESIST_ELEMENT_WIND]		= 0.0f;
		attributes[cstAttribute::RESIST_ELEMENT_EARTH]		= 0.0f;
		attributes[cstAttribute::RESIST_ELEMENT_LIGHT]		= 0.0f;
		attributes[cstAttribute::RESIST_ELEMENT_DARK]		= 0.0f;
		attributes[cstAttribute::RESIST_STATUS_POISON]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_BURN]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_FREEZE]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_STUN]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_SLOW]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_SILENCE]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_BLIND]		= 50.0f;
		attributes[cstAttribute::RESIST_STATUS_HATE]		= 50.0f;
		attributes[cstAttribute::CURRENT_HEALTH]			= attributes[cstAttribute::HEALTH];
		attributes[cstAttribute::CURRENT_MANA]				= attributes[cstAttribute::MANA];

		return attributes;
	}

};




#define CST_EFFECT_INFINITE_DURATION	(999999.0f)
#define CST_EFFECT_MAX_STACK			(8)


struct cstEffectAttributeModification
{
	cstAttribute::EType Attribute;
	float Value;


public:
	cstEffectAttributeModification()
	{
		Attribute = cstAttribute::STR;
		Value = 0.0f;
	}


	NS_INLINE bool operator==(const cstEffectAttributeModification& rhs) const
	{
		return Attribute == rhs.Attribute;
	}


	NS_INLINE bool operator==(cstAttribute::EType rhs) const
	{
		return Attribute == rhs;
	}

};



struct cstEffectContext
{
	// Unique identifier tags for this effect. Used to apply stacking or for duration effect
	cstTags EffectTags;

	// Target must have this tags to apply this effect
	cstTags RequiredTags;

	// Target must NOT have this tags to apply this effect
	cstTags IgnoredTags;

	// Tags added to target when this effect applied
	cstTags AddedTags;

	// Effect duration in seconds
	float Duration;

	// Interval in seconds to process this effect (usually every 1 sec). Only works for duration > 0.0f
	float ProcInterval;

	// Maximum stack to apply this effect. Modified attributes will be accumulative. Only works if EffectTags provided
	int MaxStack;

	// Attributes modification when process this effect
	nsTArrayInline<cstEffectAttributeModification, cstAttribute::MAX_COUNT> AttributeModifications;


public:
	cstEffectContext()
	{
		EffectTags = cstTag::NONE;
		RequiredTags = cstTag::NONE;
		IgnoredTags = cstTag::NONE;
		AddedTags = cstTag::NONE;
		Duration = 0.0f;
		ProcInterval = 0.0f;
		MaxStack = 1;
	}


	NS_INLINE void AddAttributeModification(cstAttribute::EType attribute, float value)
	{
		cstEffectAttributeModification modify;
		modify.Attribute = attribute;
		modify.Value = value;

		AttributeModifications.Add(modify);
	}

};



class cstEffectExecution : public nsObject
{
	NS_DECLARE_OBJECT(cstEffectExecution)

private:
	cstEffectContext Context;
	float LastHitTime;
	float RemainingTime;
	int CurrentStack;
	int HitCount;
	cstCharacter* TargetCharacter;
	bool bIsActive;


public:
	cstEffectExecution();
	bool CanApply(const cstCharacter* character, const cstEffectContext& context) const;
	bool ApplyEffect(float currentTime, cstCharacter* character, const cstEffectContext& context);
	void UpdateEffect(float deltaTime, float currentTime, cstAttributes& outputAttributes);

protected:
	virtual void ModifyAttributes(cstAttributes& outputAttributes, const nsTArrayInline<cstEffectAttributeModification, cstAttribute::MAX_COUNT>& attributeModifications) {}

private:
	NS_INLINE void Reset()
	{
		RemainingTime = 0.0f;
		CurrentStack = 0;
		HitCount = 0;
		TargetCharacter = nullptr;
		bIsActive = false;
	}


public:
	NS_NODISCARD_INLINE cstTags GetEffectTags() const
	{
		return Context.EffectTags;
	}


	NS_NODISCARD_INLINE bool IsInstant() const
	{
		return Context.Duration <= 0.0f;
	}


	NS_NODISCARD_INLINE bool IsActive() const
	{
		return bIsActive;
	}


	NS_NODISCARD_INLINE float GetRemainingTime() const
	{
		return RemainingTime;
	}

};



class cstEffectExecution_Add : public cstEffectExecution
{
	NS_DECLARE_OBJECT(cstEffectExecution_Add)

public:
	cstEffectExecution_Add();
	virtual void ModifyAttributes(cstAttributes& outputAttributes, const nsTArrayInline<cstEffectAttributeModification, cstAttribute::MAX_COUNT>& attributeModifications) override;

};
