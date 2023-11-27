#pragma once

//EVENTS																

//TESTS
#define EVENT_TEST																	"event:/SUS"
#define EVENT_Easter																"event:/Easter"

//Error
#define EVENT_Editor_Error 															"event:/Editor/Error"
#define EVENT_Editor_Success 														"event:/Editor/Success"

//Ambient


//Music
#define EVENT_Music_MainMenu 														"event:/Music/Title"
#define EVENT_Music																	"event:/Music/Music"

#define EVENT_Music_Level1															"event:/Music/Level1"
#define EVENT_Music_Level2															"event:/Music/Level2"
#define EVENT_Music_Level3															"event:/Music/Level3"
#define EVENT_Music_Logo 															"event:/Music/Logo"
#define EVENT_Music_Loading 														"event:/Music/Loading"

//UI/UX
#define EVENT_UI_Click 																"event:/UI/Menu/SelectLevel"/*"event:/UI/Click"*/ // temp way to play SelectLevel
#define EVENT_UI_Hover 																"event:/UI/Hover"



#pragma region Avatar

//Avatar
#define EVENT_Avatar_FootSteps 														"event:/SFX/Avatar/FootSteps"
#define EVENT_Avatar_FootStep 														"event:/SFX/Avatar/FootStep"
#define EVENT_Avatar_Jump															"event:/SFX/Avatar/Jump"
#define EVENT_Avatar_JumpDouble														"event:/SFX/Avatar/Double Jump"
#define EVENT_Avatar_HittingTheGround												"event:/SFX/Avatar/Hitting the ground"
#define EVENT_Avatar_TakeDamage														"event:/SFX/Avatar/Injured"
#define EVENT_Avatar_Death															"event:/SFX/Avatar/Death"
#define EVENT_Avatar_Punch															"event:/SFX/Avatar/Punch"
#define EVENT_Avatar_PunchImpact													"event:/SFX/Avatar/PunchImpact"
#define EVENT_Avatar_Dash															"event:/SFX/Avatar/Dash"
#define EVENT_Avatar_Dash															"event:/SFX/Avatar/Dash"
#define EVENT_Avatar_CorridorWalk													"event:/SFX/Avatar/CorridorWalk"
#define EVENT_Avatar_CorridorRun													"event:/SFX/Avatar/CorridorRun"



//Weapon
	//Smg
#define EVENT_Weapon_Smg_Shoot														"event:/SFX/Weapon/Smg/Shoot"
#define EVENT_Weapon_Smg_ReloadFullMagIn											"event:/SFX/Weapon/Smg/ReloadFull-Mag-In"
#define EVENT_Weapon_Smg_ReloadFullMagOut											"event:/SFX/Weapon/Smg/ReloadFull-Mag-Out"
#define EVENT_Weapon_Smg_ReloadFullBoltForward										"event:/SFX/Weapon/Smg/ReloadFull-Bolt-Forward"
#define EVENT_Weapon_Smg_ReloadFullBoltBack											"event:/SFX/Weapon/Smg/ReloadFull-Bolt-Back"
#define EVENT_Weapon_Smg_ReloadHalfMagIn											"event:/SFX/Weapon/Smg/ReloadHalf-Mag-In"
#define EVENT_Weapon_Smg_ReloadHalfMagOut											"event:/SFX/Weapon/Smg/ReloadHalf-Mag-Out"
#define EVENT_Weapon_Smg_Equip														"event:/SFX/Weapon/Smg/Equip"
#define EVENT_Weapon_Smg_Holster													"event:/SFX/Weapon/Smg/Holster"
#define EVENT_Weapon_Smg_Inspect1													"event:/SFX/Weapon/Smg/Inspect1"
#define EVENT_Weapon_Smg_Inspect2													"event:/SFX/Weapon/Smg/Inspect2"
#define EVENT_Weapon_Smg_GrabMag													"event:/SFX/Weapon/Smg/GrabMag"
#define EVENT_Weapon_Smg_Impact														"event:/SFX/Weapon/Smg/Impact"

	//Shotgun
#define EVENT_Weapon_Shotgun_Shoot													"event:/SFX/Weapon/Shotgun/Shoot"
#define EVENT_Weapon_Shotgun_ReloadBegin											"event:/SFX/Weapon/Shotgun/ReloadBegin"
#define EVENT_Weapon_Shotgun_ReloadFinish											"event:/SFX/Weapon/Shotgun/ReloadFinish"
#define EVENT_Weapon_Shotgun_Cock													"event:/SFX/Weapon/Shotgun/Cock"
#define EVENT_Weapon_Shotgun_AddShell												"event:/SFX/Weapon/Shotgun/AddShell"
#define EVENT_Weapon_Shotgun_Equip													"event:/SFX/Weapon/Shotgun/Equip"
#define EVENT_Weapon_Shotgun_Holster												"event:/SFX/Weapon/Shotgun/Holster"
#define EVENT_Weapon_Shotgun_Spin													"event:/SFX/Weapon/Shotgun/Spin"
#define EVENT_Weapon_Shotgun_Impact													"event:/SFX/Weapon/Shotgun/Impact"


