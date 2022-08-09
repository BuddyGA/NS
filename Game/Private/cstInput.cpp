#include "cstInput.h"



cstInputManager::cstInputManager()
{
	ResetBindings();
	DoubleClickTimeSeconds = 0.25f;
}


void cstInputManager::DetectActionBindingConflict()
{
	for (int i = 0; i < cstInputAction::MAX_COUNT; ++i)
	{
		const cstInputBinding& firstBinding = ActionBindings[i];

		if (firstBinding.Key == nsEInputKey::NONE)
		{
			continue;
		}

		for (int j = i + 1; j < cstInputAction::MAX_COUNT; ++j)
		{
			const cstInputBinding& secondBinding = ActionBindings[j];

			if (secondBinding.Key == nsEInputKey::NONE)
			{
				continue;
			}

			if (firstBinding == secondBinding)
			{
				ActionBindingConflictedDelegate.Broadcast(static_cast<cstInputAction::EType>(i), static_cast<cstInputAction::EType>(j));
				return;
			}
		}
	}
}


void cstInputManager::KeyboardButtonEvent(const nsKeyboardButtonEventArgs& e)
{
	const bool bIsButtonPressed = (e.ButtonState == nsEButtonState::PRESSED);
	
	switch (e.Key)
	{
		case nsEInputKey::KEYBOARD_ALT_LEFT: ComparisonActionBinding.bAltLeft= bIsButtonPressed; break;
		case nsEInputKey::KEYBOARD_ALT_RIGHT: ComparisonActionBinding.bAltRight = bIsButtonPressed; break;
		case nsEInputKey::KEYBOARD_CTRL_LEFT: ComparisonActionBinding.bCtrlLeft = bIsButtonPressed; break;
		case nsEInputKey::KEYBOARD_CTRL_RIGHT: ComparisonActionBinding.bCtrlRight = bIsButtonPressed; break;
		case nsEInputKey::KEYBOARD_SHIFT_LEFT: ComparisonActionBinding.bShiftLeft = bIsButtonPressed; break;
		case nsEInputKey::KEYBOARD_SHIFT_RIGHT: ComparisonActionBinding.bShiftRight = bIsButtonPressed; break;
		default: break;
	}

	ComparisonActionBinding.Key = e.Key;
	const int index = ActionBindings.Find(ComparisonActionBinding);

	if (index != NS_ARRAY_INDEX_INVALID)
	{
		cstInputBinding& action = ActionBindings[index];
		const float currentTime = static_cast<float>(nsPlatform::PerformanceQuery_Counter()) / static_cast<float>(nsPlatform::PerformanceQuery_Frequency());
		const bool bIsDoubleClick = bIsButtonPressed && (currentTime - action.LastPressedTime) <= DoubleClickTimeSeconds;
		ActionBindingPressedDelegate.Broadcast(static_cast<cstInputAction::EType>(index), bIsButtonPressed, bIsDoubleClick);
		
		if (bIsButtonPressed)
		{
			action.LastPressedTime = currentTime;
		}
	}

	ComparisonActionBinding.Key = nsEInputKey::NONE;
}


