#include "nsWindow.h"
#include "nsContainer.h"
#include <windowsx.h>


static nsTArray<nsWindow*> Windows;


static LRESULT CALLBACK ns_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	nsWindow* window = reinterpret_cast<nsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (window == nullptr)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return window->ProcessMessage(uMsg, wParam, lParam);
}



nsWindow::nsWindow(const wchar_t* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
{
	NS_Assert(nsPlatform::String_Length(title) < 32);

	nsPlatform::String_Copy(Title, title);

	static const wchar_t* _windowClassName = TEXT("nsWindow");

	WNDCLASSEX windowClass{};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.lpszClassName = _windowClassName;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.lpfnWndProc = ns_WindowProc;
	windowClass.lpszMenuName = NULL;
	windowClass.style = CS_DBLCLKS;
	RegisterClassEx(&windowClass);

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	WindowHandle = CreateWindowEx(0, _windowClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (rect.right - rect.left), (rect.bottom - rect.top), NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!WindowHandle)
	{
		MessageBox(NULL, TEXT("Fail to create window!"), TEXT("Error"), MB_ICONERROR | MB_OK);
		NS_Abort();
	}

	SetWindowLongPtr(WindowHandle, GWLP_USERDATA, (LONG_PTR)this);
	ShowWindow(WindowHandle, SW_SHOW);
	UpdateWindow(WindowHandle);

	WindowSizeState = (fullscreenMode == nsEWindowFullscreenMode::WINDOWED) ? nsEWindowSizeState::DEFAULT : nsEWindowSizeState::MAXIMIZED;
	WindowFullscreenMode = fullscreenMode;
	
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	ScreenToClient(WindowHandle, &mousePosition);
	PrevMousePosition.X = mousePosition.x;
	PrevMousePosition.Y = mousePosition.y;
	bIsMainWindow = Windows.GetCount() == 0;
	bRelativeMouseMove = false;
	bShowCursor = true;

	Windows.Add(this);
}


nsWindow::~nsWindow() noexcept
{
	Windows.Remove(this);

	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
	}
}




nsPointInt nsWindow::GetDimension() const noexcept
{
	RECT rect;
	GetClientRect(WindowHandle, &rect);

	return nsPointInt(rect.right - rect.left, rect.bottom - rect.top);
}


static nsInputKey ns_MapToInputKey(WPARAM wParam, LPARAM lParam) noexcept
{
	const UINT scanCode = (lParam & 0x00FF0000) >> 16;
	const bool bExtended = (lParam & 0x01000000) != 0;
	WPARAM keyCode = wParam;

	if (wParam == VK_SHIFT)
	{
		keyCode = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
	}
	else if (wParam == VK_CONTROL)
	{
		keyCode = bExtended ? VK_RCONTROL : VK_LCONTROL;
	}
	else if (wParam == VK_MENU)
	{
		keyCode = bExtended ? VK_RMENU : VK_LMENU;
	}

	return static_cast<nsInputKey>(keyCode);
}


