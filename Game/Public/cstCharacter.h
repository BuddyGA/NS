#pragma once

#include "cstTypes.h"
#include "nsActor.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"



class cstCharacterAttributes
{
public:
	// Strength (PATK)
	int STR;

	// Vitality (MaxHealth, PDEF)
	int VIT;

	// Intelligence (MaxMana, MATK)
	int INT;

	// Mentality (MDEF, CSPD)
	int MEN;

	// Dexterity (ASPD)
	int DEX;

	// Agility (MSPD)
	int AGI;

	// Luck (Critical%)
	int LUK;

	// Maximum health
	int MaxHealth;

	// Maximum mana
	int MaxMana;

	// Physical attack 
	int PATK;

	// Physical defense
	int PDEF;

	// Magical attack
	int MATK;

	// Magical defense
	int MDEF;

	// Attack speed
	int ASPD;

	// Casting speed
	int CSPD;

	// Movement speed
	int MSPD;

	// Critical rate (%)
	int CritRate;

	// Poison resistance
	int PoisonResistance;

	// Burn resistance
	int BurnResistance;

	// Stun resistance
	int StunResistance;

	// Slow resistance
	int SlowResistance;


public:
	cstCharacterAttributes()
	{
		STR = 5;
		VIT = 5;
		INT = 5;
		MEN = 5;
		DEX = 5;
		AGI = 5;
		LUK = 5;

		MaxHealth = 100;
		MaxMana = 100;
		PATK = 10;
		PDEF = 5;
		MATK = 10;
		MDEF = 5;
		ASPD = 100;
		CSPD = 100;
		MSPD = 300;
		CritRate = 5;

		PoisonResistance = 50;
		BurnResistance = 50;
		StunResistance = 50;
		SlowResistance = 50;
	}

};



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT()

private:
	nsCharacterMovementComponent* MovementComponent;
	nsSkeletalMeshComponent* MeshComponent;
	cstCharacterAttributes Attributes;


public:
	cstCharacter();
	virtual void OnInitialize() override;
	virtual void OnDestroy() override;
	void Move(float deltaTime, const nsVector3& worldDirection);


public:
	NS_NODISCARD_INLINE const cstCharacterAttributes& GetAttributes() const
	{
		return Attributes;
	}

};
