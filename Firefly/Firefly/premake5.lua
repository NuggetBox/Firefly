project "Firefly"
	location "%{wks.location}/Firefly/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	disablewarnings { "4996", "26812", "4267", "4251" }
    debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("FireflyEngine")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

    pchheader "FFpch.h"
	pchsource "source/FFpch.cpp"

	files
    {
		"**.h",
		"**.cpp",
		"**.hpp",
	}

	includedirs 
    {
		"source/",
		"../Vendor/include/",
		"../DiscordAPI/source/",
		"../Utils/source/",
		"../ImGui/source/",
		"../FBXImporter/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
		"../Optick/source/",
		"../SoundEngine/source/",
		"../Vendor/include/FMOD/",
		"../simdjson/source/",
		"../DDSTextureLoader/source/",
		"../SerializationUtils/source/",
		"../MuninGraph/",
		"../ImGuiNodeEditor/source/",
		"../VisualScripting/source/",
		"../MeshOptimizer/src/",
		"../PhysX/include/",
		"../PhysX/include/PhysX",
	}

	links{
		"DbgHelp.lib"
	}
	
	libdirs { 	
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
			"FBXSDK_SHARED"
		}


project "Shaders"
	location "%{wks.location}/Firefly/"
	kind "None"
	language "C++"
	flags("ExcludeFromBuild")
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")
	
	files {
		"../AssetData/FireflyEngine/Pipelines/**.hlsl",
		"../AssetData/FireflyEngine/Shaders/**.hlsl",
		"../AssetData/FireflyEngine/Shaders/**.hlsli",
	}
	
	includedirs {
		"../AssetData/FireflyEngine/Shaders/"
	}