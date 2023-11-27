project "ImageConverter"
location "%{wks.location}/ImageConverter/"
kind "StaticLib"
language "C++"
cppdialect "C++20"

targetdir ("../bin/%{cfg.buildcfg}")
targetname("%{prj.name}")
objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

files 
{
	"**.h",
	"**.cpp",
    "**.inl",
    "**.inc"
}

includedirs
{
    "../Firefly/source/",
    "../Utils/source/"
}

links
{
	-- "d3d11.lib",
	-- "dxguid.lib"
}

filter "system:windows"
    systemversion "latest"
    kind "StaticLib"
    staticruntime "off"
    systemversion "latest"
    flags 
    { 
        "MultiProcessorCompile",
    }

filter "configurations:Debug"
    defines "_DEBUG"
    runtime "Debug"
    symbols "On"

filter "configurations:Release"
    defines "_RELEASE"
    runtime "Release"
    optimize "On"
	
filter "configurations:ShipIt"
    defines "_DISTRIBUTION"
    runtime "Release"
    optimize "On"
	symbols "Off"

