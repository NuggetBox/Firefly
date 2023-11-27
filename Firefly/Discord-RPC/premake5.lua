project "Discord-RPC"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
    staticruntime "Off"
	warnings "Off"
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	includedirs{
		"include/"
	}

	files {
		"include/**.h",
		"include/**.cpp",
	}

	links {
	}

	linkoptions {
	}

	defines {
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines{"_DEBUG"}
		runtime "Debug"
        optimize "off"
		symbols "on"

	filter "configurations:Release"
		defines{"NDEBUG"}
		runtime "Release"
		optimize "on"
        symbols "on"

    filter "configurations:ShipIt"
        runtime "Release"
        optimize "on"
        symbols "off"
