#include "InputHandler.h"

#include <windowsx.h>

#include <iostream>

namespace Utils
{
	void InputHandler::Initialize(HWND aHandle)
	{
		myHandle = aHandle;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

		RAWINPUTDEVICE rid[1];
		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[0].dwFlags = RIDEV_INPUTSINK;
		rid[0].hwndTarget = myHandle;
		RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	}

	bool InputHandler::GetKeyDown(const int aKeyCode)
	{
		return myKeyboardState.virtualKeyPresses.test(aKeyCode) && !myPreviousKeyboardState.virtualKeyPresses.test(aKeyCode);
	}
	bool InputHandler::GetKeyHeld(const int aKeyCode)
	{
		return myKeyboardState.virtualKeyPresses.test(aKeyCode) && myPreviousKeyboardState.virtualKeyPresses.test(aKeyCode);
	}
	bool InputHandler::GetKeyUp(const int aKeyCode)
	{
		return !myKeyboardState.virtualKeyPresses.test(aKeyCode) && myPreviousKeyboardState.virtualKeyPresses.test(aKeyCode);
	}

	UINT InputHandler::GetPressedKey()
	{
		return myPressedKey;
	}

	void InputHandler::ResetPressedKey()
	{
		myPressedKey = 0;
	}

	bool InputHandler::GetLeftClickDown()
	{
		return myMouseState.leftClick && !myPreviousMouseState.leftClick;
	}
	bool InputHandler::GetLeftClickHeld()
	{
		return myMouseState.leftClick && myPreviousMouseState.leftClick;
	}
	bool InputHandler::GetLeftClickUp()
	{
		return !myMouseState.leftClick && myPreviousMouseState.leftClick;
	}

	bool InputHandler::GetRightClickDown()
	{
		return myMouseState.rightClick && !myPreviousMouseState.rightClick;
	}
	bool InputHandler::GetRightClickHeld()
	{
		return myMouseState.rightClick && myPreviousMouseState.rightClick;
	}
	bool InputHandler::GetRightClickUp()
	{
		return !myMouseState.rightClick && myPreviousMouseState.rightClick;
	}

	bool InputHandler::GetMiddleClickDown()
	{
		return myMouseState.middleClick && !myPreviousMouseState.middleClick;
	}
	bool InputHandler::GetMiddleClickHeld()
	{
		return myMouseState.middleClick && myPreviousMouseState.middleClick;
	}
	bool InputHandler::GetMiddleClickUp()
	{
		return !myMouseState.middleClick && myPreviousMouseState.middleClick;
	}

	bool InputHandler::GetXButtonDown(const int aXButton)
	{
		if (aXButton == 1)
			return myMouseState.xButtonOne && !myPreviousMouseState.xButtonOne;
		if (aXButton == 2)
			return myMouseState.xButtonTwo && !myPreviousMouseState.xButtonTwo;
		return false;
	}

	bool InputHandler::GetXButtonHeld(const int aXButton)
	{
		if (aXButton == 1)
			return myMouseState.xButtonOne && myPreviousMouseState.xButtonOne;
		if (aXButton == 2)
			return myMouseState.xButtonTwo && myPreviousMouseState.xButtonTwo;
		return false;
	}

	bool InputHandler::GetXButtonUp(const int aXButton)
	{
		if (aXButton == 1)
			return !myMouseState.xButtonOne && myPreviousMouseState.xButtonOne;
		if (aXButton == 2)
			return !myMouseState.xButtonTwo && myPreviousMouseState.xButtonTwo;
		return false;
	}

	POINT InputHandler::GetMousePosition()
	{
		return myMouseState.mousePos;
	}

	POINT InputHandler::GetMouseDelta()
	{
		POINT delta = myMouseState.mousePos;
		delta.x -= myPreviousMouseState.mousePos.x;
		delta.y -= myPreviousMouseState.mousePos.y;
		return delta;
	}

	POINT InputHandler::GetUncappedMouseDelta()
	{
		return myUncappedMouseDelta;
	}

	void InputHandler::SetMouseRelativePos(float aX, float aY)
	{
		myMouseRelativeX = aX;
		myMouseRelativeY = aY;
	}

