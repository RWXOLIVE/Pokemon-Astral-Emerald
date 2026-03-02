#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_info.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "battle_message.h"
#include "battle_util.h"
#include "bg.h"
#include "data.h"
#include "gpu_regs.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "reshow_battle_screen.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "util.h"
#include "window.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"

#define BATTLE_INFO_PAGE_FIELD      0
#define BATTLE_INFO_PAGE_MON        1
#define BATTLE_INFO_PAGE_AI_DAMAGE  2
#define BATTLE_INFO_PAGE_COUNT      3

#define BATTLE_INFO_WINDOW_BASE_BLOCK 0x0001
#define AI_8TH_ROLL_PERCENTAGE        92

struct BattleInfoMenu
{
    u8 originatingBattler;
    u8 selectedBattler;
    u8 page;
    u8 windowId;
    u8 monSpriteId;
    u8 iconSpriteIds[4];
    bool8 iconPalettesLoaded;
};

static EWRAM_DATA u8 sBattleInfoInitialBattler = 0;

static const u16 sBattleInfoBgColor[] = {RGB_WHITE};

static const struct BgTemplate sBattleInfoBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 20,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sBattleInfoWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 30,
    .height = 20,
    .paletteNum = 0xF,
    .baseBlock = BATTLE_INFO_WINDOW_BASE_BLOCK,
};

static const u8 sTextBattleInfo[] = _("Battle Info");
static const u8 sTextAButtonNextPage[] = _("{A_BUTTON} Next Page");
static const u8 sTextBButtonExit[] = _("{B_BUTTON} Exit");
static const u8 sTextDPadNextMon[] = _("D-Pad Next Mon");
static const u8 sTextPlayer[] = _("Player");
static const u8 sTextAi[] = _("AI");
static const u8 sTextFoe[] = _("Foe");
static const u8 sTextTailwind[] = _("Tailwind");
static const u8 sTextReflect[] = _("Reflect");
static const u8 sTextLightScreen[] = _("Light Screen");
static const u8 sTextAuroraVeil[] = _("Aurora Veil");
static const u8 sTextTrickRoom[] = _("Trick Room");
static const u8 sTextTerrain[] = _("Terrain");
static const u8 sTextAbility[] = _("Ability");
static const u8 sTextHeldItem[] = _("Held Item");
static const u8 sTextNoData[] = _("No data");
static const u8 sTextDash[] = _("-");
static const u8 sTextAiDamageRoll[] = _("AI 8th Move Damage Roll");
static const u8 sTextNoAiDamage[] = _("No AI damage data available.");
static const u8 sTextPp[] = _("PP");
static const u8 sTextSlash[] = _("/");
static const u8 sTextTerrainElectric[] = _("Electric");
static const u8 sTextTerrainGrassy[] = _("Grassy");
static const u8 sTextTerrainMisty[] = _("Misty");
static const u8 sTextTerrainPsychic[] = _("Psychic");

