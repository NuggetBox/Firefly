project "SerializationUtils"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
    staticruntime "Off"
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	includedirs{
		"source/",
		"../Firefly/source/",
		"../Imgui/source/",
		"../Utils/source/",
		"../simdjson/source/",
		"../Vendor/include/",
		"../Explorer/source/",
		"../Optick/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
	}

	files{
		"source/**.h",
		"source/**.cpp"
	}

	defines{
		"NOMINMAX"
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
		defines{"NDEBUG"}
        runtime "Release"
        optimize "on"
        symbols "off"