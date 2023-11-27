project "PhysX"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	files
	{
		"source/**.cpp",
		"source/**.h",
		"source/**.hpp",
		"pxshared/**.h",
		"pxshared/**.cpp",

		"include/**.h",
		"include/**.cpp"
	}

	includedirs
	{
		"include/PhysX/",
		"source/common/include/",
		"source/common/src/",
		"source/physx/src/",
		"source/physx/src/device/",
		"source/physx/src/buffering/",
		"source/physxgpu/include/",
		"source/geomutils/include/",
		"source/geomutils/src/",
		"source/geomutils/src/contact/",
		"source/geomutils/src/common/",
		"source/geomutils/src/convex/",
		"source/geomutils/src/distance/",
		"source/geomutils/src/sweep/",
		"source/geomutils/src/gjk/",
		"source/geomutils/src/intersection/",
		"source/geomutils/src/mesh/",
		"source/geomutils/src/hf/",
		"source/geomutils/src/pcm/",
		"source/geomutils/src/ccd/",
		"source/lowlevel/api/include",
		"source/lowlevel/software/include",
		"source/lowlevel/common/include",
		"source/lowlevel/common/include/pipeline",
		"source/lowlevel/common/include/collision",
		"source/lowlevel/common/include/utils",
		"source/lowlevelaabb/include",
		"source/lowleveldynamics/include",
		"source/simulationcontroller/include",
		"source/simulationcontroller/src",
		"source/physxcooking/src",
		"source/physxcooking/src/mesh",
		"source/physxcooking/src/convex",
		"source/scenequery/include",
		"source/physxmetadata/core/include",
		"source/immediatemode/include",
		"source/pvd/include",
		"source/foundation/include",
		"source/fastxml/include",
		"source/physxextensions/src/serialization/File",
		"source/physxmetadata/extensions/include",
		"source/physxextensions/src/serialization/Xml",
		"source/physxextensions/src/serialization",
		"source/physxextensions/src",
		"source/physxextensions/src/serialization/Binary",
		"source/physxvehicle/src/physxmetadata/include",
		"source/filebuf/include",
		"source/physxvehicle/src",
		"pxshared/include",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
		"WIN32",
		"WIN64",
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_NONSTDC_NO_DEPRECATE",
		"_WINSOCK_DEPRECATED_NO_WARNINGS",
		"PX_COOKING",
		"DISABLE_CUDA_PHYSX"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++14"

	filter "configurations:Debug"
		defines 
		{ 
			"PX_DEBUG=1",
			"PX_CHECKED=1",
			"PX_NVTX=0",
			"PX_SUPPORT_PVD=1",
			"_DEBUG"
		}
		targetname "PhysXd"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"PX_PROFILE=1",
			"PX_NVTX=0",
			"PX_SUPPORT_PVD=1",
			"NDEBUG",
		}
		targetname "PhysX"
		runtime "Release"
		optimize "on"

	filter "configurations:ShipIt"
		defines
		{
			"PX_SUPPORT_PVD=0",
			"NDEBUG"
		}
		targetname "PhysXs"
		runtime "Release"
		optimize "on"