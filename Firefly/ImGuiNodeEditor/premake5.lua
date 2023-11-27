project "ImGuiNodeEditor"
	location "%{wks.location}/ImGuiNodeEditor/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	includedirs{
		"../ImGui/source/",
		"source/"
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