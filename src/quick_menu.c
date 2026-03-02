#include "global.h"
#include "bg.h"
#include "clock.h"
#include "event_data.h"
#include "fieldmap.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon_icon.h"
#include "region_map.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "wild_encounter.h"
#include "constants/rgb.h"
#include "constants/rtc.h"
#include "constants/songs.h"

#define QOL_TIME_OVERRIDE_NONE 0xFFFF

#define ENCOUNTER_VIEWER_MAX_SPECIES 12

enum EncounterViewerPage
{
    ENCOUNTER_VIEWER_PAGE_LAND,
    ENCOUNTER_VIEWER_PAGE_WATER,
    ENCOUNTER_VIEWER_PAGE_FISHING,
    ENCOUNTER_VIEWER_PAGE_COUNT,
};

enum EncounterViewerWindow
{
    ENCOUNTER_VIEWER_WINDOW_TITLE,
    ENCOUNTER_VIEWER_WINDOW_BODY,
};

struct EncounterViewer
{
    u8 currentPage;
    u8 counts[ENCOUNTER_VIEWER_PAGE_COUNT];
    u16 species[ENCOUNTER_VIEWER_PAGE_COUNT][ENCOUNTER_VIEWER_MAX_SPECIES];
    u8 iconSpriteIds[ENCOUNTER_VIEWER_MAX_SPECIES];
};

static EWRAM_DATA struct EncounterViewer *sEncounterViewer = NULL;

static void VBlankCB_EncounterViewer(void);
static void MainCB2_EncounterViewer(void);
static void CB2_EncounterViewer(void);
static void Task_EncounterViewerInput(u8 taskId);
static void EncounterViewer_InitWindows(void);
static void EncounterViewer_LoadData(void);
static void EncounterViewer_ClearIcons(void);
static void EncounterViewer_DrawPage(void);
static u8 EncounterViewer_GetFirstAvailablePage(void);
static u8 EncounterViewer_GetNextAvailablePage(s8 direction);
static void EncounterViewer_AddUniqueSpecies(u16 *speciesList, u8 *count, u16 species);
static void EncounterViewer_LoadSpeciesFromInfo(u16 *speciesList, u8 *count, const struct WildPokemonInfo *info, u8 numMons);
static const u8 *EncounterViewer_GetPageName(u8 page);

static const u16 sEncounterViewerBgColor[] = {RGB_WHITE};

