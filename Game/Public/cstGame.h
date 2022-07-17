#pragma once

#include "cstTypes.h"



class cstGame : public nsGameApplication
{
private:
	cstEGameState PendingChangeState;
	cstEGameState CurrentState;

	nsVector3 CameraMoveAxis;
	nsTransform CameraTransform;
	float CameraDistance;
	float CameraMoveSpeed;
	bool bDebugDrawBorders;

	cstPlayerCharacter* PlayerCharacter;
	cstAbility* AbilityDummy;


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
	void BeginGameState_Intro();
	void EndGameState_Intro();

	void BeginGameState_MainMenu();
	void EndGameState_MainMenu();

	void BeginGameState_Loading();
	void EndGameState_Loading();

	void BeginGameState_Playing();
	void EndGameState_Playing();

	void BeginGameState_InGameMenu();
	void EndGameState_InGameMenu();

	void BeginGameState_Cutscene();
	void EndGameState_Cutscene();

	void BeginGameState_PauseMenu();
	void EndGameState_PauseMenu();


public:
	NS_INLINE void ChangeState(cstEGameState newState)
	{
		if (CurrentState == newState)
		{
			return;
		}

		PendingChangeState = newState;
	}


#if CST_GAME_WITH_EDITOR
private:
	void BeginGameState_Editing();
	void EndGameState_Editing();
#endif // CST_GAME_WITH_EDITOR

};
