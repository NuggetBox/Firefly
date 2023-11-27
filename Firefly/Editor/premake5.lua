project "Editor"
	location "%{wks.location}/Editor/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	disablewarnings { "4996", "26812", "4267" }
    debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	pchheader "EditorPch.h"
	pchsource "source/EditorPch.cpp"

	files {
		"**.h",
		"**.cpp",
		"**.hpp",
	}

	includedirs {
		"source/",
		"../Vendor/include/",
		"../Utils/source/",
		"../ImGui/source/",
		"../Firefly/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
		"../Optick/source/",
		"../simdjson/source",
		"../MuninGraph/",
		"../ImGuiNodeEditor/source/",
		"../VisualScripting/source/",
		"../SoundEngine/source/",
		"../Vendor/include/FMOD/",
		"../Client/source/",
		"../Server/source/",
		"../NetShared/source/",
		"../Game/source/",
		"../SerializationUtils/source/",
		"../simdjson/source/",
		"../ImageConverter/source/",
		"../Explorer/source/",
		"../PhysX/include/",
		"../PhysX/include/PhysX",
	}

	links {
        "ImGui",
		"ImGuiNodeEditor",
		"MuninGraph"
	}

	libdirs { 
			"../Vendor/lib/FMOD/%{cfg.buildcfg}/",
			"../Vendor/lib/"
	}

	filter "configurations:Debug"
		defines {
			"FF_DEBUG",
			"_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines {
			"FF_RELEASE",
			"NDEBUG"
		}
		runtime "Release"
		optimize "on"
	
    filter "configurations:ShipIt"
		defines {
			"FF_SHIPIT",
			"NDEBUG",
			"FF_INLAMNING"
		}
		runtime "Release"
		optimize "on"
		
	filter "system:windows"
		kind "StaticLib"
		staticruntime "off"
		symbols "On"		
		systemversion "latest"
		flags { 
			"MultiProcessorCompile",
		}
		defines{
			"WIN32",
			"_LIB", 
			"NOMINMAX",
		}