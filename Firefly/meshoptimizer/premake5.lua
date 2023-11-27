project "MeshOptimizer"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")


	files
	{
		"src/**cpp",
        "src/**h",
	}
	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
   
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
	
	
	filter "configurations:ShipIt"
		runtime "Release"
		optimize "on"
		symbols "off"