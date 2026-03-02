QuickMenu_EventScript_Open::
	lockall
	goto QuickMenu_EventScript_ShowMainMenu
	end

QuickMenu_EventScript_ShowMainMenu::
	multichoice 0, 0, MULTI_QUICK_MENU_MAIN, FALSE
	switch VAR_RESULT
	case 0, QuickMenu_EventScript_PortaPC
	case 1, QuickMenu_EventScript_PokeVial
	case 2, QuickMenu_EventScript_TeachMove
	case 3, QuickMenu_EventScript_ShowUtilityMenu
	case 4, QuickMenu_EventScript_EncounterViewer
	case 5, QuickMenu_EventScript_Repel
	case 6, QuickMenu_EventScript_PokeRider
	case MULTI_B_PRESSED, QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_PortaPC::
	setvar VAR_0x8004, PC_LOCATION_OTHER
	special DoPCTurnOnEffect
	playse SE_PC_ON
	msgbox Text_BootUpPC, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

QuickMenu_EventScript_PokeVial::
	playse SE_USE_ITEM
	special HealPlayerParty
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_TeachMove::
	goto FallarborTown_MoveRelearnersHouse_EventScript_AskTeachMove
	end

QuickMenu_EventScript_ShowUtilityMenu::
	multichoice 0, 0, MULTI_QUICK_MENU_UTILITY, FALSE
	switch VAR_RESULT
	case 0, QuickMenu_EventScript_ShowTimeMenu
	case 1, QuickMenu_EventScript_HatchEgg
	case 2, QuickMenu_EventScript_ShowMainMenu
	case MULTI_B_PRESSED, QuickMenu_EventScript_ShowMainMenu
	end

QuickMenu_EventScript_ShowTimeMenu::
	multichoice 0, 0, MULTI_QUICK_MENU_TIME, FALSE
	switch VAR_RESULT
	case 0, QuickMenu_EventScript_SetDay
	case 1, QuickMenu_EventScript_SetNight
	case 2, QuickMenu_EventScript_ShowUtilityMenu
	case MULTI_B_PRESSED, QuickMenu_EventScript_ShowUtilityMenu
	end

QuickMenu_EventScript_SetDay::
	setvar VAR_0x8004, DAY_HOUR_BEGIN
	special QuickMenu_SetTimeOverride
	msgbox QuickMenu_Text_TimeSetDay, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_SetNight::
	setvar VAR_0x8004, NIGHT_HOUR_BEGIN + 1
	special QuickMenu_SetTimeOverride
	msgbox QuickMenu_Text_TimeSetNight, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_HatchEgg::
	getpartysize
	goto_if_eq VAR_RESULT, 0, QuickMenu_EventScript_NoPokemon
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, QuickMenu_EventScript_ShowUtilityMenu
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, FALSE, QuickMenu_EventScript_NotAnEgg
	special EggHatch
	waitstate
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_NoPokemon::
	msgbox QuickMenu_Text_NoPokemon, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_ShowUtilityMenu
	end

QuickMenu_EventScript_NotAnEgg::
	msgbox QuickMenu_Text_NotAnEgg, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_ShowUtilityMenu
	end

QuickMenu_EventScript_EncounterViewer::
	releaseall
	special QuickMenu_OpenEncounterViewer
	end

QuickMenu_EventScript_Repel::
	specialvar VAR_RESULT, QuickMenu_ToggleInfiniteRepel
	goto_if_eq VAR_RESULT, TRUE, QuickMenu_EventScript_RepelOn
	msgbox gText_InfiniteRepelOff, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_RepelOn::
	msgbox gText_InfiniteRepelOn, MSGBOX_DEFAULT
	goto QuickMenu_EventScript_Close
	end

QuickMenu_EventScript_PokeRider::
	releaseall
	special QuickMenu_OpenFlyMap
	end

QuickMenu_EventScript_Close::
	releaseall
	end

QuickMenu_Text_PortaPC:
	.string "Porta-PC$"

QuickMenu_Text_PokeVial:
	.string "Poke Vial$"

QuickMenu_Text_TeachMove:
	.string "Teach Move$"

QuickMenu_Text_Utility:
	.string "Utility$"

QuickMenu_Text_EncounterTable:
	.string "Enc. Table$"

QuickMenu_Text_Repel:
	.string "Repel$"

QuickMenu_Text_PokeRider:
	.string "Poke Rider$"

QuickMenu_Text_TimeOfDay:
	.string "Time of Day$"

QuickMenu_Text_HatchEgg:
	.string "Hatch Egg$"

QuickMenu_Text_Back:
	.string "Back$"

QuickMenu_Text_Day:
	.string "Day$"

QuickMenu_Text_Night:
	.string "Night$"

QuickMenu_Text_TimeSetDay:
	.string "Time of day was set to day.$"

QuickMenu_Text_TimeSetNight:
	.string "Time of day was set to night.$"

QuickMenu_Text_NoPokemon:
	.string "There are no Pokemon in your party.$"

QuickMenu_Text_NotAnEgg:
	.string "That Pokemon is not an Egg.$"