//Hit
#define EVENT_Hit_Hitmarker															"event:/SFX/Weapon/Hit/Hitmarker"
#define EVENT_Hit_Metal																"event:/SFX/Weapon/Hit/Metal"
#define EVENT_Hit_Water																"event:/SFX/Weapon/Hit/Water"
#define EVENT_Hit_Wood																"event:/SFX/Weapon/Hit/Wood"
#define EVENT_Hit_Concrete															"event:/SFX/Weapon/Hit/Concrete"
#define EVENT_Hit_AirLeakStrong														"event:/SFX/Factory/AirLeakStrong"
#define EVENT_Hit_AirLeakWeak														"event:/SFX/Factory/AirLeakWeak"

#pragma endregion

#pragma region Enemys
//Enemys
	//Melee
#define EVENT_Enemy_Melee_Attack														"event:/SFX/Enemy/Melee/Attack"
#define EVENT_Enemy_Melee_Attack2														"event:/SFX/Enemy/Melee/Attack 2"
#define EVENT_Enemy_Melee_AltAttack														"event:/SFX/Enemy/Melee/AltAttack"
#define EVENT_Enemy_Melee_AltAttack2													"event:/SFX/Enemy/Melee/AltAttack2"
#define EVENT_Enemy_Melee_Death															"event:/SFX/Enemy/Melee/Death"
#define EVENT_Enemy_Melee_Injured														"event:/SFX/Enemy/Melee/Injured"
#define EVENT_Enemy_Melee_Walk															"event:/SFX/Enemy/Melee/Walk"
#define EVENT_Enemy_Melee_Jump															"event:/SFX/Enemy/Melee/Jump"
#define EVENT_Enemy_Melee_Land															"event:/SFX/Enemy/Melee/Land"



	//Ranged
#define EVENT_Enemy_Ranged_Attack														"event:/SFX/Enemy/Ranged/Attack"
#define EVENT_Enemy_Ranged_AttackSpin													"event:/SFX/Enemy/Ranged/AttackSpin"
#define EVENT_Enemy_Ranged_Death														"event:/SFX/Enemy/Ranged/Death"
#define EVENT_Enemy_Ranged_Injured														"event:/SFX/Enemy/Ranged/Injured"
#define EVENT_Enemy_Ranged_Walk															"event:/SFX/Enemy/Ranged/Walk"
#define EVENT_Enemy_Ranged_Jump															"event:/SFX/Enemy/Ranged/Jump"
#define EVENT_Enemy_Ranged_Land															"event:/SFX/Enemy/Ranged/Land"



	//Flying
#define EVENT_Enemy_Flying_Attack														"event:/SFX/Enemy/Flying/Attack"
#define EVENT_Enemy_Flying_Death														"event:/SFX/Enemy/Flying/Death"
#define EVENT_Enemy_Flying_Injured														"event:/SFX/Enemy/Flying/Injured"
#define EVENT_Enemy_Flying_Fly															"event:/SFX/Enemy/Flying/Fly"


#pragma endregion

#pragma region Objects
//Interact
	//Switch
#define EVENT_Interact_Switch_On													"event:/SFX/Interact/SwitchOn"
#define EVENT_Interact_Switch_Off													"event:/SFX/Interact/SwitchOff"

	//Teleport
#define EVENT_Interact_Teleport														"event:/SFX/HealingCrystal/Pickup"
#define EVENT_Interact_Teleport														"event:/SFX/HealingCrystal/Idle"

	//Door
#define EVENT_Interact_Door_Open													"event:/SFX/Interact/DoorOpen"
#define EVENT_Interact_Door_Close													"event:/SFX/Interact/DoorClose"


	//Lift
#define EVENT_Interact_Lift_Start													"event:/SFX/Interact/LiftStart"
#define EVENT_Interact_Lift_Stop													"event:/SFX/Interact/LiftStop"

	//Wall
#define EVENT_Interact_Wall															"event:/SFX/Interact/Wall"


	//Pickups
#define EVENT_Interact_Health														"event:/SFX/Pickup/Health"
#define EVENT_Interact_Ammo															"event:/SFX/Pickup/Ammo"

	//LaunchPad
#define EVENT_Interact_LaunchPad													"event:/SFX/LaunchPad/LaunchPad"


//Siren
#define EVENT_Siren_waveStart														"event:/SFX/Siren/wave start"

//Telephone
#define EVENT_Telephone_Ring														"event:/SFX/Office/TelephoneRing"
#define EVENT_Telephone_RingRandom													"event:/SFX/Office/TelephoneRingRandom"
#define EVENT_Telephone_Call														"event:/SFX/Office/TelephoneCall"
#define EVENT_Telephone_PickUp														"event:/SFX/Office/TelephonePickUp"
#define EVENT_Telephone_HangUp														"event:/SFX/Office/TelephonePickUp"
#define EVENT_Telephone_AllInOne													"event:/VoiceLines/TelephoneVoicelines/Telephone_Voiceline_AllInOne"


//Typewriter
#define EVENT_Typewriter_Type														"event:/SFX/Office/Typewriter"

