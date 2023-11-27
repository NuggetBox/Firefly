#include <intsafe.h>
#include <unordered_map>

class InputMap
{
public:
	enum class Keybinds
	{
		Interact,
		Pause,
		Count
	};

	InputMap();
	static void Init();
	static unsigned int GetKeybind(Keybinds aKeybind);
	static std::unordered_map<Keybinds, unsigned int> GetAllKeyBinds();
	static bool IsKeybindDown(Keybinds aKeyBind);
	static bool IsKeybindHeld(Keybinds aKeyBind);
	static bool IsKeybindUp(Keybinds aKeyBind);
	static void SetKeybind(Keybinds aKeybind, unsigned int aValue);
	static void Save();
	static void Load();
	 
private:
	static inline std::unordered_map<Keybinds, unsigned int> myKeybinds;
};