static struct BattleInfoMenu *GetStructPtr(u8 taskId);
static void SetStructPtr(u8 taskId, void *ptr);
void CB2_BattleInfoMenu(void);
static void MainCB2(void);
static void VBlankCB(void);
static void Task_BattleInfoFadeIn(u8 taskId);
static void Task_BattleInfoHandleInput(u8 taskId);
static void Task_BattleInfoFadeOut(u8 taskId);
static void DestroyBattleInfoSprites(struct BattleInfoMenu *data);
static void DrawBattleInfoPage(struct BattleInfoMenu *data);
static void DrawBattleInfoHeader(struct BattleInfoMenu *data, const u8 *title, bool32 showMonPrompt);
static void DrawFieldInfoPage(struct BattleInfoMenu *data);
static void DrawMonInfoPage(struct BattleInfoMenu *data);
static void DrawAiDamagePage(struct BattleInfoMenu *data);
static void DrawAiDamageSinglesPage(struct BattleInfoMenu *data, u8 battlerAtk, u8 battlerDef);
static void DrawAiDamageDoublesPage(struct BattleInfoMenu *data, const u8 *attackers, u8 attackerCount, const u8 *defenders, u8 defenderCount);
static bool32 SelectAdjacentBattler(struct BattleInfoMenu *data, bool32 forward);
static u8 GetLivingBattlersOnSide(u32 side, u8 *battlers);
static u8 GetFirstLivingBattlerOnSide(u32 side);
static void PrintText(u8 windowId, u8 fontId, const u8 *text, s16 x, s16 y);
static void PrintFittedText(u8 windowId, u8 fontId, const u8 *text, s16 x, s16 y, u16 width);
static void PrintRightAlignedNumber(u8 windowId, u8 fontId, s32 value, u8 digits, s16 rightX, s16 y);
static void PrintCenteredText(u8 windowId, u8 fontId, const u8 *text, s16 centerX, s16 y);
static const u8 *GetTerrainName(void);
static void GetStatStageText(u8 *dst, u8 statStage);
static const u8 *GetBattlerDisplayName(u8 battler);
static u16 GetMoveDamageRoll(u8 battlerAtk, u8 battlerDef, u16 move, u8 rollPercentage);
static u8 CreateBattlerSpriteForInfo(u8 battler, s16 x, s16 y, u8 paletteSlot);
static u8 CreateBattlerIconForInfo(struct BattleInfoMenu *data, u8 slot, u8 battler, s16 x, s16 y, u8 subpriority);

static struct BattleInfoMenu *GetStructPtr(u8 taskId)
{
    u8 *taskDataPtr = (u8 *)(&gTasks[taskId].data[0]);

    return (struct BattleInfoMenu *)(T1_READ_PTR(taskDataPtr));
}

static void SetStructPtr(u8 taskId, void *ptr)
{
    u32 structPtr = (u32)(ptr);
    u8 *taskDataPtr = (u8 *)(&gTasks[taskId].data[0]);

    taskDataPtr[0] = structPtr >> 0;
    taskDataPtr[1] = structPtr >> 8;
    taskDataPtr[2] = structPtr >> 16;
    taskDataPtr[3] = structPtr >> 24;
}

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void BattleInfo_ShowActionHint(void)
{
    if (!BattleInfo_IsAvailable())
        return;

    TryToAddBattleInfoWindow();
}

void BattleInfo_HideActionHint(void)
{
    TryToHideBattleInfoWindow();
}

bool32 BattleInfo_IsAvailable(void)
{
    u32 battleTypeFlags = gBattleTypeFlags;
    bool32 isTrainerStyleBattle = (battleTypeFlags & (BATTLE_TYPE_TRAINER
                                                   | BATTLE_TYPE_FRONTIER
                                                   | BATTLE_TYPE_EREADER_TRAINER
                                                   | BATTLE_TYPE_TRAINER_HILL
                                                   | BATTLE_TYPE_SECRET_BASE)) != 0;
    bool32 isUnsupportedBattle = (battleTypeFlags & (BATTLE_TYPE_LINK
                                                  | BATTLE_TYPE_RECORDED
                                                  | BATTLE_TYPE_RECORDED_LINK
                                                  | BATTLE_TYPE_SAFARI
                                                  | BATTLE_TYPE_WALLY_TUTORIAL)) != 0;

    return isTrainerStyleBattle && !isUnsupportedBattle;
}

void OpenBattleInfoMenu(u32 battler)
{
    sBattleInfoInitialBattler = battler;
    SetMainCallback2(CB2_BattleInfoMenu);
}

