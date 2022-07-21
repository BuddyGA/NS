#pragma once

#include "cstEffect.h"



enum class cstEItemCategory : uint8
{
	CONSUMABLE = 0,
	ARMOR,
	WEAPON,
	KEY_ITEM
};



class cstItem : public nsObject
{
	NS_DECLARE_OBJECT(cstItem)

public:
	const nsClass* SpawnActorClass;

	cstEItemCategory Category;

	cstEffectContext Effect;

	nsString DisplayName;

	nsSharedTextureAsset DisplayIcon;


public:
	cstItem();

};



class cstEquipment : public nsActor
{
	NS_DECLARE_OBJECT(nsActor)

public:
	cstEffectContext Effect;
	nsName AttachSocketName;


public:
	cstEquipment();

};



class cstWeapon : public cstEquipment
{
	NS_DECLARE_OBJECT(cstWeapon)

public:
	const nsClass* AttackAbilityClass;


public:
	cstWeapon();

};



class cstArmor : public cstEquipment
{
	NS_DECLARE_OBJECT(cstArmor)

public:
	cstArmor();

};