	float InputHandler::GetMouseRelativeXPos()
	{
		return myMouseRelativeX;
	}

	float InputHandler::GetMouseRelativeYPos()
	{
		return myMouseRelativeY;
	}

	void InputHandler::SetWindowSize(Vector2f aMin)
	{
		myMinWindowSize = aMin;
	}

	Vector2f InputHandler::GetWindowSize()
	{
		return myMinWindowSize;
	}

	int InputHandler::GetScrollWheelDelta()
	{
		return myMouseState.scrollWheelCount;
	}

	void InputHandler::SetMousePosition(const HWND& aHandle, const int anX, const int anY)
	{
		POINT point = { anX, anY };
		ClientToScreen(aHandle, &point);
		SetCursorPos(point.x, point.y);

		myMouseState.mousePos.x = anX;
		myMouseState.mousePos.y = anY;
	}

	void InputHandler::CaptureMouse()
	{
		RECT clipRect;
		GetClientRect(myHandle, &clipRect);

		POINT upperLeft;
		upperLeft.x = clipRect.left;
		upperLeft.y = clipRect.top;

		POINT lowerRight;
		lowerRight.x = clipRect.right;
		lowerRight.y = clipRect.bottom;

		MapWindowPoints(myHandle, nullptr, &upperLeft, 1);
		MapWindowPoints(myHandle, nullptr, &lowerRight, 1);

		clipRect.left = upperLeft.x;
		clipRect.top = upperLeft.y;
		clipRect.right = lowerRight.x;
		clipRect.bottom = lowerRight.y;

		ClipCursor(&clipRect);
		myCaptured = true;
	}

	void InputHandler::CaptureMouse(int aMinX, int aMinY, int aMaxX, int aMaxY)
	{
		RECT clipRect;
		clipRect.left = aMinX;
		clipRect.top = aMinY;
		clipRect.right = aMaxX;
		clipRect.bottom = aMaxY;

		ClipCursor(&clipRect);
		myCaptured = true;
	}

	void InputHandler::ReleaseMouse()
	{
		ClipCursor(nullptr);
		myCaptured = false;
	}

	bool InputHandler::IsMouseCaptured()
	{
		return myCaptured;
	}

	bool InputHandler::IsWindowFocused()
	{
		return myHasFocus;
	}

	bool InputHandler::WasFocusGained()
	{
		return myGainedFocus;
	}

	bool InputHandler::WasFocusLost()
	{
		return myLostFocus;
	}

	void InputHandler::HideMouseCursor()
	{
		if (!myCursorHidden)
		{
			ShowCursor(false);
		}

		myCursorHidden = true;
	}

	void InputHandler::ShowMouseCursor()
	{
		if (myCursorHidden)
		{
			ShowCursor(true);
		}

		myCursorHidden = false;
	}

	bool InputHandler::IsMouseHidden()
	{
		return myCursorHidden;
	}

	bool InputHandler::UpdateEvents(UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Alt-tab -> reset input
		if (message == WM_KILLFOCUS)
		{
			myMouseWasCapturedWhenLostFocus = myCaptured;
			myMouseWasHiddenWhenLostFocus = myCursorHidden;
			ShowMouseCursor();
			ReleaseMouse();
			ResetAllInput();
			myLostFocus = true;
			myHasFocus = false;
			return true;
		}

		//Regain focus
		if (message == WM_SETFOCUS)
		{
			myGainedFocus = true;
			myHasFocus = true;

			if (myMouseWasCapturedWhenLostFocus)
			{
				CaptureMouse();
			}

			if (myMouseWasHiddenWhenLostFocus)
			{
				HideMouseCursor();
			}
			return true;
		}

		//Return if we gained focus this frame, we don't want frame 1 input
		if (myGainedFocus)
		{
			return false;
		}

		if (UpdateKeyboard(message, wParam, lParam))
		{
			return true;
		}

		if (UpdateMouse(message, wParam, lParam))
		{
			return true;
		}

		return false;
	}