static const struct BgTemplate sEncounterViewerBgTemplates[] =
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
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sEncounterViewerWindowTemplates[] =
{
    [ENCOUNTER_VIEWER_WINDOW_TITLE] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [ENCOUNTER_VIEWER_WINDOW_BODY] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 3,
        .width = 30,
        .height = 17,
        .paletteNum = 15,
        .baseBlock = 91,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sText_EncounterViewerTitle[] = _("Encounter Table");
static const u8 sText_EncounterViewerNoData[] = _("No encounters are available here.");
static const u8 sText_EncounterViewerControls[] = _("{LEFT_ARROW}{RIGHT_ARROW} Change Page  B Exit");

static const u8 sText_EncounterViewerLand[] = _("Land");
static const u8 sText_EncounterViewerWater[] = _("Water");
static const u8 sText_EncounterViewerFishing[] = _("Fishing");

u16 QuickMenu_ToggleInfiniteRepel(void)
{
    FlagToggle(OW_FLAG_NO_ENCOUNTER);

    if (FlagGet(OW_FLAG_NO_ENCOUNTER))
        PlaySE(SE_REPEL);
    else
        PlaySE(SE_PC_OFF);

    return FlagGet(OW_FLAG_NO_ENCOUNTER);
}

u16 QuickMenu_SetTimeOverride(void)
{
    u16 value = gSpecialVar_0x8004;
    struct TimeBlendSettings cachedBlend = gTimeBlend;
    u32 *oldBlend = (u32 *)&cachedBlend;
    u32 *newBlend = (u32 *)&gTimeBlend;

    if (value == QOL_TIME_OVERRIDE_NONE)
    {
        VarSet(VAR_QOL_TIME_OVERRIDE, 0);
        SetTimeOfDay(0);
    }
    else
    {
        VarSet(VAR_QOL_TIME_OVERRIDE, value + 1);
        SetTimeOfDay(value);
    }

    UpdateTimeOfDay();
    FormChangeTimeUpdate();

    if (MapHasNaturalLight(gMapHeader.mapType)
     && (oldBlend[0] != newBlend[0]
      || oldBlend[1] != newBlend[1]
      || oldBlend[2] != newBlend[2]))
    {
        ApplyWeatherColorMapIfIdle(gWeatherPtr->colorMapIndex);
    }

    return 0;
}

u16 QuickMenu_OpenEncounterViewer(void)
{
    if (sEncounterViewer != NULL)
        return 0;

    sEncounterViewer = AllocZeroed(sizeof(*sEncounterViewer));
    if (sEncounterViewer == NULL)
        return 0;

    CleanupOverworldWindowsAndTilemaps();
    gMain.state = 0;
    SetMainCallback2(CB2_EncounterViewer);
    return 0;
}

u16 QuickMenu_OpenFlyMap(void)
{
    FlagSet(OW_FLAG_POKE_RIDER);
    CleanupOverworldWindowsAndTilemaps();
    FieldInitRegionMap(CB2_ReturnToField);
    return 0;
}

static void VBlankCB_EncounterViewer(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void MainCB2_EncounterViewer(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void CB2_EncounterViewer(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sEncounterViewerBgTemplates, ARRAY_COUNT(sEncounterViewerBgTemplates));
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
        FreeSpriteTileRanges();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 3:
        LoadPalette(sEncounterViewerBgColor, 0, sizeof(sEncounterViewerBgColor));
        LoadPalette(GetOverworldTextboxPalettePtr(), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 4:
        EncounterViewer_InitWindows();
        LoadMonIconPalettes();
        EncounterViewer_LoadData();
        sEncounterViewer->currentPage = EncounterViewer_GetFirstAvailablePage();
        EncounterViewer_DrawPage();
        CreateTask(Task_EncounterViewerInput, 0);
        gMain.state++;
        break;
    default:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_EncounterViewer);
        SetMainCallback2(MainCB2_EncounterViewer);
        break;
    }
}

static void Task_EncounterViewerInput(u8 taskId)
{
    u8 page;

    if (gPaletteFade.active)
        return;

    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        EncounterViewer_ClearIcons();
        FreeMonIconPalettes();
        ClearStdWindowAndFrame(ENCOUNTER_VIEWER_WINDOW_TITLE, TRUE);
        ClearStdWindowAndFrame(ENCOUNTER_VIEWER_WINDOW_BODY, TRUE);
        RemoveWindow(ENCOUNTER_VIEWER_WINDOW_TITLE);
        RemoveWindow(ENCOUNTER_VIEWER_WINDOW_BODY);
        FreeAllWindowBuffers();
        FREE_AND_SET_NULL(sEncounterViewer);
        DestroyTask(taskId);
        SetVBlankCallback(NULL);
        SetMainCallback2(CB2_ReturnToField);
        return;
    }

    if (JOY_NEW(DPAD_LEFT) || JOY_NEW(L_BUTTON))
    {
        page = EncounterViewer_GetNextAvailablePage(-1);
        if (page != sEncounterViewer->currentPage)
        {
            sEncounterViewer->currentPage = page;
            PlaySE(SE_SELECT);
            EncounterViewer_DrawPage();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT) || JOY_NEW(R_BUTTON))
    {
        page = EncounterViewer_GetNextAvailablePage(1);
        if (page != sEncounterViewer->currentPage)
        {
            sEncounterViewer->currentPage = page;
            PlaySE(SE_SELECT);
            EncounterViewer_DrawPage();
        }
    }
}

