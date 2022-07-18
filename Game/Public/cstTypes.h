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



class cstCharacter;
class cstPlayerCharacter;
class cstEffect;
class cstAbility;
class cstItem;
class cstConsumable;
class cstEquipment;
class cstWeapon;
class cstArmor;






namespace cstExecute
{
	enum EActionType : uint8
	{
		// Only click the shortcut to execute
		ACTION_CLICK_SHORTCUT_ONLY = 0,

		// Click shortcut and click on target to execute
		ACTION_CLICK_SHORTCUT_AND_CLICK_TARGET,

		// Click shortcut and click on ground to execute
		ACTION_CLICK_SHORTCUT_AND_CLICK_GROUND,

		// Click shortcut, drag mouse on ground, and release to execute
		ACTION_CLICK_SHORTCUT_AND_DRAG_RELEASE
	};


	enum EResult : uint8
	{
		RESULT_SUCCESS = 0,
		RESULT_EXECUTING,
		RESULT_COOLDOWN,
		RESULT_WEAPON_REQUIRED,
		RESULT_NOT_ENOUGH_MANA,
		RESULT_EXECUTOR_KOed,
		RESULT_EXECUTOR_SILENCED,
		RESULT_EXECUTOR_STUNNED,
		RESULT_INVALID_EXECUTOR,
		RESULT_INVALID_TARGET_ANY,
		RESULT_INVALID_TARGET_SELF,
		RESULT_INVALID_TARGET_ALLY,
		RESULT_INVALID_TARGET_ENEMY,
	};



	struct TargetParams
	{
		cstCharacter* TargetCharacter;
		nsVector3 TargetGroundLocation;
		nsVector3 DragStartLocation;
		nsVector3 DragEndLocation;


	public:
		TargetParams()
			: TargetCharacter(nullptr)
		{
		}

	};


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



namespace cstEStatusEffect
{
	enum Flag : uint16
	{
		Normal		= (0),
		Poison		= (1UL << 0),
		Burn		= (1UL << 1),
		Freeze		= (1UL << 2),
		Stun		= (1UL << 3),
		Slow		= (1UL << 4),
		Silence		= (1UL << 5),
		Hate		= (1UL << 6),
		Injured		= (1UL << 7),
		KO			= (1UL << 8),
	};
};

typedef uint16 cstStatusEffects;




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
	float CRIT;

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

	// Freeze resistance
	float FreezeResistance;

	// Stun resistance
	float StunResistance;

	// Slow resistance
	float SlowResistance;

	// Silence resistance
	float SilenceResistance;

	// Hate resistance
	float HateResistance;

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

	// Damage/recovery: Health
	float DamageHealth;

	// Damage/recovery: Mana
	float DamageMana;

	// Damage/buff: Fire element contribution
	float ElementFire;

	// Damage/buff: Water element contribution
	float ElementWater;

	// Damage/buff: Wind element contribution
	float ElementWind;

	// Damage/buff: Earth element contribution
	float ElementEarth;

	// Damage/buff: Light element contribution
	float ElementLight;

	// Damage/buff: Dark element contribution
	float ElementDark;

	// Damage/buff: Poison status effect contribution
	float StatusPoison;

	// Damage/buff: Burn status effect contribution
	float StatusBurn;

	// Damage/buff: Stun status effect contribution
	float StatusStun;

	// Damage/buff: Slow status effect contribution
	float StatusSlow;

	// Damage/buff: Silence status effect contribution
	float StatusSilence;

	// Damage/buff: Hate status effect contribution
	float StatusHate;

	// Is physical type
	bool bIsPhysical;

	// Is magical type
	bool bIsMagical;


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
		CRIT = 0.0f;

		FireResistance = 0.0f;
		WaterResistance = 0.0f;
		WindResistance = 0.0f;
		EarthResistance = 0.0f;
		LightResistance = 0.0f;
		DarkResistance = 0.0f;
		PoisonResistance = 0.0f;
		BurnResistance = 0.0f;
		FreezeResistance = 0.0f;
		StunResistance = 0.0f;
		SlowResistance = 0.0f;
		SilenceResistance = 0.0f;
		HateResistance = 0.0f;

		Health = 0.0f;
		HealthRegenPerSecond = 0.0f;
		Mana = 0.0f;
		ManaRegenPerSecond = 0.0f;
		AbilityManaCostReduction = 0.0f;
		AbilityCooldownReduction = 0.0f;
		DamageHealth = 0.0f;
		DamageMana = 0.0f;
		ElementFire = 0.0f;
		ElementWater = 0.0f;
		ElementWind = 0.0f;
		ElementEarth = 0.0f;
		ElementLight = 0.0f;
		ElementDark = 0.0f;
		StatusPoison = 0.0f;
		StatusBurn = 0.0f;
		StatusStun = 0.0f;
		StatusSlow = 0.0f;
		StatusSilence = 0.0f;
		StatusHate = 0.0f;
		bIsPhysical = false;
		bIsMagical = false;
	}


	static NS_INLINE cstAttributes GetCharacterBaseAttributes()
	{
		cstAttributes attributes;

		attributes.STR = 5.0f;
		attributes.VIT = 5.0f;
		attributes.INT = 5.0f;
		attributes.MEN = 5.0f;
		attributes.DEX = 5.0f;
		attributes.AGI = 5.0f;
		attributes.LUK = 5.0f;
		attributes.MaxHealth = 100.0f;
		attributes.MaxMana = 100.0f;
		attributes.PATK = 10.0f;
		attributes.PDEF = 5.0f;
		attributes.MATK = 10.0f;
		attributes.MDEF = 5.0f;
		attributes.ASPD = 100.0f;
		attributes.CSPD = 100.0f;
		attributes.MSPD = 300.0f;
		attributes.CRIT = 5.0f;
		attributes.FireResistance = 0.0f;
		attributes.WaterResistance = 0.0f;
		attributes.WindResistance = 0.0f;
		attributes.EarthResistance = 0.0f;
		attributes.LightResistance = 0.0f;
		attributes.DarkResistance = 0.0f;
		attributes.PoisonResistance = 50.0f;
		attributes.BurnResistance = 50.0f;
		attributes.FreezeResistance = 50.0f;
		attributes.StunResistance = 50.0f;
		attributes.SlowResistance = 50.0f;
		attributes.SilenceResistance = 50.0f;
		attributes.Health = attributes.MaxHealth;
		attributes.HealthRegenPerSecond = 0.0f;
		attributes.Mana = attributes.MaxMana;
		attributes.ManaRegenPerSecond = 1.0f;

		return attributes;
	}

};
