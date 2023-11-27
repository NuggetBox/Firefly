project "Launcher"
	location "%{wks.location}/Launcher/"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
    debugdir "../AssetData/"
	targetdir ("../bin/%{cfg.buildcfg}")
	objdir ("../bin-int/%{prj.name}/%{cfg.buildcfg}")

	files {
		"**.h",
		"**.cpp",
	}

	includedirs {
		"source/",
		"../Discord-RPC/include/",
		"../DiscordAPI/source/",
		"../Firefly/source/",
		"../Utils/source/",
		"../ImGui/source/",
		"../Game/source/",
		"../msdfgen/",
		"../msdfgen/msdfgen/",
		"../msdfgen/msdfgen/include/",
		"../Optick/source/",
		"../Vendor/include/",
		"../Vendor/include/FMOD/",
		"../SoundEngine/source/",
		"../PerforceWrapper/source/",
		"../ImGuiNodeEditor/source/",
		"../MuninGraph/",
		"../PhysX/include/"
	}

	links {
		"../Vendor/lib/freetype.lib",
		"d3d11.lib",
		"d3dcompiler.lib",
		"GFSDK_SSAO_D3D11.win64.lib",
		"../Vendor/lib/discord_game_sdk.dll.lib",
		"FBXSDK/libfbxsdk.lib",
        "Firefly",
		"Utils",
		"Game",
	
		"msdfgen", 
		"Optick",
		"ImGui",
		"ImGuiNodeEditor",
		"MuninGraph",
		"VisualScripting",
		"simdjson",
		"SerializationUtils",
		"ImGui",
		"FBXImporter",
		"DDSTextureLoader",
		"SoundEngine",
		"PerforceWrapper",
		"Netapi32.lib",
		"meshoptimizer",
		"Discord-RPC",
		"DiscordAPI",
		"PhysX",
	}

	linkoptions {
		"/WHOLEARCHIVE:FireflyEngine",
		"/WHOLEARCHIVE:Game",
		"/WHOLEARCHIVE:VisualScripting",
		"/WHOLEARCHIVE:SerializationUtils"
	}

	libdirs { 
		"../Vendor/lib/",
		"../Vendor/lib/FMOD/%{cfg.buildcfg}/",
		"../bin/%{cfg.buildcfg}/"
	}

	defines{
		"WIN32",
		"_LIB", 
		"NOMINMAX" 
	}

	systemversion "latest"

	filter "configurations:Debug"
		targetname("Firefly_Debug")
		defines {
			"FF_DEBUG",
			"_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		includedirs {
			"../Editor/source/",
		}
		links {
			"TGAFBXImporterd.lib",
			"fmodL_vc.lib",
			"fmodstudioL_vc.lib",
			"dppD.lib",
			"efswd.lib",
			"ImageConverter",
			"Editor",
		}
		linkoptions {
			"/WHOLEARCHIVE:Editor",
			"/WHOLEARCHIVE:PhysXd.lib",
		}

	filter "configurations:Release"
		targetname("Firefly")
		defines {
			"FF_RELEASE",
			"NDEBUG"
		}
		runtime "Release"
		optimize "on"
		includedirs
		{
			"../Editor/source/",
		}
		links {
			"TGAFBXImporter.lib",
			"fmod_vc.lib",
			"fmodstudio_vc.lib",
			"dpp.lib",
			"efswr.lib",
			"ImageConverter",
			"Editor",
		}
		postbuildcommands {
			'{COPY} "%{cfg.targetdir}/%{cfg.targetname}.exe" "../AssetData"',
			'{COPY} "%{cfg.targetdir}/%{cfg.targetname}.pdb" "../AssetData"'
		}
	
		linkoptions {
			"/WHOLEARCHIVE:Editor",
			"/WHOLEARCHIVE:PhysX.lib",
		}

    filter "configurations:ShipIt"
		targetname("Can Strike")
		defines {
			"FF_SHIPIT",
			"NDEBUG",
			"FF_INLAMNING"
		}
		runtime "Release"
		optimize "on"
		links {
			"TGAFBXImporter.lib",
			"fmod_vc.lib",
			"fmodstudio_vc.lib",
			"dpp.lib"
		}
		linkoptions {
			"/WHOLEARCHIVE:PhysXs.lib",
		}
		postbuildcommands {
			'{COPY} "%{cfg.targetdir}/%{cfg.targetname}.exe" "../AssetData"',
			'{COPY} "%{cfg.targetdir}/%{cfg.targetname}.pdb "../AssetData"'
		}