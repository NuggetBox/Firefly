#pragma once
#include <Windows.h>
#include <bitset>
#include "Utils/Math/Vector2.hpp"

namespace Utils
{
	struct KeyboardState
	{
		std::bitset<256> virtualKeyPresses;
	};

	struct MouseState
	{
		POINT mousePos;

		bool leftClick;
		bool rightClick;
		bool middleClick;
		bool xButtonOne;
		bool xButtonTwo;
		short scrollWheelCount;
	};

	class InputHandler
	{
	public:
		InputHandler() = delete;
		~InputHandler() = delete;

		static void Initialize(HWND aHandle);

		static bool GetKeyDown(const int aKeyCode);
		static bool GetKeyHeld(const int aKeyCode);
		static bool GetKeyUp(const int aKeyCode);
		static UINT GetPressedKey();
		static void ResetPressedKey();

		static bool GetLeftClickDown();
		static bool GetLeftClickHeld();
		static bool GetLeftClickUp();

		static bool GetRightClickDown();
		static bool GetRightClickHeld();
		static bool GetRightClickUp();

		static bool GetMiddleClickDown();
		static bool GetMiddleClickHeld();
		static bool GetMiddleClickUp();
		/**
		 * \brief
		 * \param aXButton Upper x button has the value of 2 and lower has value of 1.
		 * \return
		 */
		static bool GetXButtonDown(const int aXButton);
		/**
		 * \brief
		 * \param aXButton Upper x button has the value of 2 and lower has value of 1.
		 * \return
		 */
		static bool GetXButtonHeld(const int aXButton);
		/**
		* \brief
		* \param aXButton Upper x button has the value of 2 and lower has value of 1.
		* \return
		*/
		static bool GetXButtonUp(const int aXButton);


		static POINT GetMousePosition();
		static POINT GetMouseDelta();
		static POINT GetUncappedMouseDelta();
		/// <summary>
		/// Set the relative mouse position to the viewport if it exists
		/// </summary>
		/// <param name="aX"></param>
		/// <param name="aY"></param>
		static void SetMouseRelativePos(float aX, float aY);
		/// <summary>
		/// get the relative X position to the viewport 
		/// </summary>
		/// <returns></returns>
		static float GetMouseRelativeXPos();
		/// <summary>
		/// get the relative X position to the viewport 
		/// </summary>
		/// <returns></returns>
		static float GetMouseRelativeYPos();

		static void SetWindowSize(Vector2f aMin);
		static Vector2f GetWindowSize();
		// Positive = Scroll away from user, Negative = Scroll towards user
		static int GetScrollWheelDelta();

		static void SetMousePosition(const HWND& aHandle, const int anX, const int anY);

		static void CaptureMouse();
		static void CaptureMouse(int aMinX, int aMinY, int aMaxX, int aMaxY);
		static void ReleaseMouse();
		static bool IsMouseCaptured();

		static bool IsWindowFocused();
		static bool WasFocusGained();
		static bool WasFocusLost();

		static void HideMouseCursor();
		static void ShowMouseCursor();
		static bool IsMouseHidden();

		static bool UpdateEvents(UINT message, WPARAM wParam, LPARAM lParam);
		static void UpdatePreviousState();
		static void ResetAllInput();

	private:
		static bool UpdateKeyboard(UINT message, WPARAM wParam, LPARAM lParam);
		static bool UpdateMouse(UINT message, WPARAM wParam, LPARAM lParam);

		static inline KeyboardState myKeyboardState;
		static inline KeyboardState myPreviousKeyboardState;
		static inline MouseState myMouseState;
		static inline MouseState myPreviousMouseState;
		static inline POINT myUncappedMouseDelta;
		static inline UINT myPressedKey;

		static inline float myMouseRelativeX;
		static inline float myMouseRelativeY;
		static inline Utils::Vector2f myMinWindowSize;

		static inline HWND myHandle;
		static inline bool myCursorHidden = false;
		static inline bool myCaptured = false;

		static inline bool myGainedFocus = false;
		static inline bool myLostFocus = false;
		static inline bool myHasFocus = true;

		static inline bool myMouseWasCapturedWhenLostFocus = false;
		static inline bool myMouseWasHiddenWhenLostFocus = false;
	};
}