void CB2_BattleInfoMenu(void)
{
    u8 taskId;
    struct BattleInfoMenu *data;

    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBattleInfoBgTemplates, ARRAY_COUNT(sBattleInfoBgTemplates));
        ResetAllBgsCoordinates();
        FreeAllWindowBuffers();
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadPalette(sBattleInfoBgColor, 0, sizeof(sBattleInfoBgColor));
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xF0, 16);
        gMain.state++;
        break;
    case 4:
        taskId = CreateTask(Task_BattleInfoFadeIn, 0);
        data = AllocZeroed(sizeof(*data));
        SetStructPtr(taskId, data);
        data->originatingBattler = sBattleInfoInitialBattler;
        data->selectedBattler = sBattleInfoInitialBattler;
        data->page = BATTLE_INFO_PAGE_FIELD;
        data->windowId = AddWindow(&sBattleInfoWindowTemplate);
        data->monSpriteId = SPRITE_NONE;
        for (u32 i = 0; i < ARRAY_COUNT(data->iconSpriteIds); i++)
            data->iconSpriteIds[i] = SPRITE_NONE;
        data->iconPalettesLoaded = FALSE;
        PutWindowTilemap(data->windowId);
        gMain.state++;
        break;
    case 5:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_BattleInfoFadeIn(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    DrawBattleInfoPage(GetStructPtr(taskId));
    gTasks[taskId].func = Task_BattleInfoHandleInput;
}

static void Task_BattleInfoHandleInput(u8 taskId)
{
    struct BattleInfoMenu *data = GetStructPtr(taskId);
    bool32 redraw = FALSE;

    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(-1, 0, 0, 0x10, RGB_BLACK);
        gTasks[taskId].func = Task_BattleInfoFadeOut;
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        data->page++;
        if (data->page >= BATTLE_INFO_PAGE_COUNT)
            data->page = BATTLE_INFO_PAGE_FIELD;
        redraw = TRUE;
    }
    else if (data->page == BATTLE_INFO_PAGE_MON)
    {
        if (JOY_NEW(DPAD_RIGHT) || JOY_NEW(DPAD_DOWN))
            redraw = SelectAdjacentBattler(data, TRUE);
        else if (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_UP))
            redraw = SelectAdjacentBattler(data, FALSE);
    }

    if (redraw)
        DrawBattleInfoPage(data);
}