LRESULT nsWindow::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			if (OnClose())
			{
				if (bIsMainWindow)
				{
					PostQuitMessage(0);
				}
				else
				{
					NS_ValidateV(0, TEXT("Not implemented yet!"));
				}
			}

			return 0;
		}


		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				WindowSizeState = nsEWindowSizeState::MINIMIZED;
				OnMinimized();
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				WindowSizeState = nsEWindowSizeState::MAXIMIZED;
				OnMaximized();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (WindowSizeState != nsEWindowSizeState::DEFAULT)
				{
					WindowSizeState = nsEWindowSizeState::DEFAULT;
					OnRestored();
				}
			}

			return 0;
		}


		case WM_MOUSEMOVE:
		{
			nsMouseMoveEventArgs e{};
			e.Position.X = GET_X_LPARAM(lParam);
			e.Position.Y = GET_Y_LPARAM(lParam);

			if (bRelativeMouseMove)
			{
				e.DeltaPosition.X = e.Position.X - PrevMousePosition.X;
				e.DeltaPosition.Y = e.Position.Y - PrevMousePosition.Y;
				SetMouseCursorPosition(PrevMousePosition);
			}
			else if (PrevMousePosition.X != e.Position.X || PrevMousePosition.Y != e.Position.Y)
			{
				e.DeltaPosition.X = e.Position.X - PrevMousePosition.X;
				e.DeltaPosition.Y = e.Position.Y - PrevMousePosition.Y;
				PrevMousePosition = e.Position;
			}

			OnMouseMove(e);

			return 0;
		}


		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			nsMouseButtonEventArgs e{};
			e.Position.X = GET_X_LPARAM(lParam);
			e.Position.Y = GET_Y_LPARAM(lParam);

			if (uMsg == WM_LBUTTONDOWN)
			{
				e.Key = nsEInputKey::MOUSE_LEFT;
			}
			else if (uMsg == WM_MBUTTONDOWN)
			{
				e.Key = nsEInputKey::MOUSE_MIDDLE;
			}
			else
			{
				e.Key = nsEInputKey::MOUSE_RIGHT;
			}

			e.ButtonState = nsEButtonState::PRESSED;

			SetCapture(WindowHandle);
			OnMouseButton(e);

			return 0;
		}


		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			nsMouseButtonEventArgs e{};
			e.Position.X = GET_X_LPARAM(lParam);
			e.Position.Y = GET_Y_LPARAM(lParam);

			if (uMsg == WM_LBUTTONUP)
			{
				e.Key = nsEInputKey::MOUSE_LEFT;
			}
			else if (uMsg == WM_MBUTTONUP)
			{
				e.Key = nsEInputKey::MOUSE_MIDDLE;
			}
			else
			{
				e.Key = nsEInputKey::MOUSE_RIGHT;
			}

			e.ButtonState = nsEButtonState::RELEASED;

			ReleaseCapture();
			OnMouseButton(e);

			return 0;
		}


		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		{
			nsMouseButtonEventArgs e{};
			e.Position.X = GET_X_LPARAM(lParam);
			e.Position.Y = GET_Y_LPARAM(lParam);

			if (uMsg == WM_LBUTTONDBLCLK)
			{
				e.Key = nsEInputKey::MOUSE_LEFT;
			}
			else if (uMsg == WM_MBUTTONDBLCLK)
			{
				e.Key = nsEInputKey::MOUSE_MIDDLE;
			}
			else
			{
				e.Key = nsEInputKey::MOUSE_RIGHT;
			}

			e.ButtonState = nsEButtonState::REPEAT;
			OnMouseButton(e);

			return 0;
		}


		case WM_MOUSEWHEEL:
		{
			const int delta = GET_WHEEL_DELTA_WPARAM(wParam) / 120;

			nsMouseWheelEventArgs e{};
			e.Position.X = GET_X_LPARAM(lParam);
			e.Position.Y = GET_Y_LPARAM(lParam);
			e.ScrollValue = nsPointInt(0, delta);
			OnMouseWheel(e);

			return 0;
		}


		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			nsKeyboardButtonEventArgs e{};
			e.Key = ns_MapToInputKey(wParam, lParam);
			e.ButtonState = nsEButtonState::PRESSED;
			OnKeyboardButton(e);

			return 0;
		}


		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			nsKeyboardButtonEventArgs e{};
			e.Key = ns_MapToInputKey(wParam, lParam);
			e.ButtonState = nsEButtonState::RELEASED;
			OnKeyboardButton(e);

			return 0;
		}
		
		case WM_SYSCHAR:
		{
			//NS_LogDebug(nsSystemLog, "WM_SYSCHAR: %u", wParam);

			return 0;
		}

		case WM_CHAR:
		{
			OnCharInput(static_cast<char>(wParam));

			return 0;
		}

		default:
			break;
	}

	return DefWindowProc(WindowHandle, uMsg, wParam, lParam);
}
