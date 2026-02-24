OldaleTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, OldaleTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

OldaleTown_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_OLDALE_TOWN
	call Common_EventScript_UpdateBrineyLocation
	end

OldaleTown_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_OLDALE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

OldaleTown_PokemonCenter_1F_EventScript_Gentleman::
	msgbox OldaleTown_PokemonCenter_1F_Text_TrainersCanUsePC, MSGBOX_NPC
	end

OldaleTown_PokemonCenter_1F_EventScript_Boy::
	msgbox OldaleTown_PokemonCenter_1F_Text_PokemonCentersAreGreat, MSGBOX_NPC
	end

OldaleTown_PokemonCenter_1F_EventScript_Girl::
	lock
	faceplayer
	goto_if_set FLAG_SYS_POKEDEX_GET, OldaleTown_PokemonCenter_1F_EventScript_WirelessClubAvailable
	msgbox OldaleTown_PokemonCenter_1F_Text_WirelessClubNotAvailable, MSGBOX_DEFAULT
	release
	end

OldaleTown_PokemonCenter_1F_EventScript_WirelessClubAvailable::
	msgbox OldaleTown_PokemonCenter_1F_Text_TradedInWirelessClub, MSGBOX_DEFAULT
	release
	end

OldaleTown_PokemonCenter_1F_Text_TrainersCanUsePC:
	.string "That PC in the corner there is\n"
	.string "for any POKéMON TRAINER to use.\p"
	.string "Naturally, that means you're welcome\n"
	.string "to use it, too.$"

OldaleTown_PokemonCenter_1F_Text_PokemonCentersAreGreat:
	.string "POKéMON CENTERS are great!\p"
	.string "You can use their services as much\n"
	.string "as you like, and it's all for free.\l"
	.string "You never have to worry!$"

OldaleTown_PokemonCenter_1F_Text_WirelessClubNotAvailable:
	.string "The POKéMON WIRELESS CLUB on\n"
	.string "the second floor was built recently.\p"
	.string "But they say they're still making\n"
	.string "adjustments.$"

OldaleTown_PokemonCenter_1F_Text_TradedInWirelessClub:
	.string "The POKéMON WIRELESS CLUB on\n"
	.string "the second floor was built recently.\p"
	.string "I traded POKéMON right away.$"

OldaleTown_ResourceNPC::
	lock
	faceplayer
	message OldaleTown_ResourceNPC_Text
	multichoice 0, 0, MULTI_RESOURCE_NPC, 0
	closemessage
	switch VAR_RESULT
	case 0, OldaleTown_ResourceNPC_PreStatus
	case 1, OldaleTown_ResourceNPC_PreDamage
	case 2, OldaleTown_ResourceNPC_IV
	case 3, OldaleTown_ResourceNPC_Nature
	case 4, OldaleTown_ResourceNPC_Nickname
	case 5, OldaleTown_ResourceNPC_Information
	case MULTI_B_PRESSED, OldaleTown_ResourceNPC_ComeAgain

OldaleTown_ResourceNPC_PreStatus::
	message OldaleTown_ChooseStatus_Text
	multichoice 0, 0, MULTI_PRESTATUS, 0
	closemessage
	switch VAR_RESULT
	case 0, OldaleTown_ResourceNPC_Burn
	case 1, OldaleTown_ResourceNPC_Freeze
	case 2, OldaleTown_ResourceNPC_Poison
	case 3, OldaleTown_ResourceNPC_Paralysis
	case 4, OldaleTown_ResourceNPC_Sleep
	case 5, OldaleTown_ResourceNPC_Frostbite
	case MULTI_B_PRESSED, OldaleTown_ResourceNPC_ComeAgain

OldaleTown_ResourceNPC_PreDamage::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC
	special PreDamage
	waitstate
	releaseall
	end

OldaleTown_ResourceNPC_IV::
	checkitem ITEM_HEART_SCALE
	goto_if_eq VAR_RESULT, FALSE, NoHeartScale
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC
	goto_if_ne VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_ChooseIV
	end

OldaleTown_ResourceNPC_ChooseIV::
	message OldaleTown_ChooseIV_Text
	multichoice 0, 0, MULTI_IVS, 0
	closemessage
	switch VAR_RESULT
	case 0, OldaleTown_ResourceNPC_MaxHP
	case 1, OldaleTown_ResourceNPC_MaxAtk
	case 2, OldaleTown_ResourceNPC_MaxDef
	case 3, OldaleTown_ResourceNPC_MaxSpAtk
	case 4, OldaleTown_ResourceNPC_MaxSpDef
	case 5, OldaleTown_ResourceNPC_MaxSpd
	case MULTI_B_PRESSED, OldaleTown_ResourceNPC_ComeAgain