static void Task_BattleInfoFadeOut(u8 taskId)
{
    struct BattleInfoMenu *data;

    if (gPaletteFade.active)
        return;

    data = GetStructPtr(taskId);
    DestroyBattleInfoSprites(data);
    ClearWindowTilemap(data->windowId);
    RemoveWindow(data->windowId);
    FreeAllWindowBuffers();
    Free(data);
    DestroyTask(taskId);
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

static void DestroyBattleInfoSprites(struct BattleInfoMenu *data)
{
    u32 i;

    if (data->monSpriteId != SPRITE_NONE)
    {
        FreeAndDestroyMonPicSprite(data->monSpriteId);
        data->monSpriteId = SPRITE_NONE;
    }

    for (i = 0; i < ARRAY_COUNT(data->iconSpriteIds); i++)
    {
        if (data->iconSpriteIds[i] != SPRITE_NONE)
        {
            FreeAndDestroyMonIconSprite(&gSprites[data->iconSpriteIds[i]]);
            data->iconSpriteIds[i] = SPRITE_NONE;
        }
    }

    if (data->iconPalettesLoaded)
    {
        FreeMonIconPalettes();
        data->iconPalettesLoaded = FALSE;
    }
}

static void DrawBattleInfoPage(struct BattleInfoMenu *data)
{
    FillWindowPixelBuffer(data->windowId, 0x11);
    DestroyBattleInfoSprites(data);

    switch (data->page)
    {
    case BATTLE_INFO_PAGE_FIELD:
        DrawFieldInfoPage(data);
        break;
    case BATTLE_INFO_PAGE_MON:
        DrawMonInfoPage(data);
        break;
    case BATTLE_INFO_PAGE_AI_DAMAGE:
        DrawAiDamagePage(data);
        break;
    }

    CopyWindowToVram(data->windowId, COPYWIN_FULL);
}

static void DrawBattleInfoHeader(struct BattleInfoMenu *data, const u8 *title, bool32 showMonPrompt)
{
    PrintText(data->windowId, FONT_NORMAL, title, 6, 4);
    PrintText(data->windowId, FONT_SMALL, sTextAButtonNextPage, 148, 6);
    PrintText(data->windowId, FONT_SMALL, sTextBButtonExit, 186, 145);
    if (showMonPrompt)
        PrintText(data->windowId, FONT_SMALL, sTextDPadNextMon, 140, 20);
}

static void DrawFieldInfoPage(struct BattleInfoMenu *data)
{
    u8 text[24];
    u8 foeBattler;

    DrawBattleInfoHeader(data, sTextBattleInfo, FALSE);
    foeBattler = GetFirstLivingBattlerOnSide(B_SIDE_OPPONENT);
    PrintCenteredText(data->windowId, FONT_NORMAL, sTextPlayer, 126, 30);
    PrintCenteredText(data->windowId, FONT_NORMAL, (foeBattler != MAX_BATTLERS_COUNT && BattlerHasAi(foeBattler)) ? sTextAi : sTextFoe, 184, 30);

    PrintText(data->windowId, FONT_NORMAL, sTextTailwind, 18, 44);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_PLAYER].tailwindTimer, 2, 147, 44);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_OPPONENT].tailwindTimer, 2, 205, 44);

    PrintText(data->windowId, FONT_NORMAL, sTextReflect, 18, 62);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_PLAYER].reflectTimer, 2, 147, 62);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_OPPONENT].reflectTimer, 2, 205, 62);

    PrintText(data->windowId, FONT_NORMAL, sTextLightScreen, 18, 80);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_PLAYER].lightscreenTimer, 2, 147, 80);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_OPPONENT].lightscreenTimer, 2, 205, 80);

    PrintText(data->windowId, FONT_NORMAL, sTextAuroraVeil, 18, 98);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_PLAYER].auroraVeilTimer, 2, 147, 98);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gSideTimers[B_SIDE_OPPONENT].auroraVeilTimer, 2, 205, 98);

    PrintText(data->windowId, FONT_NORMAL, sTextTrickRoom, 18, 118);
    PrintRightAlignedNumber(data->windowId, FONT_NORMAL, gFieldTimers.trickRoomTimer, 2, 106, 118);

    PrintText(data->windowId, FONT_NORMAL, sTextTerrain, 18, 134);
    if (gFieldStatuses & STATUS_FIELD_TERRAIN_ANY)
    {
        const u8 *terrain = GetTerrainName();
        u16 length;

        StringCopy(text, terrain);
        length = StringLength(text);
        text[length++] = CHAR_SPACE;
        text[length] = EOS;
        ConvertIntToDecimalStringN(text + StringLength(text), gFieldTimers.terrainTimer, STR_CONV_MODE_LEFT_ALIGN, 2);
        PrintText(data->windowId, FONT_SMALL, text, 82, 136);
    }
    else
    {
        PrintRightAlignedNumber(data->windowId, FONT_NORMAL, 0, 2, 106, 134);
    }
}

