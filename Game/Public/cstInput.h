#pragma once

#include "cstTypes.h"



namespace cstInputAction
{
	enum EType : uint8
	{
		CAMERA_PANNING = 0,			// Default: Spacebar

		MENU_INVENTORY,				// Default: I
		MENU_CHARACTER,				// Default: K
		MENU_ABILITY,				// Default: L
		MENU_PAUSE,					// Default: ESCAPE

		CHARACTER_0,				// Default: 1
		CHARACTER_1,				// Default: 2
		CHARACTER_2,				// Default: 3
		CHARACTER_TOGGLE_FOCUS,		// Default: TAB
		CHARACTER_SELECT_ALL,		// Default: LeftCtrl + A

		ABILITY_SLOT_STOP,			// Default: S
		ABILITY_SLOT_ATTACK,		// Default: A
		ABILITY_SLOT_0,				// Default: Q
		ABILITY_SLOT_1,				// Default: W
		ABILITY_SLOT_2,				// Default: E
		ABILITY_SLOT_3,				// Default: R
		ABILITY_SLOT_4,				// Default: T
		ABILITY_SLOT_5,				// Default: D
		ABILITY_SLOT_6,				// Default: F
		ABILITY_SLOT_7,				// Default: G

		ITEM_SLOT_0,				// Default: LeftAlt + Q
		ITEM_SLOT_1,				// Default: LeftAlt + W
		ITEM_SLOT_2,				// Default: LeftAlt + E
		ITEM_SLOT_3,				// Default: LeftAlt + R
		ITEM_SLOT_4,				// Default: LeftAlt + A
		ITEM_SLOT_5,				// Default: LeftAlt + S
		ITEM_SLOT_6,				// Default: LeftAlt + D
		ITEM_SLOT_7,				// Default: LeftAlt + F

		PRESETS_ABILITY_0,			// Default: F1
		PRESETS_ABILITY_1,			// Default: F2

		PRESETS_ITEM_0,				// Default: F3
		PRESETS_ITEM_1,				// Default: F4

		MAX_COUNT
	};

	constexpr const wchar_t* NAMES[MAX_COUNT] =
	{
		TEXT("CameraPanning"),
		TEXT("MenuInventory"),
		TEXT("MenuCharacter"),
		TEXT("MenuAbility"),
		TEXT("MenuPause"),
		TEXT("Character_0"),
		TEXT("Character_1"),
		TEXT("Character_2"),
		TEXT("CharacterToggleFocus"),
		TEXT("CharacterSelectAll"),
		TEXT("AbilitySlotStop"),
		TEXT("AbilitySlotAttack"),
		TEXT("AbilitySlot_0"),
		TEXT("AbilitySlot_1"),
		TEXT("AbilitySlot_2"),
		TEXT("AbilitySlot_3"),
		TEXT("AbilitySlot_4"),
		TEXT("AbilitySlot_5"),
		TEXT("AbilitySlot_6"),
		TEXT("AbilitySlot_7"),
		TEXT("ItemSlot_0"),
		TEXT("ItemSlot_1"),
		TEXT("ItemSlot_2"),
		TEXT("ItemSlot_3"),
		TEXT("ItemSlot_4"),
		TEXT("ItemSlot_5"),
		TEXT("ItemSlot_6"),
		TEXT("ItemSlot_7"),
		TEXT("PresetsAbility_0"),
		TEXT("PresetsAbility_1"),
		TEXT("PresetsItem_0"),
		TEXT("PresetsItem_1")
	};

	constexpr int MENU_MAX_COUNT = (MENU_PAUSE - MENU_INVENTORY + 1);
	constexpr int CHARACTER_MAX_COUNT = (CHARACTER_SELECT_ALL - CHARACTER_0 + 1);
	constexpr int ABILITY_SLOT_MAX_COUNT = (ABILITY_SLOT_7 - ABILITY_SLOT_STOP + 1);
	constexpr int ITEM_SLOT_MAX_COUNT = (ITEM_SLOT_7 - ITEM_SLOT_0 + 1);


	NS_NODISCARD_INLINE int GetAbilitySlotIndex(EType inputType)
	{
		NS_Assert(inputType >= cstInputAction::ABILITY_SLOT_STOP && inputType <= cstInputAction::ABILITY_SLOT_7);
		return cstInputAction::ABILITY_SLOT_MAX_COUNT - (cstInputAction::ABILITY_SLOT_7 - inputType);
	}


	NS_NODISCARD_INLINE int GetItemSlotIndex(EType inputType)
	{
		NS_Assert(inputType >= cstInputAction::ITEM_SLOT_0 && inputType <= cstInputAction::ITEM_SLOT_7);
		return cstInputAction::ITEM_SLOT_MAX_COUNT - (cstInputAction::ITEM_SLOT_7 - inputType);
	}

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


class cstInputManager
{
private:
	cstInputBinding CurrentInputAction;
	nsTArrayInline<cstInputBinding, cstInputAction::MAX_COUNT> ActionBindings;

public:
	float DoubleClickTimeSeconds;
	cstInputActionBindingPressedDelegate ActionBindingPressedDelegate;
	cstInputActionBindingConflictedDelegate ActionBindingConflictedDelegate;


public:
	cstInputManager();

private:
	void DetectActionBindingConflict();

public:
	void KeyboardButtonEvent(const nsKeyboardButtonEventArgs& e);
	void ResetBindings();
	void SaveBindings();
	void LoadBindings();
	void AssignInputActionBinding(cstInputAction::EType inputActionType, cstInputBinding newBinding);

};
