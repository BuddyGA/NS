#pragma once

#ifdef __CST_GAME_BUILD__
#define CST_GAME_API __declspec(dllexport)
#else
#define CST_GAME_API __declspec(dllimport)
#endif // __CST_GAME_BUILD__


#ifndef __CST_GAME_SHIPPING__
#define CST_GAME_WITH_EDITOR	
#endif // !__CST_GAME_SHIPPING__



#include "nsGameApplication.h"
#include "nsConsole.h"
#include "nsActor.h"
#include "nsAssetTypes.h"



extern "C" CST_GAME_API nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept;


extern nsLogCategory cstGameLog;
extern nsLogCategory cstPlayerLog;
extern nsLogCategory cstCharacterLog;
extern nsLogCategory cstAbilityLog;



class cstCharacter;
class cstAbility;
class cstItem;
class cstConsumable;
class cstEquipment;
class cstWeapon;
class cstArmor;



namespace cstTag
{
	enum Flags : uint64
	{
		NONE					= (0),

		Character_Player		= (1 << 0),
		Character_Enemy			= (1 << 1),
		Character_Neutral		= (1 << 2),

		Physical				= (1 << 3),
		Magical					= (1 << 4),

		Status_Poison			= (1 << 5),
		Status_Burn				= (1 << 6),
		Status_Freeze			= (1 << 7),
		Status_Stun				= (1 << 8),
		Status_Slow				= (1 << 9),
		Status_Silence			= (1 << 10),
		Status_Hate				= (1 << 11),
		Status_Injured			= (1 << 12),
		Status_KO				= (1 << 13),

		Immune_Physical			= (1 << 14),
		Immune_Magical			= (1 << 15),

		Weapon_Dual_Dagger		= (1 << 16),
		Weapon_Sword_Shield		= (1 << 17),
		Weapon_Greatsword		= (1 << 18),
		Weapon_Staff			= (1 << 19),

		DISABLE_ACTION		= Status_Freeze | Status_Stun | Status_Hate | Status_KO
	};
};

typedef uint64 cstTags;



namespace cstAttribute
{
	enum EType : uint8
	{
		STR = 0,									// Strength (PATK)
		VIT,										// Vitality (MaxHealth, PDEF)
		INT,										// Intelligence (MaxMana, MATK)
		MEN,										// Mentality (MDEF, CSPD)
		DEX,										// Dexterity (ASPD)
		AGI,										// Agility (MSPD)
		CRT,										// Physical/Magical critical rate (%)
		PATK,										// Physical attack
		PDEF,										// Physical defense
		MATK,										// Magical attack
		MDEF,										// Magical defense
		ASPD,										// Attack speed
		CSPD,										// Casting speed
		MSPD,										// Movement speed
		MAX_HEALTH,									// Maximum health
		MAX_MANA,									// Maximum mana
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
		CURRENT_ABILITY_MANA_COST_REDUCTION,		// Current ability mana cost reduction (%)
		CURRENT_ABILITY_COOLDOWN_REDUCTION,			// Current ability cooldown reduction (%)
		CURRENT_STATUS_POISON,						// Current status meter: Poison 
		CURRENT_STATUS_BURN,						// Current status meter: Burn 
		CURRENT_STATUS_FREEZE,						// Current status meter: Freeze
		CURRENT_STATUS_STUN,						// Current status meter: Stun
		CURRENT_STATUS_SLOW,						// Current status meter: Slow
		CURRENT_STATUS_SILENCE,						// Current status meter: Silence
		CURRENT_STATUS_BLIND,						// Current status meter: Blind
		CURRENT_STATUS_HATE,						// Current status meter: Hate

		DAMAGE_HEALTH,								// Damage/recovery: Health
		DAMAGE_MANA,								// Damage/recovery: Mana
		DAMAGE_ELEMENT_FIRE,						// Damage/buff: Fire element contribution
		DAMAGE_ELEMENT_WATER,						// Damage/buff: Water element contribution
		DAMAGE_ELEMENT_WIND,						// Damage/buff: Wind element contribution
		DAMAGE_ELEMENT_EARTH,						// Damage/buff: Earth element contribution
		DAMAGE_ELEMENT_LIGHT,						// Damage/buff: Light element contribution
		DAMAGE_ELEMENT_DARK,						// Damage/buff: Dark element contribution
		DAMAGE_STATUS_POISON,						// Damage/buff: Poison status effect contribution
		DAMAGE_STATUS_BURN,							// Damage/buff: Burn status effect contribution
		DAMAGE_STATUS_FREEZE,						// Damage/buff: Freeze status effect contribution
		DAMAGE_STATUS_STUN,							// Damage/buff: Stun status effect contribution
		DAMAGE_STATUS_SLOW,							// Damage/buff: Slow status effect contribution
		DAMAGE_STATUS_SILENCE,						// Damage/buff: Silence status effect contribution
		DAMAGE_STATUS_BLIND,						// Damage/buff: Blind status effect contribution
		DAMAGE_STATUS_HATE,							// Damage/buff: Hate status effect contribution

		MAX_COUNT
	};
};


class cstAttributes
{
private:
	float Values[cstAttribute::MAX_COUNT];


public:
	cstAttributes()
		: Values()
	{
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
		attributes[cstAttribute::MAX_HEALTH]				= 100.0f;
		attributes[cstAttribute::MAX_MANA]					= 100.0f;
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
		attributes[cstAttribute::CURRENT_HEALTH]			= attributes[cstAttribute::MAX_HEALTH];
		attributes[cstAttribute::CURRENT_MANA]				= attributes[cstAttribute::MAX_MANA];

		return attributes;
	}

};
