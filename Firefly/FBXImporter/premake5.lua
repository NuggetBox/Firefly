project "FBXImporter"
	location "%{wks.location}/FBXImporter/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	pchheader "TGAFbx.pch.h"
	pchsource "source/TGAFbx.pch.cpp"

	includedirs
	{
		"source/",
		"../Vendor/include/FBXSDK/"
	}
	
	links
    {
        
	}

	files {
		"**.h",
		"**.cpp",
		"**.hpp"
	}
	
	filter "configurations:Debug"
		defines {"_DEBUG"}
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines {"NDEBUG"}
		runtime "Release"
		optimize "on"
    filter "configurations:ShipIt"
		defines {"NDEBUG"}
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
			"FBXSDK_SHARED"
		}