//Radio
#define EVENT_Radio_Music															"event:/SFX/Office/RadioMusic"

//pause ClipBoard
#define EVENT_ClipBoard_Enter														"event:/SFX/Interact/PauseEnter"
#define EVENT_ClipBoard_Exit														"event:/SFX/Interact/PauseExit"
#define EVENT_ClipBoard_Drop														"event:/SFX/ClipBoard/Droped"

//KeyPad
#define EVENT_Keypad																"event:/SFX/Interact/KeyPad"

//GarageDoor
#define EVENT_GarageDoor															"event:/SFX/Factory/GarageDoor"

//Snus
#define EVENT_Snus																	"event:/SFX/Avatar/Snus"

//Level Select
#define EVENT_LevelSelect															"event:/UI/Menu/SelectLevel"


//GloryKill
#define EVENT_GloryKill_Bonk														"event:/SFX/GloryKill/Bonk"
//GloryKill Basic
#define EVENT_GloryKill_Basic														"event:/SFX/GloryKill/Basic"
#define EVENT_GloryKill_BasicAlt													"event:/SFX/GloryKill/BasicAlt"
//GloryKill Ranged
#define EVENT_GloryKill_Ranged														"event:/SFX/GloryKill/Ranged"
#define EVENT_GloryKill_RangedAlt													"event:/SFX/GloryKill/RangedAlt"


//GloryKill Basic
#define EVENT_GloryKill_Basic_Rip													"event:/SFX/GloryKill/Basic/Rip"
#define EVENT_GloryKill_Basic_Bonk													"event:/SFX/GloryKill/Basic/Bonk"
#define EVENT_GloryKill_Basic_Hit													"event:/SFX/GloryKill/Basic/Hit"
//GloryKill Ranged
#define EVENT_GloryKill_Ranged_Rip													"event:/SFX/GloryKill/Ranged/Rip"
#define EVENT_GloryKill_Ranged_Bonk													"event:/SFX/GloryKill/Ranged/Bonk"
#define EVENT_GloryKill_Ranged_Hit													"event:/SFX/GloryKill/Ranged/Hit"

#pragma endregion

#pragma region VoiceLines
//VoiceLines
//Antagonist
#define EVENT_VoiceLines_Antagonist_WaveStart_00									"event:/VoiceLines/Antagonist/WaveStart/Antagonist_WaveStart_00"
#define EVENT_VoiceLines_Antagonist_WaveStart_01									"event:/VoiceLines/Antagonist/WaveStart/Antagonist_WaveStart_01"
#define EVENT_VoiceLines_Antagonist_WaveEnd_00										"event:/VoiceLines/Antagonist/WaveEnd/Antagonist_WaveEnd_00"
#define EVENT_VoiceLines_Antagonist_WaveEnd_01										"event:/VoiceLines/Antagonist/WaveEnd/Antagonist_WaveEnd_01"
#define EVENT_VoiceLines_Antagonist_Illegal_00										"event:/VoiceLines/Antagonist/Idle/Antagonist_Idle_00"
#define EVENT_VoiceLines_Antagonist_Illegal_01										"event:/VoiceLines/Antagonist/Idle/Antagonist_Idle_01"

//Enemies
#define EVENT_VoiceLines_ROBOTS_StopIt												"event:/SFX/Enemies/Antagonist_Aah"
#define EVENT_VoiceLines_ROBOTS_StopStriking_00										"event:/SFX/Enemies/Antagonist_StopStriking_00"
#define EVENT_VoiceLines_ROBOTS_StopStriking_01										"event:/SFX/Enemies/Antagonist_StopStriking_01"
#define EVENT_VoiceLines_ROBOTS_Aah													"event:/SFX/Enemies/Antagonist_Aah"
#define EVENT_VoiceLines_ROBOTS_ScanningProtocolEngaged								"event:/SFX/Enemies/Antagonist_ScanningProtocolEngaged"
#define EVENT_VoiceLines_ROBOTS_KillHim												"event:/SFX/Enemies/Antagonist_KillHim"
#define EVENT_VoiceLines_ROBOTS_ContractBreachDetected								"event:/SFX/Enemies/Antagonist_ContractBreachDetected"
#define EVENT_VoiceLines_ROBOTS_EliminateEliminate									"event:/SFX/Enemies/Antagonist_EliminateEliminate"





#pragma endregion

//BUSSES
#define BUS_MASTER 																	"bus:/"											
#define BUS_SFX																		"bus:/SFX"											
#define BUS_MUSIC 																	"bus:/Music"										
#define BUS_UI 																		"bus:/UI"										
#define BUS_AMBIANCE																"bus:/Ambiance"
#define BUS_VOICELINES																"bus:/VoiceLines"

//PARAMETERS
//	//Music
//#define PARAMETER_Music_Combat													"parameter:/Music/Combat"
//#define PARAMETER_Music_Biome														"parameter:/Music/Biome"

	//SFX
#define PARAMETER_GroundType														"parameter:/GroundType"
#define PARAMETER_Music																"parameter:/Music"



//SNAPSHOTS
#define SNAPSHOT_Pause																"snapshot:/Pause"
