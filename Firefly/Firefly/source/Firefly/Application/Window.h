#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <functional>
#include <optional>

namespace Firefly
{
	using callback_function_wndProc = std::function<LRESULT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)>;

	enum class WindowMode
	{
		Fullscreen,
		Windowed,
		Borderless
	};

	struct WindowProperties
	{
		WindowProperties(const std::wstring& aTitle = L"DX11", uint32_t aWidth = 1280, uint32_t aHeight = 720, bool aEnableVSync = true, WindowMode aWindowMode = WindowMode::Windowed)
			: Title(aTitle), Width(aWidth), Height(aHeight), WindowMode(aWindowMode), X(0), Y(0)
		{}

		std::wstring Title;
		int X;
		int Y;
		int Width;
		int Height;
		WindowMode WindowMode;
	};

	class Window
	{
	public:
		Window() = default;
		Window(const WindowProperties& aPropeties);
		~Window();
		void SetTitle(const std::string& aTitle);
		static void ProcessMessages();
		//DX11& GetDX11();
		HWND GetHandle() const;
		int GetWidth() const;
		int GetHeight() const;
		int GetXPosition() const;
		int GetYPosition() const;

		void Resize(int aWidth, int aHeight);

		void SetFullScreen(const bool& aFullsceen);
		bool GetIsFullscreen() const;

		void SetWndFunctionToCall(callback_function_wndProc aFunction);
		void UnsetWindowsFunction();

		bool IsMinimized();

	private:
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	private:
		int myWindowPositionX;	 //only correct if never moved window
		int myWindowPositionY;	 //only correct if never moved window
		int myWidth;
		int myHeight;

		struct SavedWindowInfo
		{
			RECT Rect;

			LONG Style;
			LONG ExStyle;
			bool Maximized;
		} mySavedWindowInfo;

		bool myIsFullscreen;
		HWND myHWnd;
		callback_function_wndProc myWndFunctionToCall;
	};
}