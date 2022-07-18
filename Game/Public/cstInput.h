#pragma once

#include "cstTypes.h"



namespace cstInputAction
{
	enum EType : uint8
	{
		CAMERA_PANNING = 0,

		MENU_INVENTORY,
		MENU_CHARACTER,
		MENU_ABILITY,
		MENU_PAUSE,

		CHARACTER_0,
		CHARACTER_1,
		CHARACTER_2,
		CHARACTER_TOGGLE_FOCUS,
		CHARACTER_SELECT_ALL,

		ABILITY_SLOT_ATTACK,
		ABILITY_SLOT_STOP,
		ABILITY_SLOT_0,
		ABILITY_SLOT_1,
		ABILITY_SLOT_2,
		ABILITY_SLOT_3,
		ABILITY_SLOT_4,
		ABILITY_SLOT_5,
		ABILITY_SLOT_6,
		ABILITY_SLOT_7,

		ITEM_SLOT_0,
		ITEM_SLOT_1,
		ITEM_SLOT_2,
		ITEM_SLOT_3,
		ITEM_SLOT_4,
		ITEM_SLOT_5,
		ITEM_SLOT_6,
		ITEM_SLOT_7,

		PRESETS_ABILITY_0,
		PRESETS_ABILITY_1,

		PRESETS_ITEM_0,
		PRESETS_ITEM_1,

		MAX_COUNT
	};

	constexpr const wchar_t* NAMES[MAX_COUNT] =
	{
		TEXT("Camera_Panning"),
		TEXT("Menu_Inventory"),
		TEXT("Menu_Character"),
		TEXT("Menu_Ability"),
		TEXT("Menu_Pause"),
		TEXT("Character_0"),
		TEXT("Character_1"),
		TEXT("Character_2"),
		TEXT("Character_Toggle_Focus"),
		TEXT("Character_Select_All"),
		TEXT("Ability_Attack"),
		TEXT("Ability_Stop"),
		TEXT("Ability_Slot_0"),
		TEXT("Ability_Slot_1"),
		TEXT("Ability_Slot_2"),
		TEXT("Ability_Slot_3"),
		TEXT("Ability_Slot_4"),
		TEXT("Ability_Slot_5"),
		TEXT("Ability_Slot_6"),
		TEXT("Ability_Slot_7"),
		TEXT("Item_Slot_0"),
		TEXT("Item_Slot_1"),
		TEXT("Item_Slot_2"),
		TEXT("Item_Slot_3"),
		TEXT("Item_Slot_4"),
		TEXT("Item_Slot_5"),
		TEXT("Item_Slot_6"),
		TEXT("Item_Slot_7"),
		TEXT("Presets_Ability_0"),
		TEXT("Presets_Ability_1"),
		TEXT("Presets_Item_0"),
		TEXT("Presets_Item_1")
	};

	constexpr int MENU_MAX_COUNT = (MENU_PAUSE - MENU_INVENTORY + 1);
	constexpr int CHARACTER_MAX_COUNT = (CHARACTER_SELECT_ALL - CHARACTER_0 + 1);
	constexpr int ABILITY_SLOT_MAX_COUNT = (ABILITY_SLOT_7 - ABILITY_SLOT_ATTACK + 1);
	constexpr int ITEM_SLOT_MAX_COUNT = (ITEM_SLOT_7 - ITEM_SLOT_0 + 1);

};



struct cstInputBinding
{
	nsInputKey Key;
	float LastPressedTime;
	bool bAltLeft;
	bool bAltRight;
	bool bCtrlLeft;
	bool bCtrlRight;
	bool bShiftLeft;
	bool bShiftRight;


public:
	cstInputBinding()
	{
		Key = nsEInputKey::NONE;
		LastPressedTime = 0.0f;
		bAltLeft = false;
		bAltRight = false;
		bCtrlLeft = false;
		bCtrlRight = false;
		bShiftLeft = false;
		bShiftRight = false;
	}


	NS_INLINE bool operator==(const cstInputBinding& rhs) const
	{
		return Key == rhs.Key &&
			bAltLeft == rhs.bAltLeft && bAltRight == rhs.bAltRight &&
			bCtrlLeft == rhs.bCtrlLeft && bCtrlRight == rhs.bCtrlRight &&
			bShiftLeft == rhs.bShiftLeft && bShiftRight == rhs.bShiftRight;
	}

};



NS_DELEGATE_ThreeParams(cstInputActionBindingPressedDelegate, cstInputAction::EType, inputActionType, bool, bIsPressed, bool, bIsDoubleClick)
NS_DELEGATE_TwoParams(cstInputActionBindingConflictedDelegate, cstInputAction::EType, firstInputActionType, cstInputAction::EType, secondInputActionType)


class cstInputSettings
{
private:
	cstInputBinding ComparisonActionBinding;
	nsTArrayInline<cstInputBinding, cstInputAction::MAX_COUNT> ActionBindings;

public:
	float DoubleClickTimeSeconds;
	cstInputActionBindingPressedDelegate ActionBindingPressedDelegate;
	cstInputActionBindingConflictedDelegate ActionBindingConflictedDelegate;


public:
	cstInputSettings();

private:
	void DetectActionBindingConflict();

public:
	void KeyboardButtonEvent(const nsKeyboardButtonEventArgs& e);
	void Reset();
	void Save();
	void Load();
	void AssignInputActionBinding(cstInputAction::EType inputActionType, cstInputBinding newBinding);

};
