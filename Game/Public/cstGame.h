#pragma once

#include "cstInput.h"



enum class cstEGameState : uint8
{
	NONE = 0,

	INTRO,
	MAIN_MENU,
	LOADING,
	PLAYING,
	IN_GAME_MENU,
	PAUSE_MENU,

#ifdef CST_GAME_WITH_EDITOR
	EDITING,
#endif // NS_GAME_WITH_EDITOR
};



class cstGameplaySettings
{
public:
	float CameraDistance;
	float CameraMoveSpeed;


public:
	cstGameplaySettings();
	void Reset();
	void Save();
	void Load();

};


typedef nsTArrayInline<cstCharacter*, 3> cstPlayerCharacters;
typedef nsTArrayInline<cstAbility*, cstInputAction::ABILITY_SLOT_MAX_COUNT> cstPlayerAbilitySlots;
typedef nsTArrayInline<cstItem*, cstInputAction::ITEM_SLOT_MAX_COUNT> cstPlayerItemSlots;



class cstGame : public nsGameApplication
{
private:
	cstGameplaySettings GameplaySettings;
	cstInputSettings InputSettings;

	cstEGameState PendingChangeState;
	cstEGameState CurrentState;

	nsTransform CameraTransform;
	nsVector2 CameraMoveAxis;
	bool bCameraPanning;

	cstPlayerCharacters PlayerCharacters;
	cstPlayerAbilitySlots PlayerAbilitySlots[3];
	cstPlayerItemSlots PlayerItemSlots[3];

	nsPhysicsHitResult MouseRayHitResult;
	nsTArrayInline<cstCharacter*, 3> SelectedCharacters;
	int FocusedCharacterIndex;
	

public:
	cstGame(const wchar_t* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept;
	virtual void Initialize() noexcept override;
	virtual void Shutdown() noexcept override;
	virtual void TickUpdate(float deltaTime) noexcept;
	virtual void PreRender() noexcept override;

protected:
	virtual void OnMouseMove(const nsMouseMoveEventArgs& e) noexcept override;
	virtual void OnMouseButton(const nsMouseButtonEventArgs& e) noexcept override;
	virtual void OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept override;
	virtual void OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept override;
	virtual void OnGUI(nsGUIContext& context) noexcept override;

private:
	void OnInputActionBindingPressed(cstInputAction::EType inputActionType, bool bIsPressed, bool bIsDoubleClick);
	void OnInputActionBindingConflicted(cstInputAction::EType firstInputActionTpe, cstInputAction::EType secondInputActionType);

	void BeginState_Intro();
	void EndState_Intro();

	void BeginState_MainMenu();
	void EndState_MainMenu();

	void BeginState_Loading();
	void EndState_Loading();

	void BeginState_Playing();
	void EndState_Playing();

	void BeginState_InGameMenu();
	void EndState_InGameMenu();

	void BeginState_PauseMenu();
	void EndState_PauseMenu();


	NS_INLINE void ChangeState(cstEGameState newState)
	{
		if (CurrentState == newState)
		{
			return;
		}

		PendingChangeState = newState;
	}


#ifdef CST_GAME_WITH_EDITOR
private:
	bool bShowDebugFocusedCharacter;
	bool bDebugDrawBorders;

private:
	void BeginState_Editing();
	void EndState_Editing();
#endif // CST_GAME_WITH_EDITOR

};
