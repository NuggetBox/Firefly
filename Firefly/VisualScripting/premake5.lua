project "VisualScripting"
	location "%{wks.location}/VisualScripting/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	disablewarnings { "4996", "26812", "4267" }
    debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	--pchheader "VisualScriptingPch.h"
	--pchsource "source/VisualScriptingPch.cpp"

	files {
		"**.h",
		"**.cpp",
		"**.hpp",
		"**.inl"
	}

	includedirs {
		"source/",
		"../Vendor/include/",
		"../Utils/source/",
		"../ImGui/source/",
		"../Firefly/source/",
		"../Optick/source/",
		"../MuninGraph/",
		"../SoundEngine/source/",
		"../Vendor/include/FMOD/",
		"../ImGuiNodeEditor/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
		"../Game/source/",
		"../PhysX/include/",
		"../PhysX/include/PhysX",
	}
	
	libdirs 
	{ 	
		"../Vendor/lib/FMOD/%{cfg.buildcfg}/"
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
			"NDEBUG"
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