OldaleTown_ResourceNPC_MaxHP::
	specialvar VAR_RESULT, ScriptGetPartyMonHP
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmHp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxHpIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

NoHeartScale::
	msgbox NoHeartScale_Text, MSGBOX_DEFAULT
	closemessage
	release
	end

NotEnoughHeartScale::
	msgbox NotEnoughHeartScale_Text, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_MaxAtk::
	specialvar VAR_RESULT, ScriptGetPartyMonAtk
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmAtk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxAtkIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

OldaleTown_ResourceNPC_MaxDef::
	specialvar VAR_RESULT, ScriptGetPartyMonDef
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmDef, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxDefIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

OldaleTown_ResourceNPC_MaxSpd::
	specialvar VAR_RESULT, ScriptGetPartyMonSpd
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmSpd, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxSpdIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

OldaleTown_ResourceNPC_MaxSpAtk::
	specialvar VAR_RESULT, ScriptGetPartyMonSpAtk
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmSpAtk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxSpAtkIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

OldaleTown_ResourceNPC_MaxSpDef::
	specialvar VAR_RESULT, ScriptGetPartyMonSpDef
	goto_if_eq VAR_RESULT, 31, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmSpDef, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_ChooseIV
	special MaxSpDefIV
	playse SE_SAVE
	removeitem ITEM_HEART_SCALE
	release
	end

OldaleTown_ResourceNPC_NoEffect::
	msgbox OldaleTown_ResourceNPC_NoEffect_Text, MSGBOX_DEFAULT
	waitmessage
	release
	end


OldaleTown_ResourceNPC_Nickname::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC
	goto_if_ne VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_NameRatersHouse_EventScript_RateMonNickname
	end

OldaleTown_ResourceNPC_Information::
	msgbox OldaleTown_Info, MSGBOX_DEFAULT
	waitmessage
	goto OldaleTown_ResourceNPC
	release
	end

OldaleTown_ResourceNPC_Burn::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_BURN
	playse SE_M_EMBER
	msgbox BurnHeal, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Freeze::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_FREEZE
	playse SE_M_ICY_WIND
	msgbox Cold, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Poison::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_POISON
	playse SE_M_TOXIC
	msgbox Toxic, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Paralysis::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_PARALYSIS
	playse SE_M_THUNDER_WAVE
	msgbox Para, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Sleep::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_SLEEP
	playse SE_M_YAWN
	msgbox Sleep, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Frostbite::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC_PreStatus
	setmonstatus STATUS1_FROSTBITE
	playse SE_M_ICY_WIND
	msgbox Frosbite, MSGBOX_DEFAULT
	closemessage
	release
	end	

OldaleTown_ResourceNPC_Nature::
	checkitem ITEM_HEART_SCALE, 3
	goto_if_eq VAR_RESULT, FALSE, NotEnoughHeartScale
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_ResourceNPC
	message OldaleTown_ResourceNPC_SelectNature_Text
	scrollingmultichoice 0, 0, 0, 5, TRUE
	closemessage
	switch VAR_RESULT
	case 0, OldaleTown_ResourceNPC_Hardy
	case 1, OldaleTown_ResourceNPC_Lonely
	case 2, OldaleTown_ResourceNPC_Brave
	case 3, OldaleTown_ResourceNPC_Adamant
	case 4, OldaleTown_ResourceNPC_Naughty
	case 5, OldaleTown_ResourceNPC_Bold
	case 6, OldaleTown_ResourceNPC_Docile
	case 7, OldaleTown_ResourceNPC_Relaxed
	case 8, OldaleTown_ResourceNPC_Impish
	case 9, OldaleTown_ResourceNPC_Lax
	case 10, OldaleTown_ResourceNPC_Timid
	case 11, OldaleTown_ResourceNPC_Hasty
	case 12, OldaleTown_ResourceNPC_Serious
	case 13, OldaleTown_ResourceNPC_Jolly
	case 14, OldaleTown_ResourceNPC_Naive
	case 15, OldaleTown_ResourceNPC_Modest
	case 16, OldaleTown_ResourceNPC_Mild
	case 17, OldaleTown_ResourceNPC_Quiet
	case 18, OldaleTown_ResourceNPC_Bashful
	case 19, OldaleTown_ResourceNPC_Rash
	case 20, OldaleTown_ResourceNPC_Calm
	case 21, OldaleTown_ResourceNPC_Gentle
	case 22, OldaleTown_ResourceNPC_Sassy
	case 23, OldaleTown_ResourceNPC_Careful
	case 24, OldaleTown_ResourceNPC_Quirky
	case MULTI_B_PRESSED, OldaleTown_ResourceNPC_Nature


