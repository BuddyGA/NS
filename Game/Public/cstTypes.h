#pragma once

#ifdef __CST_GAME_BUILD__
#define CST_GAME_API __declspec(dllexport)
#else
#define CST_GAME_API __declspec(dllimport)
#endif // __CST_GAME_BUILD__


#define CST_GAME_WITH_EDITOR			(1)


#include "nsGameApplication.h"
#include "nsConsole.h"
#include "nsActor.h"
#include "nsAssetTypes.h"



extern "C" CST_GAME_API nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept;


extern nsLogCategory cstGameLog;
extern nsLogCategory cstPlayerLog;
extern nsLogCategory cstCharacterLog;
extern nsLogCategory cstAbilityLog;



enum class cstEGameState : uint8
{
	NONE = 0,

	INTRO,
	MAIN_MENU,
	LOADING,
	PLAYING,
	IN_GAME_MENU,
	CUTSCENE,
	PAUSE_MENU,

#if CST_GAME_WITH_EDITOR
	EDITING,
#endif // NS_GAME_WITH_EDITOR
};



enum class cstEElementType : uint8
{
	NONE = 0,

	FIRE,
	WATER,
	WIND,
	EARTH,
	LIGHT,
	DARK
};



struct cstAttributes
{
	// Strength (PATK)
	float STR;

	// Vitality (MaxHealth, PDEF)
	float VIT;

	// Intelligence (MaxMana, MATK)
	float INT;

	// Mentality (MDEF, CSPD)
	float MEN;

	// Dexterity (ASPD)
	float DEX;

	// Agility (MSPD)
	float AGI;

	// Luck (Critical%)
	float LUK;

	// Maximum health
	float MaxHealth;

	// Maximum mana
	float MaxMana;

	// Physical attack 
	float PATK;

	// Physical defense
	float PDEF;

	// Magical attack
	float MATK;

	// Magical defense
	float MDEF;

	// Attack speed
	float ASPD;

	// Casting speed
	float CSPD;

	// Movement speed
	float MSPD;

	// Critical rate (%)
	float CritRate;

	// Fire element resistance
	float FireResistance;

	// Water element resistance
	float WaterResistance;

	// Wind element resistance
	float WindResistance;

	// Earth element resistance
	float EarthResistance;

	// Light element resistance
	float LightResistance;

	// Dark element resistance
	float DarkResistance;

	// Poison resistance
	float PoisonResistance;

	// Burn resistance
	float BurnResistance;

	// Stun resistance
	float StunResistance;

	// Slow resistance
	float SlowResistance;

	// Current health
	float Health;

	// Amount of health regenerate per second
	float HealthRegenPerSecond;

	// Current mana
	float Mana;

	// Amount of mana regenerate per second
	float ManaRegenPerSecond;

	// Ability mana cost reduction (%)
	float AbilityManaCostReduction;

	// Ability cooldown reduction (%)
	float AbilityCooldownReduction;


public:
	cstAttributes()
	{
		STR = 0.0f;
		VIT = 0.0f;
		INT = 0.0f;
		MEN = 0.0f;
		DEX = 0.0f;
		AGI = 0.0f;
		LUK = 0.0f;

		MaxHealth = 0.0f;
		MaxMana = 0.0f;
		PATK = 0.0f;
		PDEF = 0.0f;
		MATK = 0.0f;
		MDEF = 0.0f;
		ASPD = 0.0f;
		CSPD = 0.0f;
		MSPD = 0.0f;
		CritRate = 0.0f;

		FireResistance = 0.0f;
		WaterResistance = 0.0f;
		WindResistance = 0.0f;
		EarthResistance = 0.0f;
		LightResistance = 0.0f;
		DarkResistance = 0.0f;
		PoisonResistance = 0.0f;
		BurnResistance = 0.0f;
		StunResistance = 0.0f;
		SlowResistance = 0.0f;

		Health = 0.0f;
		HealthRegenPerSecond = 0.0f;
		Mana = 0.0f;
		ManaRegenPerSecond = 0.0f;
		AbilityManaCostReduction = 0.0f;
		AbilityCooldownReduction = 0.0f;
	}

};



namespace cstECharacterStatus
{
	enum Flag : uint8
	{
		None		= (0),
		KO			= (1 << 0),
		Poison		= (1 << 1),
		Silence		= (1 << 2),
		Stun		= (1 << 3),
	};
};

typedef uint8 cstCharacterStatusFlags;




class cstCharacter;
class cstPlayerCharacter;
class cstAbility;
class cstEquipment;
class cstWeapon;
class cstArmor;
