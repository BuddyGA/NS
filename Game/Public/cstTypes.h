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

		Weapon_MagicGloves		= (1 << 16),
		Weapon_SwordShield		= (1 << 17),
		Weapon_DualDagger		= (1 << 18),
		Weapon_Staff			= (1 << 19),
		Weapon_Greatsword		= (1 << 20),
		Weapon_Bow				= (1 << 21),

		CHARACTER_DISABLED		= Status_Freeze | Status_Stun | Status_Hate | Status_KO
	};
};

typedef uint64 cstTags;