OldaleTown_ResourceNPC_Hardy::
	setvar VAR_TEMP_1, NATURE_HARDY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmHardy, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_HARDY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Lonely::
	setvar VAR_TEMP_1, NATURE_LONELY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmLonely, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_LONELY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Brave::
	setvar VAR_TEMP_1, NATURE_BRAVE
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmBrave, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_BRAVE
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Adamant::
	setvar VAR_TEMP_1, NATURE_ADAMANT
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmAdamant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_ADAMANT
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Naughty::
	setvar VAR_TEMP_1, NATURE_NAUGHTY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmNaughty, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_NAUGHTY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Bold::
	setvar VAR_TEMP_1, NATURE_BOLD
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmBold, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_BOLD
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Docile::
	setvar VAR_TEMP_1, NATURE_DOCILE
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmDocile, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_DOCILE
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Relaxed::
	setvar VAR_TEMP_1, NATURE_RELAXED
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmRelaxed, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_RELAXED
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Impish::
	setvar VAR_TEMP_1, NATURE_IMPISH
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmImpish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_IMPISH
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Lax::
	setvar VAR_TEMP_1, NATURE_LAX
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmLax, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_LAX
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Timid::
	setvar VAR_TEMP_1, NATURE_TIMID
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmTimid, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_TIMID
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Hasty::
	setvar VAR_TEMP_1, NATURE_HASTY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmHasty, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_HASTY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Serious::
	setvar VAR_TEMP_1, NATURE_SERIOUS
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmSerious, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_SERIOUS
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Jolly::
	setvar VAR_TEMP_1, NATURE_JOLLY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmJolly, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_JOLLY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Naive::
	setvar VAR_TEMP_1, NATURE_NAIVE
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmNaive, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_NAIVE
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Modest::
	setvar VAR_TEMP_1, NATURE_MODEST
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmModest, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_MODEST
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Mild::
	setvar VAR_TEMP_1, NATURE_MILD
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmMild, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_MILD
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Quiet::
	setvar VAR_TEMP_1, NATURE_QUIET
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmQuiet, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_QUIET
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Bashful::
	setvar VAR_TEMP_1, NATURE_BASHFUL
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmBashful, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_BASHFUL
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Rash::
	setvar VAR_TEMP_1, NATURE_RASH
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmRash, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_RASH
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Calm::
	setvar VAR_TEMP_1, NATURE_CALM
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmCalm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_CALM
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Gentle::
	setvar VAR_TEMP_1, NATURE_GENTLE
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmGentle, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_GENTLE
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Sassy::
	setvar VAR_TEMP_1, NATURE_SASSY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmSassy, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_SASSY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Careful::
	setvar VAR_TEMP_1, NATURE_CAREFUL
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmCareful, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_CAREFUL
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_Quirky::
	setvar VAR_TEMP_1, NATURE_QUIRKY
	specialvar VAR_RESULT, ScriptCheckPartyMonNature
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_ResourceNPC_NoEffect
	msgbox OldaleTown_ConfirmQuirky, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_ResourceNPC_Nature
	setmonnature NATURE_QUIRKY
	removeitem ITEM_HEART_SCALE, 3
	playse SE_SAVE
	msgbox Done, MSGBOX_DEFAULT
	closemessage
	release
	end

OldaleTown_ResourceNPC_ComeAgain::
	msgbox OldaleTown_ResourceNPC_ComeAgain_Text, MSGBOX_DEFAULT
	waitmessage
	release
	end

OldaleTown_NameRatersHouse_EventScript_RateMonNickname::
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, OldaleTown_NameRatersHouse_EventScript_CantRateEgg
	special BufferMonNickname
	special IsMonOTIDNotPlayers
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_NameRatersHouse_EventScript_PlayerNotMonsOT
	specialvar VAR_RESULT, MonOTNameNotPlayer
	special BufferMonNickname
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_NameRatersHouse_EventScript_PlayerNotMonsOT
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_NameRatersHouse_EventScript_ChangeNickname
	end

OldaleTown_NameRatersHouse_EventScript_CantRateEgg::
	msgbox OldaleTown_NameRatersHouse_Text_ThatIsMerelyAnEgg, MSGBOX_DEFAULT
	release
	end

OldaleTown_NameRatersHouse_EventScript_PlayerNotMonsOT::
	msgbox OldaleTown_NameRatersHouse_Text_MagnificentName, MSGBOX_DEFAULT
	release
	end

OldaleTown_NameRatersHouse_EventScript_ChangeNickname::
	msgbox OldaleTown_NameRatersHouse_Text_WhatShallNewNameBe, MSGBOX_DEFAULT
	call Common_EventScript_NameReceivedPartyMon
	specialvar VAR_RESULT, TryPutNameRaterShowOnTheAir
	special BufferMonNickname
	release
	end