void cstInputManager::ResetBindings()
{
	ComparisonActionBinding = cstInputBinding();

	ActionBindings.Resize(cstInputAction::MAX_COUNT);

	ActionBindings[cstInputAction::CAMERA_PANNING].Key = nsEInputKey::KEYBOARD_SPACEBAR;

	ActionBindings[cstInputAction::MENU_INVENTORY].Key = nsEInputKey::KEYBOARD_I;
	ActionBindings[cstInputAction::MENU_CHARACTER].Key = nsEInputKey::KEYBOARD_J;
	ActionBindings[cstInputAction::MENU_ABILITY].Key = nsEInputKey::KEYBOARD_K;
	ActionBindings[cstInputAction::MENU_PAUSE].Key = nsEInputKey::KEYBOARD_ESCAPE;

	ActionBindings[cstInputAction::CHARACTER_0].Key = nsEInputKey::KEYBOARD_1;
	ActionBindings[cstInputAction::CHARACTER_1].Key = nsEInputKey::KEYBOARD_2;
	ActionBindings[cstInputAction::CHARACTER_2].Key = nsEInputKey::KEYBOARD_3;
	ActionBindings[cstInputAction::CHARACTER_TOGGLE_FOCUS].Key = nsEInputKey::KEYBOARD_TAB;
	ActionBindings[cstInputAction::CHARACTER_SELECT_ALL].Key = nsEInputKey::KEYBOARD_A;
	ActionBindings[cstInputAction::CHARACTER_SELECT_ALL].bCtrlLeft = true;

	ActionBindings[cstInputAction::ABILITY_SLOT_ATTACK].Key = nsEInputKey::KEYBOARD_A;
	ActionBindings[cstInputAction::ABILITY_SLOT_STOP].Key = nsEInputKey::KEYBOARD_S;
	ActionBindings[cstInputAction::ABILITY_SLOT_0].Key = nsEInputKey::KEYBOARD_Q;
	ActionBindings[cstInputAction::ABILITY_SLOT_1].Key = nsEInputKey::KEYBOARD_W;
	ActionBindings[cstInputAction::ABILITY_SLOT_2].Key = nsEInputKey::KEYBOARD_E;
	ActionBindings[cstInputAction::ABILITY_SLOT_3].Key = nsEInputKey::KEYBOARD_R;
	ActionBindings[cstInputAction::ABILITY_SLOT_4].Key = nsEInputKey::KEYBOARD_T;
	ActionBindings[cstInputAction::ABILITY_SLOT_5].Key = nsEInputKey::KEYBOARD_D;
	ActionBindings[cstInputAction::ABILITY_SLOT_6].Key = nsEInputKey::KEYBOARD_F;
	ActionBindings[cstInputAction::ABILITY_SLOT_7].Key = nsEInputKey::KEYBOARD_G;

	ActionBindings[cstInputAction::ITEM_SLOT_0].Key = nsEInputKey::KEYBOARD_Q;
	ActionBindings[cstInputAction::ITEM_SLOT_0].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_1].Key = nsEInputKey::KEYBOARD_W;
	ActionBindings[cstInputAction::ITEM_SLOT_1].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_2].Key = nsEInputKey::KEYBOARD_E;
	ActionBindings[cstInputAction::ITEM_SLOT_2].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_3].Key = nsEInputKey::KEYBOARD_R;
	ActionBindings[cstInputAction::ITEM_SLOT_3].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_4].Key = nsEInputKey::KEYBOARD_A;
	ActionBindings[cstInputAction::ITEM_SLOT_4].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_5].Key = nsEInputKey::KEYBOARD_S;
	ActionBindings[cstInputAction::ITEM_SLOT_5].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_6].Key = nsEInputKey::KEYBOARD_D;
	ActionBindings[cstInputAction::ITEM_SLOT_6].bAltLeft = true;
	ActionBindings[cstInputAction::ITEM_SLOT_7].Key = nsEInputKey::KEYBOARD_F;
	ActionBindings[cstInputAction::ITEM_SLOT_7].bAltLeft = true;

	ActionBindings[cstInputAction::PRESETS_ABILITY_0].Key = nsEInputKey::KEYBOARD_F1;
	ActionBindings[cstInputAction::PRESETS_ABILITY_1].Key = nsEInputKey::KEYBOARD_F2;
	ActionBindings[cstInputAction::PRESETS_ITEM_0].Key = nsEInputKey::KEYBOARD_F3;
	ActionBindings[cstInputAction::PRESETS_ITEM_1].Key = nsEInputKey::KEYBOARD_F4;

	DetectActionBindingConflict();
}


void cstInputManager::SaveBindings()
{
}


void cstInputManager::LoadBindings()
{
}


void cstInputManager::AssignInputActionBinding(cstInputAction::EType inputActionType, cstInputBinding newBinding)
{

}
