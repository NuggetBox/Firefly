project "Game"
	location "%{wks.location}/Game/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	disablewarnings { "4996", "26812", "4267" }
    debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

    pchheader "Gamepch.h"
	pchsource "source/Gamepch.cpp"

	links {
		"SoundEngine"
	}

	files {
		"**.h",
		"**.cpp",
		"**.hpp",
	}

	includedirs {
		"source/",
		"../Firefly/source/",
		"../Vendor/include/",
		"../Utils/source/",
		"../ImGui/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
		"../Vendor/include/FMOD/",
		"../SoundEngine/source/",
		"../Optick/source/",
		"../VisualScripting/source/",
		"../PhysX/include/",
		"../PhysX/include/PhysX",
		"../DiscordAPI/source/",
	}


	libdirs { 
		"../Vendor/lib/",
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
			"NDEBUG",
			"FF_INLAMNING"
		}
		runtime "Release"
		optimize "on"
		
	filter "system:windows"
		staticruntime "off"
		symbols "On"		
		systemversion "latest"
		flags { 
			"MultiProcessorCompile"
		}
		defines{
			"WIN32",
			"_LIB", 
			"NOMINMAX" 
		}