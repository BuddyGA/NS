#pragma once

#include "nsWindow.h"
#include "nsViewport.h"
#include "nsGUIFramework.h"


class nsRenderer;
class nsWorld;



class NS_ENGINE_API nsGameApplication : public nsWindow
{
protected:
	nsWorld* MainWorld;
	nsViewport MainViewport;
	nsRenderer* MainRenderer;
	nsGUIContext GUIContext;
	nsGUIConsoleWindow ConsoleWindow;

	nsGUIWindow TestWindow0;
	nsGUITable TestTable;
	/*
	nsGUIWindow TestWindow1;
	*/

public:
	bool bShowFPS;


public:
	nsGameApplication(const char* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept;
	virtual void Initialize() noexcept;
	virtual void Shutdown() noexcept;
	virtual void HandleConsoleCommand(const nsString& command, const nsString* params, int paramCount) noexcept;
	virtual void StartPlay() noexcept;
	virtual void TickUpdate(float deltaTime) noexcept;
	virtual void PostPhysicsUpdate() noexcept;
	virtual void PreRender() noexcept;
	void DrawGUI() noexcept;

protected:
	virtual void OnMouseMove(const nsMouseMoveEventArgs& e) noexcept override;
	virtual void OnMouseButton(const nsMouseButtonEventArgs& e) noexcept override;
	virtual void OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept override;
	virtual void OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept override;
	virtual void OnCharInput(char c) noexcept override;
	virtual void OnMinimized() noexcept override;
	virtual void OnMaximized() noexcept override;
	virtual void OnRestored() noexcept override;
	virtual void OnGUI(nsGUIContext& context) noexcept {}

private:
	void LoadTestLevel_Boxes();

};


typedef nsGameApplication* (*nsFCreateGameApplication)(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode);
