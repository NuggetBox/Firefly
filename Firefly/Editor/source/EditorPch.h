#ifndef EDITOR_PCH
#define EDITOR_PCH

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include <WinUser.h>
#include <shellapi.h>
#include <WinBase.h> 
#include <shobjidl_core.h>
#include <filesystem>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <string>
#include <format>
#include <functional>
#include <random>
#include <istream>
#include <fstream>
#include <future>

#include "nlohmann/json.hpp"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "Firefly/Core/FireflyProfiling.h"

#endif // FIREFLY_PCH