#pragma once

#include "nsActorComponents.h"



class cstAttributeComponent : public nsActorComponent
{
	NS_DECLARE_OBJECT()

private:
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
	cstAttributeComponent();

};