static void DrawMonInfoPage(struct BattleInfoMenu *data)
{
    u8 statText[8];
    u16 item = gBattleMons[data->selectedBattler].item;
    const u8 *abilityName;
    const u8 *itemName;

    DrawBattleInfoHeader(data, GetBattlerDisplayName(data->selectedBattler), TRUE);
    data->monSpriteId = CreateBattlerSpriteForInfo(data->selectedBattler, 40, 50, 15);
    if (data->monSpriteId != SPRITE_NONE)
    {
        // Keep the info portrait static so battler swaps do not expose stale anim frames.
        StartSpriteAnim(&gSprites[data->monSpriteId], 0);
        gSprites[data->monSpriteId].animPaused = TRUE;
    }

    abilityName = gAbilitiesInfo[GetBattlerAbility(data->selectedBattler)].name;
    itemName = item == ITEM_NONE ? sTextNoData : GetItemName(item);

    PrintText(data->windowId, FONT_NORMAL, sTextAbility, 104, 40);
    PrintFittedText(data->windowId, FONT_SMALL, abilityName, 170, 42, 54);
    PrintText(data->windowId, FONT_NORMAL, sTextHeldItem, 104, 58);
    PrintFittedText(data->windowId, FONT_SMALL, itemName, 170, 60, 54);

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        s16 y = 78 + i * 13;
        u16 move = gBattleMons[data->selectedBattler].moves[i];

        if (move == MOVE_NONE)
        {
            PrintText(data->windowId, FONT_NORMAL, sTextDash, 8, y);
            continue;
        }

        PrintFittedText(data->windowId, FONT_NORMAL, GetMoveName(move), 8, y, 74);
        PrintText(data->windowId, FONT_SMALL, sTextPp, 84, y + 2);
        PrintRightAlignedNumber(data->windowId, FONT_SMALL, gBattleMons[data->selectedBattler].pp[i], 2, 110, y + 2);
        PrintText(data->windowId, FONT_SMALL, sTextSlash, 112, y + 2);
        PrintRightAlignedNumber(data->windowId, FONT_SMALL, CalculatePPWithBonus(move, gBattleMons[data->selectedBattler].ppBonuses, i), 2, 132, y + 2);
    }

    PrintText(data->windowId, FONT_SMALL, gStatNamesTable[STAT_ATK], 156, 78);
    GetStatStageText(statText, gBattleMons[data->selectedBattler].statStages[STAT_ATK]);
    PrintText(data->windowId, FONT_SMALL, statText, 220, 78);

    PrintText(data->windowId, FONT_SMALL, gStatNamesTable[STAT_DEF], 156, 91);
    GetStatStageText(statText, gBattleMons[data->selectedBattler].statStages[STAT_DEF]);
    PrintText(data->windowId, FONT_SMALL, statText, 220, 91);

    PrintText(data->windowId, FONT_SMALL, gStatNamesTable[STAT_SPATK], 156, 104);
    GetStatStageText(statText, gBattleMons[data->selectedBattler].statStages[STAT_SPATK]);
    PrintText(data->windowId, FONT_SMALL, statText, 220, 104);

    PrintText(data->windowId, FONT_SMALL, gStatNamesTable[STAT_SPDEF], 156, 117);
    GetStatStageText(statText, gBattleMons[data->selectedBattler].statStages[STAT_SPDEF]);
    PrintText(data->windowId, FONT_SMALL, statText, 220, 117);

    PrintText(data->windowId, FONT_SMALL, gStatNamesTable[STAT_SPEED], 156, 130);
    GetStatStageText(statText, gBattleMons[data->selectedBattler].statStages[STAT_SPEED]);
    PrintText(data->windowId, FONT_SMALL, statText, 220, 130);
}

static void DrawAiDamagePage(struct BattleInfoMenu *data)
{
    u8 attackers[MAX_BATTLERS_COUNT];
    u8 defenders[MAX_BATTLERS_COUNT];
    u8 attackerCount = GetLivingBattlersOnSide(B_SIDE_OPPONENT, attackers);
    u8 defenderCount = GetLivingBattlersOnSide(B_SIDE_PLAYER, defenders);

    PrintText(data->windowId, FONT_NORMAL, sTextAiDamageRoll, 6, 4);
    PrintText(data->windowId, FONT_SMALL, sTextAButtonNextPage, 6, 145);
    if (attackerCount == 0 || defenderCount == 0)
    {
        PrintText(data->windowId, FONT_NORMAL, sTextNoAiDamage, 24, 72);
        return;
    }

    if (attackerCount > 1 || defenderCount > 1)
        DrawAiDamageDoublesPage(data, attackers, attackerCount, defenders, defenderCount);
    else
        DrawAiDamageSinglesPage(data, attackers[0], defenders[0]);
}

