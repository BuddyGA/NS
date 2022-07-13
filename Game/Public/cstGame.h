#pragma once

#include "cstTypes.h"



class cstGame : public nsGameApplication
{
private:
	cstEGameState CurrentState;
	cstEGameState PendingChangeState;
	cstCharacter* Character;

	nsVector3 CameraMoveAxis;
	nsTransform CameraTransform;
	float CameraDistance;
	float CameraMoveSpeed;
	bool bDebugDrawBorders;


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
	void HandleGameState_Intro();
	void HandleGameState_MainMenu();
	void HandleGameState_Loading();
	void HandleGameState_Playing();
	void HandleGameState_Cutscene();


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
	void HandleGameState_Editing();

#endif // CST_GAME_WITH_EDITOR

};
