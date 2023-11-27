workspace "Firefly"
	startproject "Launcher"
	architecture "x64"

	configurations{
		"Debug",
		"Release",
        "ShipIt"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group("Core")
include "Editor"
include "Firefly"
include "Launcher"
group("")

group("Dependencies")
include "Utils"
include "ImGui"
include "msdfgen"
include "FBXImporter"
include "Optick"
include "SoundEngine"
include "simdjson"
include "DDSTextureLoader"
include "ImGuiNodeEditor"
include "MuninGraph"
include "VisualScripting"
include "SerializationUtils"
include "ImageConverter"
include "meshoptimizer"
include "Discord-RPC"
include "DiscordAPI"
include "PhysX"
group("")

include "Game"