static bool32 SelectAdjacentBattler(struct BattleInfoMenu *data, bool32 forward)
{
    u8 original = data->selectedBattler;
    u8 battler = data->selectedBattler;
    s32 i;

    for (i = 0; i < gBattlersCount; i++)
    {
        if (forward)
            battler = (battler + 1) % gBattlersCount;
        else
            battler = (battler + gBattlersCount - 1) % gBattlersCount;

        if (IsBattlerAlive(battler))
        {
            data->selectedBattler = battler;
            break;
        }
    }

    return data->selectedBattler != original;
}

static u8 GetFirstLivingBattlerOnSide(u32 side)
{
    u32 i;

    for (i = 0; i < gBattlersCount; i++)
    {
        if (GetBattlerSide(i) == side && IsBattlerAlive(i))
            return i;
    }

    return MAX_BATTLERS_COUNT;
}

static void PrintText(u8 windowId, u8 fontId, const u8 *text, s16 x, s16 y)
{
    AddTextPrinterParameterized(windowId, fontId, text, x, y, 0, NULL);
}

static void PrintFittedText(u8 windowId, u8 fontId, const u8 *text, s16 x, s16 y, u16 width)
{
    AddTextPrinterParameterized(windowId, GetFontIdToFit(text, fontId, 0, width), text, x, y, 0, NULL);
}

static void PrintRightAlignedNumber(u8 windowId, u8 fontId, s32 value, u8 digits, s16 rightX, s16 y)
{
    u8 text[8];
    s16 width;

    ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, digits);
    width = GetStringWidth(fontId, text, 0);
    AddTextPrinterParameterized(windowId, fontId, text, rightX - width, y, 0, NULL);
}

static void PrintCenteredText(u8 windowId, u8 fontId, const u8 *text, s16 centerX, s16 y)
{
    s16 width = GetStringWidth(fontId, text, 0);

    AddTextPrinterParameterized(windowId, fontId, text, centerX - width / 2, y, 0, NULL);
}

