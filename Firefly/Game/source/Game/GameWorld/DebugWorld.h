#pragma once

namespace Firefly
{
	class Font;

	class DebugWorld
	{
	public:
		void Start();
		void Update();
		void Exit();

	private:

		void ShowDebugFPS();

		Ref<Firefly::Font> myFPSFont;
	};
}