OldaleTown_ResourceNPC_SelectNature_Text:
	.string "Which nature would you like?$"

NoHeartScale_Text:
	.string "You don't have any heart scales!$"

NotEnoughHeartScale_Text:
	.string "You don't have enough heart scales!$"

BurnHeal:
	.string "You better have Burn Heal!$"

Cold:
	.string "Now that's cold!$"

Toxic:
	.string "Don't you know that you're Toxic?$"

Para:
	.string "Shocking!$"

Sleep:
	.string "Sleep well, sweet prince.$"

Frosbite:
	.string "Chill out, you look a little frosty!$"	

Done:
	.string "Consider it done!$"

OldaleTown_ResourceNPC_Text:
	.string "What can I do for you?$"

OldaleTown_ChooseStatus_Text:
	.string "What status do you need?$"

OldaleTown_ChooseIV_Text:
	.string "What stat would you like to maximize?$"

OldaleTown_ChooseNature_Text:
	.string "What nature would you like?$"

OldaleTown_ResourceNPC_ComeAgain_Text:
	.string "Have a nice day!$"

OldaleTown_ResourceNPC_NoEffect_Text:
	.string "It would have no effect.$"

OldaleTown_NameRatersHouse_Text_WhatShallNewNameBe:
	.string "What shall the new nickname be?$"

OldaleTown_NameRatersHouse_Text_ThatIsMerelyAnEgg:
	.string "You can't nickname an EGG!$"

OldaleTown_NameRatersHouse_Text_MagnificentName:
	.string "It would be rude to change\n"
	.string "this Pokémon's name!$"

OldaleTown_ConfirmHardy:
	.string "Change to Hardy nature?$"

OldaleTown_ConfirmLonely:
	.string "Change to Lonely nature?$"

OldaleTown_ConfirmBrave:
	.string "Change to Brave nature?$"

OldaleTown_ConfirmAdamant:
	.string "Change to Adamant nature?$"

OldaleTown_ConfirmNaughty:
	.string "Change to Naughty nature?$"

OldaleTown_ConfirmBold:
	.string "Change to Bold nature?$"

OldaleTown_ConfirmDocile:
	.string "Change to Docile nature?$"

OldaleTown_ConfirmRelaxed:
	.string "Change to Relaxed nature?$"

OldaleTown_ConfirmImpish:
	.string "Change to Impish nature?$"

OldaleTown_ConfirmLax:
	.string "Change to Lax nature?$"

OldaleTown_ConfirmTimid:
	.string "Change to Timid nature?$"

OldaleTown_ConfirmHasty:
	.string "Change to Hasty nature?$"

OldaleTown_ConfirmSerious:
	.string "Change to Serious nature?$"

OldaleTown_ConfirmJolly:
	.string "Change to Jolly nature?$"

OldaleTown_ConfirmNaive:
	.string "Change to Naive nature?$"

OldaleTown_ConfirmModest:
	.string "Change to Modest nature?$"

OldaleTown_ConfirmMild:
	.string "Change to Mild nature?$"

OldaleTown_ConfirmQuiet:
	.string "Change to Quiet nature?$"

OldaleTown_ConfirmBashful:
	.string "Change to Bashful nature?$"

OldaleTown_ConfirmRash:
	.string "Change to Rash nature?$"

OldaleTown_ConfirmCalm:
	.string "Change to Calm nature?$"

OldaleTown_ConfirmGentle:
	.string "Change to Gentle nature?$"

OldaleTown_ConfirmSassy:
	.string "Change to Sassy nature?$"

OldaleTown_ConfirmCareful:
	.string "Change to Careful nature?$"

OldaleTown_ConfirmQuirky:
	.string "Change to Quirky nature?$"

OldaleTown_ConfirmHp:
	.string "Maximize HP?$"

OldaleTown_ConfirmAtk:
	.string "Maximize Attack?$"

OldaleTown_ConfirmDef:
	.string "Maximize Defense?$"

OldaleTown_ConfirmSpAtk:
	.string "Maximize Sp.Atk?$"

OldaleTown_ConfirmSpDef:
	.string "Maximize Sp.Def?$"

OldaleTown_ConfirmSpd:
	.string "Maximize Speed?$"

OldaleTown_NoScales:
	.string "You do not have enough Heart Scales.$"

OldaleTown_Info:
	.string "PreStatus and nickname services\n"
	.string "free of charge.\p"
	.string "If you have 1 Heart Scale I can\n"
	.string "maximize one of your Pokémon's stats!\p"
	.string "For 3 Heart Scales, I can even\n"
	.string "change your Pokémon's nature!$"