static const u8 *GetTerrainName(void)
{
    if (gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN)
        return sTextTerrainElectric;
    if (gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN)
        return sTextTerrainGrassy;
    if (gFieldStatuses & STATUS_FIELD_MISTY_TERRAIN)
        return sTextTerrainMisty;
    if (gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
        return sTextTerrainPsychic;

    return sTextDash;
}

static void GetStatStageText(u8 *dst, u8 statStage)
{
    if (statStage == DEFAULT_STAT_STAGE)
    {
        StringCopy(dst, sTextDash);
    }
    else if (statStage > DEFAULT_STAT_STAGE)
    {
        dst[0] = CHAR_PLUS;
        dst[1] = CHAR_0 + (statStage - DEFAULT_STAT_STAGE);
        dst[2] = EOS;
    }
    else
    {
        dst[0] = CHAR_HYPHEN;
        dst[1] = CHAR_0 + (DEFAULT_STAT_STAGE - statStage);
        dst[2] = EOS;
    }
}

static const u8 *GetBattlerDisplayName(u8 battler)
{
    return gBattleMons[battler].nickname;
}

static u16 GetMoveDamageRoll(u8 battlerAtk, u8 battlerDef, u16 move, u8 rollPercentage)
{
    struct DamageContext ctx = {0};
    enum Type oldDynamicMoveType;
    bool8 oldSwapDamageCategory;
    u8 oldMagnitudeBasePower;
    u8 oldPresentBasePower;
    s32 damage;
    s32 fixedDamage;

    if (move == MOVE_NONE || GetMovePower(move) == 0)
        return 0;

    if (GetMoveEffect(move) == EFFECT_NATURE_POWER)
        move = GetNaturePowerMove(battlerAtk);

    oldDynamicMoveType = gBattleStruct->dynamicMoveType;
    oldSwapDamageCategory = gBattleStruct->swapDamageCategory;
    oldMagnitudeBasePower = gBattleStruct->magnitudeBasePower;
    oldPresentBasePower = gBattleStruct->presentBasePower;

    SetDynamicMoveCategory(battlerAtk, battlerDef, move);
    SetTypeBeforeUsingMove(move, battlerAtk);

    // Stabilize damage for random-power moves on this read-only info screen.
    gBattleStruct->magnitudeBasePower = 70;
    gBattleStruct->presentBasePower = 80;

    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = ctx.chosenMove = move;
    ctx.moveType = GetBattleMoveType(move);
    ctx.isCrit = FALSE;
    ctx.randomFactor = FALSE;
    ctx.updateFlags = FALSE;
    ctx.weather = GetCurrentBattleWeather();
    ctx.fixedBasePower = 0;
    ctx.holdEffectAtk = GetBattlerHoldEffect(battlerAtk);
    ctx.holdEffectDef = GetBattlerHoldEffect(battlerDef);
    ctx.abilityAtk = GetBattlerAbility(battlerAtk);
    ctx.abilityDef = GetBattlerAbility(battlerDef);
    ctx.typeEffectivenessModifier = CalcTypeEffectivenessMultiplier(&ctx);

    fixedDamage = DoFixedDamageMoveCalc(&ctx);
    if (fixedDamage != INT32_MAX)
    {
        gBattleStruct->dynamicMoveType = oldDynamicMoveType;
        gBattleStruct->swapDamageCategory = oldSwapDamageCategory;
        gBattleStruct->magnitudeBasePower = oldMagnitudeBasePower;
        gBattleStruct->presentBasePower = oldPresentBasePower;
        return fixedDamage;
    }

    damage = CalculateMoveDamageVars(&ctx);
    if (damage <= 0)
    {
        gBattleStruct->dynamicMoveType = oldDynamicMoveType;
        gBattleStruct->swapDamageCategory = oldSwapDamageCategory;
        gBattleStruct->magnitudeBasePower = oldMagnitudeBasePower;
        gBattleStruct->presentBasePower = oldPresentBasePower;
        return 0;
    }

    damage *= rollPercentage;
    damage /= 100;
    damage = ApplyModifiersAfterDmgRoll(&ctx, damage);
    gBattleStruct->dynamicMoveType = oldDynamicMoveType;
    gBattleStruct->swapDamageCategory = oldSwapDamageCategory;
    gBattleStruct->magnitudeBasePower = oldMagnitudeBasePower;
    gBattleStruct->presentBasePower = oldPresentBasePower;
    if (damage == 0)
        damage = 1;
    return damage;
}

static u8 GetLivingBattlersOnSide(u32 side, u8 *battlers)
{
    u8 count = 0;

    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerSide(battler) == side && IsBattlerAlive(battler))
            battlers[count++] = battler;
    }

    return count;
}

static u8 CreateBattlerSpriteForInfo(u8 battler, s16 x, s16 y, u8 paletteSlot)
{
    struct Pokemon *mon = GetBattlerMon(battler);

    return CreateMonPicSprite(gBattleMons[battler].species,
                              GetMonData(mon, MON_DATA_IS_SHINY),
                              GetMonData(mon, MON_DATA_PERSONALITY),
                              TRUE,
                              x, y, paletteSlot, TAG_NONE);
}

static u8 CreateBattlerIconForInfo(struct BattleInfoMenu *data, u8 slot, u8 battler, s16 x, s16 y, u8 subpriority)
{
    struct Pokemon *mon = GetBattlerMon(battler);

    if (!data->iconPalettesLoaded)
    {
        LoadMonIconPalettes();
        data->iconPalettesLoaded = TRUE;
    }

    data->iconSpriteIds[slot] = CreateMonIcon(gBattleMons[battler].species,
                                              SpriteCallbackDummy,
                                              x, y,
                                              subpriority,
                                              GetMonData(mon, MON_DATA_PERSONALITY));
    gSprites[data->iconSpriteIds[slot]].oam.priority = 0;
    return data->iconSpriteIds[slot];
}