static void EncounterViewer_InitWindows(void)
{
    InitWindows(sEncounterViewerWindowTemplates);
    DeactivateAllTextPrinters();
    LoadMessageBoxAndBorderGfx();

    DrawStdWindowFrame(ENCOUNTER_VIEWER_WINDOW_TITLE, FALSE);
    FillWindowPixelBuffer(ENCOUNTER_VIEWER_WINDOW_TITLE, PIXEL_FILL(1));
    PutWindowTilemap(ENCOUNTER_VIEWER_WINDOW_TITLE);

    DrawStdWindowFrame(ENCOUNTER_VIEWER_WINDOW_BODY, FALSE);
    FillWindowPixelBuffer(ENCOUNTER_VIEWER_WINDOW_BODY, PIXEL_FILL(1));
    PutWindowTilemap(ENCOUNTER_VIEWER_WINDOW_BODY);
}

static void EncounterViewer_LoadData(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;

    memset(sEncounterViewer->counts, 0, sizeof(sEncounterViewer->counts));
    memset(sEncounterViewer->species, 0, sizeof(sEncounterViewer->species));
    memset(sEncounterViewer->iconSpriteIds, MAX_SPRITES, sizeof(sEncounterViewer->iconSpriteIds));

    if (headerId == HEADER_NONE)
        return;

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);
    EncounterViewer_LoadSpeciesFromInfo(
        sEncounterViewer->species[ENCOUNTER_VIEWER_PAGE_LAND],
        &sEncounterViewer->counts[ENCOUNTER_VIEWER_PAGE_LAND],
        gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo,
        LAND_WILD_COUNT
    );

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);
    EncounterViewer_LoadSpeciesFromInfo(
        sEncounterViewer->species[ENCOUNTER_VIEWER_PAGE_WATER],
        &sEncounterViewer->counts[ENCOUNTER_VIEWER_PAGE_WATER],
        gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo,
        WATER_WILD_COUNT
    );
    EncounterViewer_LoadSpeciesFromInfo(
        sEncounterViewer->species[ENCOUNTER_VIEWER_PAGE_WATER],
        &sEncounterViewer->counts[ENCOUNTER_VIEWER_PAGE_WATER],
        gWildMonHeaders[headerId].encounterTypes[timeOfDay].rockSmashMonsInfo,
        ROCK_WILD_COUNT
    );

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_FISHING);
    EncounterViewer_LoadSpeciesFromInfo(
        sEncounterViewer->species[ENCOUNTER_VIEWER_PAGE_FISHING],
        &sEncounterViewer->counts[ENCOUNTER_VIEWER_PAGE_FISHING],
        gWildMonHeaders[headerId].encounterTypes[timeOfDay].fishingMonsInfo,
        FISH_WILD_COUNT
    );
}

static void EncounterViewer_ClearIcons(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sEncounterViewer->iconSpriteIds); i++)
    {
        if (sEncounterViewer->iconSpriteIds[i] != MAX_SPRITES)
        {
            FreeAndDestroyMonIconSprite(&gSprites[sEncounterViewer->iconSpriteIds[i]]);
            sEncounterViewer->iconSpriteIds[i] = MAX_SPRITES;
        }
    }
}

