#pragma once

#include "cstTypes.h"



class cstEquipment : public nsActor
{
	NS_DECLARE_OBJECT(nsActor)

public:
	cstAttributes Attributes;


public:
	cstEquipment();

};



class cstWeapon : public cstEquipment
{
	NS_DECLARE_OBJECT(cstWeapon)

public:
	cstWeapon();

};



class cstArmor : public cstEquipment
{
	NS_DECLARE_OBJECT(cstArmor)

public:
	cstArmor();

};
