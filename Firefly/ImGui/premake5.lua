project "ImGui"
	location "%{wks.location}/ImGui/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	includedirs{
		"source/imgui/"
	}

	files {
		"**.h",
		"**.cpp",
		"**.hpp"
	}
	
	filter "configurations:Debug"
		defines {"FF_DEBUG"}
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "FF_RELEASE"
		runtime "Release"
		optimize "on"
    filter "configurations:ShipIt"
		defines "FF_SHIPIT"
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
			"_LIB"
		}