static void EncounterViewer_DrawPage(void)
{
    u32 i;
    u8 page = sEncounterViewer->currentPage;
    u8 count = sEncounterViewer->counts[page];
    s16 x;
    s16 y;

    EncounterViewer_ClearIcons();

    DrawStdWindowFrame(ENCOUNTER_VIEWER_WINDOW_TITLE, FALSE);
    FillWindowPixelBuffer(ENCOUNTER_VIEWER_WINDOW_TITLE, PIXEL_FILL(1));
    DrawStdWindowFrame(ENCOUNTER_VIEWER_WINDOW_BODY, FALSE);
    FillWindowPixelBuffer(ENCOUNTER_VIEWER_WINDOW_BODY, PIXEL_FILL(1));

    AddTextPrinterParameterized(
        ENCOUNTER_VIEWER_WINDOW_TITLE,
        FONT_NORMAL,
        sText_EncounterViewerTitle,
        8,
        1,
        TEXT_SKIP_DRAW,
        NULL
    );

    GetMapName(gStringVar4, GetCurrentRegionMapSectionId(), 0);
    AddTextPrinterParameterized(
        ENCOUNTER_VIEWER_WINDOW_TITLE,
        FONT_NORMAL,
        gStringVar4,
        GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 224),
        1,
        TEXT_SKIP_DRAW,
        NULL
    );

    AddTextPrinterParameterized(
        ENCOUNTER_VIEWER_WINDOW_BODY,
        FONT_NORMAL,
        EncounterViewer_GetPageName(page),
        8,
        2,
        TEXT_SKIP_DRAW,
        NULL
    );

    AddTextPrinterParameterized(
        ENCOUNTER_VIEWER_WINDOW_BODY,
        FONT_SMALL,
        sText_EncounterViewerControls,
        8,
        112,
        TEXT_SKIP_DRAW,
        NULL
    );

    if (count == 0)
    {
        AddTextPrinterParameterized(
            ENCOUNTER_VIEWER_WINDOW_BODY,
            FONT_NORMAL,
            sText_EncounterViewerNoData,
            8,
            40,
            TEXT_SKIP_DRAW,
            NULL
        );
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            x = 20 + (36 * (i % 6));
            y = 56 + (36 * (i / 6));
            sEncounterViewer->iconSpriteIds[i] = CreateMonIcon(
                sEncounterViewer->species[page][i],
                SpriteCB_MonIcon,
                x,
                y,
                0,
                0
            );
        }
    }

    CopyWindowToVram(ENCOUNTER_VIEWER_WINDOW_TITLE, COPYWIN_FULL);
    CopyWindowToVram(ENCOUNTER_VIEWER_WINDOW_BODY, COPYWIN_FULL);
}

static u8 EncounterViewer_GetFirstAvailablePage(void)
{
    u32 i;

    for (i = 0; i < ENCOUNTER_VIEWER_PAGE_COUNT; i++)
    {
        if (sEncounterViewer->counts[i] != 0)
            return i;
    }

    return ENCOUNTER_VIEWER_PAGE_LAND;
}

static u8 EncounterViewer_GetNextAvailablePage(s8 direction)
{
    s32 page = sEncounterViewer->currentPage;
    s32 i;

    for (i = 0; i < ENCOUNTER_VIEWER_PAGE_COUNT; i++)
    {
        page += direction;
        if (page < 0)
            page = ENCOUNTER_VIEWER_PAGE_COUNT - 1;
        else if (page >= ENCOUNTER_VIEWER_PAGE_COUNT)
            page = 0;

        if (sEncounterViewer->counts[page] != 0)
            return page;
    }

    return sEncounterViewer->currentPage;
}

static void EncounterViewer_AddUniqueSpecies(u16 *speciesList, u8 *count, u16 species)
{
    u32 i;

    if (species == SPECIES_NONE || *count >= ENCOUNTER_VIEWER_MAX_SPECIES)
        return;

    for (i = 0; i < *count; i++)
    {
        if (speciesList[i] == species)
            return;
    }

    speciesList[*count] = species;
    (*count)++;
}

static void EncounterViewer_LoadSpeciesFromInfo(u16 *speciesList, u8 *count, const struct WildPokemonInfo *info, u8 numMons)
{
    u32 i;

    if (info == NULL || info->encounterRate == 0)
        return;

    for (i = 0; i < numMons; i++)
        EncounterViewer_AddUniqueSpecies(speciesList, count, info->wildPokemon[i].species);
}

static const u8 *EncounterViewer_GetPageName(u8 page)
{
    switch (page)
    {
    case ENCOUNTER_VIEWER_PAGE_WATER:
        return sText_EncounterViewerWater;
    case ENCOUNTER_VIEWER_PAGE_FISHING:
        return sText_EncounterViewerFishing;
    case ENCOUNTER_VIEWER_PAGE_LAND:
    default:
        return sText_EncounterViewerLand;
    }
}
