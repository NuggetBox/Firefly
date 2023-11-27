#include "FFpch.h"
#include "Window.h"
#include <filesystem>
#include <shellapi.h>
#include <Windows.h>
#include "Firefly/Rendering/GraphicsContext.h"
#include <WinUser.h>

namespace Firefly
{
	Window::Window(const WindowProperties& aProperties)
	{
		myWindowPositionX = aProperties.X;
		myWindowPositionY = aProperties.Y;
		myWidth = aProperties.Width;
		myHeight = aProperties.Height;

		mySavedWindowInfo.Style = 0;
		mySavedWindowInfo.ExStyle = 0;
		myIsFullscreen = false;

		const auto localPath = std::filesystem::current_path();

#ifndef FF_SHIPIT
		const wchar_t* AppName = L"Firefly";
		const auto pathToIcon = "\\Editor\\Icons\\icon_firefly.ico";
		DWORD WindowStyles = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
#else
		const wchar_t* AppName = L"Zenith";
		const auto pathToIcon = R"(\Assets\Textures\Icons\icon_game.ico)";
		DWORD WindowStyles = WS_POPUP | WS_VISIBLE;
#endif

		auto iconPath = localPath.string() + pathToIcon;
		HICON icon = (HICON)LoadImageA(nullptr,
			iconPath.c_str(),
			IMAGE_ICON, 
			0, 0, 
			LR_LOADFROMFILE | LR_DEFAULTSIZE);

		//Register window class
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = HandleMsgSetup;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = nullptr;
		windowClass.hIcon = icon;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = nullptr;
		windowClass.lpszClassName = AppName;
		windowClass.hIconSm = nullptr;
		RegisterClassEx(&windowClass);

		//create window and get hwnd
		myHWnd = CreateWindowEx(0,
			AppName,
			aProperties.Title.c_str(),
			WindowStyles,
			myWindowPositionX,
			myWindowPositionY,
			aProperties.Width,
			aProperties.Height,
			nullptr,
			nullptr,
			nullptr,
			this);

		Resize(aProperties.Width, aProperties.Height);
	}

	Window::~Window()
	{
		DestroyWindow(myHWnd);
	}

	void Window::SetTitle(const std::string& aTitle)
	{
		std::filesystem::path p = aTitle;
		if (SetWindowText(myHWnd, p.generic_wstring().c_str()) == 0)
		{
			//throw FWND_LAST_EXCEPT();
		}
	}

	void Window::ProcessMessages()
	{
		FF_PROFILEFUNCTION();

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//if (msg.message == WM_QUIT)
			//{
			//	return msg.wParam;
			//}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//return {};
	}

	HWND Window::GetHandle() const
	{
		return myHWnd;
	}

	int Window::GetWidth() const
	{
		return myWidth;
	}

	int Window::GetHeight() const
	{
		return myHeight;
	}

	void Window::Resize(int aWidth, int aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;

		RECT wr;
		if (GetWindowRect(myHWnd, &wr))
		{
			wr.right = myWidth + wr.left;
		}

		wr.bottom = myHeight + wr.top;

		//TODO: Most of our window issues lie here, we use the wrong flags and are resizing/moving it the wrong way
		AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

		SetWindowPos(myHWnd, 0, wr.left, wr.right, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE);
	}

	void Window::SetFullScreen(const bool& aFullsceen)
	{
		if (!GraphicsContext::IsCreated())
		{
			LOGERROR("Not allowed to set fullscreen before context is created!");
			FF_ASSERT(false, "Not allowed to set fullscreen before context is created!");
			return;
		}
		if (myIsFullscreen == aFullsceen) return;


		// Save current window state if not already fullscreen.
		if (!myIsFullscreen)
		{
			// Save current window information.  We force the window into restored mode
			// before going fullscreen because Windows doesn't seem to hide the
			// taskbar if the window is in the maximized state.
			mySavedWindowInfo.Maximized = !!::IsZoomed(myHWnd);
			if (mySavedWindowInfo.Maximized)
			{
				::SendMessage(myHWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			}
			mySavedWindowInfo.Style = GetWindowLong(myHWnd, GWL_STYLE);
			mySavedWindowInfo.ExStyle = GetWindowLong(myHWnd, GWL_EXSTYLE);
			GetWindowRect(myHWnd, &mySavedWindowInfo.Rect);
		}

		myIsFullscreen = aFullsceen;

		if (myIsFullscreen)
		{
			// Set new window style and size.
			SetWindowLong(myHWnd, GWL_STYLE,
				mySavedWindowInfo.Style & ~(WS_CAPTION | WS_THICKFRAME));
			SetWindowLong(myHWnd, GWL_EXSTYLE,
				mySavedWindowInfo.ExStyle & ~(WS_EX_DLGMODALFRAME |
					WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

			MONITORINFO monitor_info;
			monitor_info.cbSize = sizeof(monitor_info);
			GetMonitorInfo(MonitorFromWindow(myHWnd, MONITOR_DEFAULTTONEAREST),
				&monitor_info);
			myWidth = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
			myHeight = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
			//gfx::Rect window_rect(monitor_info.rcMonitor);
			SetWindowPos(myHWnd, NULL, monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
				myWidth, myHeight,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		}
		else
		{
			// Reset original window style and size.  The multiple window size/moves
			// here are ugly, but if SetWindowPos() doesn't redraw, the taskbar won't be
			// repainted.  Better-looking methods welcome.
			SetWindowLong(myHWnd, GWL_STYLE, mySavedWindowInfo.Style);
			SetWindowLong(myHWnd, GWL_EXSTYLE, mySavedWindowInfo.ExStyle);

			// On restore, resize to the previous saved rect size.
			SetWindowPos(myHWnd, NULL, mySavedWindowInfo.Rect.left, mySavedWindowInfo.Rect.top,
				mySavedWindowInfo.Rect.right - mySavedWindowInfo.Rect.left,
				mySavedWindowInfo.Rect.bottom - mySavedWindowInfo.Rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

			if (mySavedWindowInfo.Maximized)
			{
				::SendMessage(myHWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
			}
		}
	}

	bool Window::GetIsFullscreen() const
	{
		return myIsFullscreen;
	}

	LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_NCCREATE)
		{
			DragAcceptFiles(hWnd, true);
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);

	}

	LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (myWndFunctionToCall)
		{
			myWndFunctionToCall(hWnd, msg, wParam, lParam);
		}
		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_DESTROY:
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}

	void Window::SetWndFunctionToCall(callback_function_wndProc aFunction)
	{
		myWndFunctionToCall = aFunction;
	}

	void Window::UnsetWindowsFunction()
	{
		myWndFunctionToCall = nullptr;
	}

	bool Window::IsMinimized()
	{
		LONG lStyles = GetWindowLong(myHWnd, GWL_STYLE);

		if (lStyles & WS_MINIMIZE)
			return true;
		else
			return false;
	}

	int Window::GetXPosition() const
	{
		RECT rect;

		GetWindowRect(myHWnd, &rect);
		return rect.left;
	}

	int Window::GetYPosition() const
	{
		RECT rect;

		GetWindowRect(myHWnd, &rect);
		return rect.top;
	}
}