	bool InputHandler::UpdateKeyboard(UINT message, WPARAM wParam, LPARAM lParam)
	{
		bool handled = false;

		switch (message)
		{
		case WM_KEYDOWN:
		{
			// If use WM_CHAR message for text input, skip A-Z, 0-9 here
			//myPreviousKeyboardState.virtualKeyPresses.set(wParam, myKeyboardState.virtualKeyPresses.test(wParam));
			//myPreviousKeyboardState.virtualKeyPresses.set(wParam, GetPreviousKeyState(lParam));
			myKeyboardState.virtualKeyPresses.set(wParam);
			myPressedKey = wParam;
			handled = true;
			break;
		}
		case WM_KEYUP:
		{
			//myPreviousKeyboardState.virtualKeyPresses.set(wParam, myKeyboardState.virtualKeyPresses.test(wParam));
			//myPreviousKeyboardState.virtualKeyPresses.set(wParam, GetPreviousKeyState(lParam));
			myKeyboardState.virtualKeyPresses.reset(wParam);
			handled = true;
			myPressedKey = 0;
			break;
		}
		case WM_SYSKEYUP:
		{
			//Lol, imagine
			if (wParam == 17)
			{
				myKeyboardState.virtualKeyPresses.reset(17);
				handled = true;
			}

			break;
		}
		default:
		{
			handled = false;
		}
		}

		return handled;
	}

	bool InputHandler::UpdateMouse(UINT message, WPARAM wParam, LPARAM lParam)
	{
		bool handled = true;

		switch (message)
		{
		case WM_LBUTTONDOWN:
		{
			myMouseState.leftClick = true;
			myPressedKey = 1;
			break;
		}
		case WM_LBUTTONUP:
		{
			myMouseState.leftClick = false;
			myPressedKey = 0;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			myMouseState.rightClick = true;
			myPressedKey = 2;
			break;
		}
		case WM_RBUTTONUP:
		{
			myMouseState.rightClick = false;
			myPressedKey = 0;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			myMouseState.middleClick = true;
			break;
		}
		case WM_MBUTTONUP:
		{
			myMouseState.middleClick = false;
			break;
		}
		case WM_XBUTTONDOWN:
		{
			auto p = GET_XBUTTON_WPARAM(wParam);
			if (p == XBUTTON1)
			{
				myPressedKey = 3;
				myMouseState.xButtonOne = true;
			}
			if (p == XBUTTON2)
			{
				myPressedKey = 4;
				myMouseState.xButtonTwo = true;
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			auto p = GET_XBUTTON_WPARAM(wParam);
			if (p == XBUTTON1)
				myMouseState.xButtonOne = false;
			if (p == XBUTTON2)
				myMouseState.xButtonTwo = false;

			myPressedKey = 0;
			break;
		}
		case WM_MOUSEMOVE:
		{
			myMouseState.mousePos.x = GET_X_LPARAM(lParam);
			myMouseState.mousePos.y = GET_Y_LPARAM(lParam);
			break;
		}
		case WM_INPUT:
		{
			constexpr UINT size = sizeof(RAWINPUT);
			static BYTE lpb[size];

			UINT dwSize = size;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				myUncappedMouseDelta.x += raw->data.mouse.lLastX;
				myUncappedMouseDelta.y += raw->data.mouse.lLastY;
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			myMouseState.scrollWheelCount += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			break;
		}
		default:
		{
			handled = false;
		}
		}

		return handled;
	}

	void InputHandler::UpdatePreviousState()
	{
		myPreviousKeyboardState = myKeyboardState;
		myPreviousMouseState = myMouseState;
		myUncappedMouseDelta = { 0, 0 };
		myGainedFocus = false;
		myLostFocus = false;

		myMouseState.scrollWheelCount = 0;
	}

	void InputHandler::ResetAllInput()
	{
		ZeroMemory(&myKeyboardState, sizeof(KeyboardState));
		ZeroMemory(&myPreviousKeyboardState, sizeof(KeyboardState));
		ZeroMemory(&myMouseState, sizeof(MouseState));
		ZeroMemory(&myPreviousMouseState, sizeof(MouseState));
		ZeroMemory(&myUncappedMouseDelta, sizeof(POINT));
		ZeroMemory(&myMouseRelativeX, sizeof(float));
		ZeroMemory(&myMouseRelativeY, sizeof(float));
		ZeroMemory(&myMinWindowSize, sizeof(Vector2f));
	}
}