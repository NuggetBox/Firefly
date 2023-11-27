project "SoundEngine"
	location "%{wks.location}/SoundEngine/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
    debugdir "AssetData/"
	
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	
	pchheader ("SoundEngine.pch.h")
	pchsource ("source/SoundEngine.pch.cpp")
	
	files
	{
		"./**.h",
		"./**.hpp",
		"./**.cpp",
		"./**.impl"
	}
	
	includedirs
	{
		"source/",
		"include/",
		"../Vendor/include/FMOD/",
		"../Firefly/source/",
		"../Utils/source/"
	}
	
	libdirs 
	{ 
		"../Vendor/lib/FMOD/%{cfg.buildcfg}/",
	}


	links
	{

	}

		
	filter "configurations:Debug"
		defines {"FF_DEBUG"}
		runtime "Debug"
		symbols "on"
		links
		{
			"fmodL_vc.lib",
			"fmodstudioL_vc.lib"
		}
	filter "configurations:Release"
		defines "FF_RELEASE"
		runtime "Release"
		optimize "on"
		links
		{
			"fmod_vc.lib",
			"fmodstudio_vc.lib"
		}
  filter "configurations:ShipIt"
		defines "FF_SHIPIT"
		runtime "Release"
		optimize "on"
		links
		{
			"fmod_vc.lib",
			"fmodstudio_vc.lib"
		}
		
	filter "system:windows"
		staticruntime "off"
		symbols "On"		
		systemversion "latest"
		flags 
        { 
			"MultiProcessorCompile",

		}
		defines
		{
			"WIN32",
			"_LIB", 
			"_SOUNDENGINE_LIB",
			"NOMINMAX" 
		}
		