static void DrawAiDamageSinglesPage(struct BattleInfoMenu *data, u8 battlerAtk, u8 battlerDef)
{
    CreateBattlerIconForInfo(data, 0, battlerAtk, 22, 80, 0);
    CreateBattlerIconForInfo(data, 1, battlerDef, 146, 18, 0);

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        s16 y = 34 + i * 18;
        u16 move = gBattleMons[battlerAtk].moves[i];

        if (move == MOVE_NONE)
        {
            PrintText(data->windowId, FONT_NORMAL, sTextDash, 78, y);
            PrintText(data->windowId, FONT_NORMAL, sTextDash, 176, y);
            continue;
        }

        PrintFittedText(data->windowId, FONT_NORMAL, GetMoveName(move), 74, y, 94);
        if (GetMovePower(move) == 0)
            PrintText(data->windowId, FONT_NORMAL, sTextDash, 176, y);
        else
            PrintRightAlignedNumber(data->windowId, FONT_NORMAL, GetMoveDamageRoll(battlerAtk, battlerDef, move, AI_8TH_ROLL_PERCENTAGE), 4, 186, y);
    }
}

static void DrawAiDamageDoublesPage(struct BattleInfoMenu *data, const u8 *attackers, u8 attackerCount, const u8 *defenders, u8 defenderCount)
{
    const s16 defenderXs[] = {164, 214};
    const s16 attackerIconYs[] = {70, 124};
    const s16 rowStartYs[] = {30, 96};
    const s16 damageRightXs[] = {182, 230};

    for (u32 i = 0; i < defenderCount && i < ARRAY_COUNT(defenderXs); i++)
        CreateBattlerIconForInfo(data, i, defenders[i], defenderXs[i], 16, 0);

    for (u32 i = 0; i < attackerCount && i < ARRAY_COUNT(attackerIconYs); i++)
        CreateBattlerIconForInfo(data, i + 2, attackers[i], 18, attackerIconYs[i], 0);

    for (u32 attackerIndex = 0; attackerIndex < attackerCount && attackerIndex < ARRAY_COUNT(rowStartYs); attackerIndex++)
    {
        u8 battlerAtk = attackers[attackerIndex];

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            s16 y = rowStartYs[attackerIndex] + moveIndex * 16;
            u16 move = gBattleMons[battlerAtk].moves[moveIndex];

            if (move == MOVE_NONE)
            {
                PrintText(data->windowId, FONT_NORMAL, sTextDash, 56, y);
                for (u32 defenderIndex = 0; defenderIndex < defenderCount && defenderIndex < ARRAY_COUNT(damageRightXs); defenderIndex++)
                    PrintText(data->windowId, FONT_NORMAL, sTextDash, damageRightXs[defenderIndex] - 6, y);
                continue;
            }

            PrintFittedText(data->windowId, FONT_NORMAL, GetMoveName(move), 54, y, 88);
            for (u32 defenderIndex = 0; defenderIndex < defenderCount && defenderIndex < ARRAY_COUNT(damageRightXs); defenderIndex++)
            {
                u8 battlerDef = defenders[defenderIndex];

                if (GetMovePower(move) == 0)
                {
                    PrintText(data->windowId, FONT_NORMAL, sTextDash, damageRightXs[defenderIndex] - 6, y);
                }
                else
                {
                    PrintRightAlignedNumber(data->windowId,
                                            FONT_NORMAL,
                                            GetMoveDamageRoll(battlerAtk, battlerDef, move, AI_8TH_ROLL_PERCENTAGE),
                                            4,
                                            damageRightXs[defenderIndex],
                                            y);
                }
            }
        }
    }
}
