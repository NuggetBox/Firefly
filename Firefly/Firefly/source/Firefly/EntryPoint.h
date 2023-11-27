#pragma once
#include "Firefly/Application/Application.h"
#include "Firefly/Application/ExceptionHandler.h"

extern Firefly::Application* Firefly::CreateApplication();

////ConsoleApp configuration
//int main(int argc, char** argv)
//{
//    auto app = Firefly::CreateApplication();
//    app->Fly();
//    delete app;
//    return 0;
//}

void Fly();

//WindowedApp configuration
auto APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)->int
{
#ifndef FF_INLAMNING
	AllocConsole();
	FILE* newstdin = nullptr;
	FILE* newstdout = nullptr;
	FILE* newstderr = nullptr;

	freopen_s(&newstdin, "conin$", "r", stdin);
	freopen_s(&newstdout, "conout$", "w", stdout);
	freopen_s(&newstderr, "conout$", "w", stderr);
#endif

#ifndef FF_INLAMNING
	__try
	{
		Fly();
	}
	__except (ExceptionHandler::HandleException(GetExceptionInformation()))
	{
	}
#else
	Fly();
#endif
	return 0;
}

inline void Fly()
{
	const auto app = Firefly::CreateApplication();
	app->Fly();
	delete app;
}