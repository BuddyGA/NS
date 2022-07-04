#include "cstAttributeComponent.h"



NS_CLASS_BEGIN(cstAttributeComponent, nsActorComponent)
NS_CLASS_END(cstAttributeComponent)

cstAttributeComponent::cstAttributeComponent()
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
