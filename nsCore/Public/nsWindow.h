#pragma once

#include "nsPlatform.h"



enum class nsEWindowSizeState : uint8
{
	DEFAULT = 0,
	MINIMIZED,
	MAXIMIZED
};


enum class nsEWindowFullscreenMode : uint8
{
	WINDOWED = 0,
	FULLSCREEN,
	WINDOWED_FULLSCREEN
};



class NS_CORE_API nsWindow
{
	NS_DECLARE_NOCOPY(nsWindow)

private:
	char Title[32];
	nsWindowHandle WindowHandle;
	nsEWindowSizeState WindowSizeState;
	nsEWindowFullscreenMode WindowFullscreenMode;
	nsPointInt PrevMousePosition;
	bool bIsMainWindow;
	bool bRelativeMouseMove;
	bool bShowCursor;


public:
	nsWindow(const char* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept;
	virtual ~nsWindow() noexcept;

	// Get window dimension
	NS_NODISCARD nsPointInt GetDimension() const noexcept;


	// Set to show/hide mouse cursor
	NS_INLINE void ShowMouseCursor(bool bShow) noexcept
	{
		if (bShowCursor != bShow)
		{
			bShowCursor = bShow;
			nsPlatform::Mouse_ShowCursor(bShowCursor);
		}
	}


	// Set mouse cursor position inside window
	NS_INLINE void SetMouseCursorPosition(const nsPointInt& position) noexcept
	{
		nsPlatform::Mouse_SetCursorWindowPosition(WindowHandle, position);
	}


	// Set enabled/disabled relative mouse mode 
	NS_INLINE void SetMouseRelativeMode(bool bEnabled) noexcept
	{
		bRelativeMouseMove = bEnabled;
	}


	// Set mouse cursor clip inside window
	NS_INLINE void ClipMouseCursor(bool bClip) noexcept
	{
		const nsPointInt dimension = GetDimension();
		nsPlatform::Mouse_ClipCursor(bClip, WindowHandle, nsRectInt(0, 0, dimension.X, dimension.Y));
	}


	// Get window title
	NS_NODISCARD_INLINE const char* GetTitle() const noexcept
	{
		return Title;
	}


	// Get window handle
	NS_NODISCARD_INLINE nsWindowHandle GetHandle() const noexcept
	{
		return WindowHandle;
	}


	// Is window minimized?
	NS_NODISCARD_INLINE bool IsMinimized() const noexcept
	{
		return WindowSizeState == nsEWindowSizeState::MINIMIZED;
	}


protected:
	virtual void OnMouseMove(const nsMouseMoveEventArgs& e) noexcept {}
	virtual void OnMouseButton(const nsMouseButtonEventArgs& e) noexcept {}
	virtual void OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept {}
	virtual void OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept {}
	virtual void OnCharInput(char c) noexcept {}
	virtual void OnGainFocus() noexcept {}
	virtual void OnLostFocus() noexcept {}
	virtual void OnShow() noexcept {}
	virtual void OnHide() noexcept {}
	virtual void OnMinimized() noexcept {}
	virtual void OnMaximized() noexcept {}
	virtual void OnRestored() noexcept {}
	virtual bool OnClose() noexcept { return true; }


#ifdef NS_PLATFORM_WINDOWS
public:
	LRESULT ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif // NS_PLATFORM_WINDOWS

};
