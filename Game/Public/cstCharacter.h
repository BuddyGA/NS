#pragma once

#include "cstTypes.h"
#include "nsActor.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"



class cstCharacterAttributes
{
public:
	// Strength (PATK)
	uint8 STR;

	// Vitality (MaxHealth, PDEF)
	uint8 VIT;

	// Intelligence (MaxMana, MATK)
	uint8 INT;

	// Mentality (MDEF, CSPD)
	uint8 MEN;

	// Agility (ASPD, MSPD)
	uint8 AGI;

	// Luck (Critical%)
	uint8 LUK;

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


public:
	cstCharacterAttributes()
	{
		STR = 5;
		VIT = 5;
		INT = 5;
		MEN = 5;
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
	}

};



class cstCharacter : public nsActor
{
	NS_DECLARE_OBJECT()

private:
	//nsCapsuleCollisionComponent* CapsuleComponent;
	nsCharacterMovementComponent* MovementComponent;
	nsMeshComponent* MeshComponent;
	cstCharacterAttributes Attributes;

	nsTArray<nsPhysicsHitResult> MoveHitResults;


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
