project "MuninGraph"
	location "%{wks.location}/MuninGraph/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	targetname("%{prj.name}")
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	pchheader "MuninGraph.pch.h"
	pchsource "MuninGraph.pch.cpp"

	includedirs{
		"../ImGui/source/",
		"../ImGuiNodeEditor/source/",
		"../Vendor/include/nlohmann/",
		"../MuninGraph/"
	}

	files {
		"**.h",
		"**.cpp",
		"**.hpp",
		"**.inl"
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
		systemversion "latest"
		flags { 
			"MultiProcessorCompile"
		}