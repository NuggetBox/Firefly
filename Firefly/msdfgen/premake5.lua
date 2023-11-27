project "msdfgen"
	location "%{wks.location}/msdfgen/"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	disablewarnings { "4996", "26812", "4267" }
    debugdir "AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	targetname("%{prj.name}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	files
    {
		"**.h",
		"**.cpp",
		"**.hpp",
	}

	includedirs 
    {
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include",
		"../Vendor/include/freetype/",
	}

	libdirs { "../Vendor/lib/" }

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
		flags 
        { 
			"MultiProcessorCompile",

		}
		defines{
			"WIN32",
			"_LIB", 
			"NOMINMAX" 
		}