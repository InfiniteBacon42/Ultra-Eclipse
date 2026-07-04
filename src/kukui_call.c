#include "global.h"
#include "bg.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "event_scripts.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "malloc.h"
#include "math_util.h"
#include "menu.h"
#include "naming_screen.h"
#include "overworld.h"
#include "palette.h"
#include "pokeball.h"
#include "random.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "string_util.h"
#include "international_string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "util.h"
#include "m4a.h"

static u8 sKukuiCallMainTaskId;

static void LoadMainMenuWindowFrameTiles(u8, u16);
static void AddComputerBackgroundObjects(u8);
static void Task_KukuiCall_GettingACall(u8);
static void Task_LaunchCall(u8);
static void Task_HeyThere(u8);
static void Task_AlolaIsARegion(u8);
static void Task_CoolPokemon(u8);
static void Task_AllOver(u8);
static void Task_LoveOurPokemon(u8);
static void Task_AndYouAre(u8);
static void Task_TestLoop(u8);

static void SpriteCB_Null(struct Sprite *sprite);
static void NewGameKukuiCall_PrintNameplate(void);

extern void FastUnsafeCopy32(void *dst, const void *src, u32 size);

extern const struct OamData gOamData_AffineDouble_ObjNormal_64x32;
extern const struct OamData gOamData_AffineOff_ObjNormal_64x64;
extern const struct OamData gOamData_AffineOff_ObjNormal_64x32;
extern const struct OamData gOamData_AffineOff_ObjNormal_32x32;
extern const struct OamData gOamData_AffineOff_ObjNormal_32x16;
extern const struct OamData gOamData_AffineOff_ObjNormal_16x16;
extern const struct OamData gOamData_AffineOff_ObjNormal_16x8;

// The Player Character's PC Wallpaper - BG Layer 3 (Bottom, Priority 3) of the Kukui Call scene
static const u32 sComputer_Background_Tiles[] = INCGFX_U32("graphics/kukui_call/computer_bg_tiles.png", ".4bpp.smol");
static const u32 sComputer_Background_Tilemap[] = INCGFX_U32("graphics/kukui_call/computer_bg_tiles.bin", ".smolTM");
static const u16 sComputer_Background_Pals[] = INCGFX_U16("graphics/kukui_call/computer_bg_tiles.png", ".gbapal");

// Kukui's Background in the video call window - BG Layer 2 (Lower, Priority 2) of the Kukui Call scene
// Background1 is visually askew, Background2 is visually upright
static const u32 sCall_Background1_Tiles[] = INCGFX_U32("graphics/kukui_call/call_bg1_tiles.png", ".4bpp.smol");
static const u32 sCall_Background1_Tilemap[] = INCGFX_U32("graphics/kukui_call/call_bg1_tiles.bin", ".smolTM");
static const u32 sCall_Background2_Tiles[] = INCGFX_U32("graphics/kukui_call/call_bg2_tiles.png", ".4bpp.smol");
static const u32 sCall_Background2_Tilemap[] = INCGFX_U32("graphics/kukui_call/call_bg2_tiles.bin", ".smolTM");
static const u16 sCall_Background_Pals[] = INCGFX_U16("graphics/kukui_call/call_bg1_tiles.png", ".gbapal");

// Kukui himself - BG Layer 1 (Higher, Priority 1) of the Kukui Call scene
// Kukui1 - Leaned back, visually askew
// Kukui2 - Leaned forward, grabbing camera, still visually askew
// Kukui3 - Leaned forward, grabbing camera, now visually upright
// Kukui4 - Leaned back, waving to camera (4a is shifted right)
// Kukui5 - Default pose (5a is shifted left)
// Kukui6 - Leaned to his left, gesturing to content to his right
// Kukui7 - Holding out pokeball to camera
// Kukui8 - Head tilted back, hand up as if tossing pokeball up out of frame
static const u32 sKukui1_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui1_tiles.png", ".4bpp.smol");
static const u32 sKukui1_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui1_tiles.bin", ".smolTM");
static const u32 sKukui2_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui2_tiles.png", ".4bpp.smol");
static const u32 sKukui2_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui2_tiles.bin", ".smolTM");
static const u32 sKukui3_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui3_tiles.png", ".4bpp.smol");
static const u32 sKukui3_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui3_tiles.bin", ".smolTM");
static const u32 sKukui4_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui4_tiles.png", ".4bpp.smol");
static const u32 sKukui4_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui4_tiles.bin", ".smolTM");
static const u32 sKukui4a_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui4a_tiles.bin", ".smolTM");
static const u32 sKukui5_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui5_tiles.png", ".4bpp.smol");
static const u32 sKukui5_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui5_tiles.bin", ".smolTM");
static const u32 sKukui5a_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui5a_tiles.bin", ".smolTM");
static const u32 sKukui6_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui6_tiles.png", ".4bpp.smol");
static const u32 sKukui6_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui6_tiles.bin", ".smolTM");
static const u32 sKukui7_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui7_tiles.png", ".4bpp.smol");
static const u32 sKukui7_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui7_tiles.bin", ".smolTM");
static const u32 sKukui8_Tiles[] = INCGFX_U32("graphics/kukui_call/kukui/kukui8_tiles.png", ".4bpp.smol");
static const u32 sKukui8_Tilemap[] = INCGFX_U32("graphics/kukui_call/kukui/kukui8_tiles.bin", ".smolTM");
static const u16 sKukui_Pals[] = INCGFX_U16("graphics/kukui_call/kukui/kukui1_tiles.png", ".gbapal");

// Kukui's Rockruff - BG Layer 0 (Top, Priority 0) of the Kukui Call scene
// (Rockruff_a is hFlipped and shifted slightly left, b is hFlipped and shifted more left)
static const u32 sRockruff_Tiles[] = INCGFX_U32("graphics/kukui_call/rockruff/rockruff_tiles.png", ".4bpp.smol");
static const u32 sRockruff_Tilemap[] = INCGFX_U32("graphics/kukui_call/rockruff/rockruff_tiles.bin", ".smolTM");
static const u32 sRockruff_a_Tilemap[] = INCGFX_U32("graphics/kukui_call/rockruff/rockruff_tiles_a.bin", ".smolTM");
static const u32 sRockruff_b_Tilemap[] = INCGFX_U32("graphics/kukui_call/rockruff/rockruff_tiles_b.bin", ".smolTM");
static const u16 sRockruff_Pals[] = INCGFX_U16("graphics/kukui_call/rockruff/rockruff_tiles.png", ".gbapal");

// Sprites for the video call's app window
// call_window_scalable - 64x32 - a 46x28 rectangle that can be scaled up and down and hue-shifted to simulate a computer window opening and closing
// call_window_corner - 64x64 - a north-west corner that can be rotated +90* for NE, SE, and SW
// call_window_edge - 64x32 - a north edge that can be vertically flipped
// call_window_ui - 64x32 - the window's top bar with minimize, maximize, and close buttons
static const u32 sCall_Window_Scalable_Gfx[] = INCGFX_U32("graphics/kukui_call/call_window/call_window_scalable.png", ".4bpp.smol");
static const u32 sCall_Window_Corner_Gfx[] = INCGFX_U32("graphics/kukui_call/call_window/call_window_corner.png", ".4bpp.smol");
static const u32 sCall_Window_Edge_Gfx[] = INCGFX_U32("graphics/kukui_call/call_window/call_window_edge.png", ".4bpp.smol");
static const u32 sCall_Window_UI_Gfx[] = INCGFX_U32("graphics/kukui_call/call_window/call_window_ui.png", ".4bpp.smol");
static const u16 sCall_Window_Pals[] = INCGFX_U16("graphics/kukui_call/call_window/call_window_corner.png", ".gbapal");

// Sprites for the desktop's apps
// settings_icon - 32x32 - settings app icon
// camera_icon - 32x32 - seems to be a photos folder icon but might just be a camera app icon
// notification_icon - 16x16 - a red dot to place on the upper right corner of the video call app icon while the call is active
// video icon - 4x2 spritesheet of 32x32 frames - video call app icon
  // offset 0 - default visual
  // offset 16 - "Ringing Frame 1" - the icon raises slightly with a glowing border
  // offset 32 - "Ringing Frame 2" - the icon stays raised, the border expands out and away like a wave
  // offset 48 - "Ringing Frame 3" - the icon lowers slightly, the wave expands fully and thins as if disappating
  // offset 64 - "Selecting Frame 1" - the icon becomes fully white
  // offset 80 - "Selecting Frame 2" - the icon becomes larger, begins fading back to normal
  // offset 96 - "Selecting Frame 3" - the icon size remains, continues fading back to normal
  // offset 112 - "Selecting Frame 4" - the icon becomes slightly smaller, continues fading back to normal
static const u32 sSettings_Icon_Gfx[] = INCGFX_U32("graphics/kukui_call/icons/settings_icon.png", ".4bpp.smol");
static const u16 sSettings_Icon_Pals[] = INCGFX_U16("graphics/kukui_call/icons/settings_icon.png", ".gbapal");
static const u32 sCamera_Icon_Gfx[] = INCGFX_U32("graphics/kukui_call/icons/camera_icon.png", ".4bpp.smol");
static const u16 sCamera_Icon_Pals[] = INCGFX_U16("graphics/kukui_call/icons/camera_icon.png", ".gbapal");
static const u32 sNotification_Icon_Gfx[] = INCGFX_U32("graphics/kukui_call/icons/notification_icon.png", ".4bpp.smol");
static const u16 sNotification_Icon_Pals[] = INCGFX_U16("graphics/kukui_call/icons/notification_icon.png", ".gbapal");
static const u32 sVideo_Icon_Gfx[] = INCGFX_U32("graphics/kukui_call/icons/video_icon.png", ".4bpp.smol", "-mwidth 4 -mheight 4");
static const u16 sVideo_Icon_Pals[] = INCGFX_U16("graphics/kukui_call/icons/video_icon.png", ".gbapal");

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sNewGameKukuiCallTextWindows[] =
{
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1
    },
    {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 13,
        .width = DLW_WIN_PLATE_SIZE,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x81,
    },
    DUMMY_WIN_TEMPLATE
};

static const u8 sTextColor_Headers[] = {TEXT_DYNAMIC_COLOR_1, TEXT_DYNAMIC_COLOR_2, TEXT_DYNAMIC_COLOR_3};
static const u8 sTextColor_MenuInfo[] = {TEXT_DYNAMIC_COLOR_1, TEXT_COLOR_WHITE, TEXT_DYNAMIC_COLOR_3};

static const u8 sTextColor_White[] = { 0, 1, 2, 0 };
static const u8 sTextColor_DarkGray[] = { 0, 2, 3, 0 };

static const struct ScrollArrowsTemplate sScrollArrowsTemplate_MainMenu = {2, 0x78, 8, 3, 0x78, 0x98, 3, 4, 1, 1, 0};

#define GFX_TAG_ICON_SETTINGS       0x1000
#define GFX_TAG_ICON_CAMERA         0x1001
#define GFX_TAG_ICON_NOTIFICATION   0x1002
#define GFX_TAG_ICON_VIDEO          0x1003
#define GFX_TAG_CALL_WINDOW_CORNER  0x1004
#define GFX_TAG_CALL_WINDOW_EDGE    0x1005
#define GFX_TAG_CALL_WINDOW_UI      0x1006
#define GFX_TAG_CALL_WINDOW_SCALABLE    0x1007

#define PAL_TAG_ICON_SETTINGS       0x1000
#define PAL_TAG_ICON_CAMERA         0x1001
#define PAL_TAG_ICON_NOTIFICATION   0x1002
#define PAL_TAG_ICON_VIDEO          0x1003
#define PAL_TAG_CALL_WINDOW         0x1004

static const struct CompressedSpriteSheet sSettings_Icon_SpriteSheet =
{
    .data = sSettings_Icon_Gfx,
    .size = 0x400,
    .tag = GFX_TAG_ICON_SETTINGS
};

static const struct CompressedSpriteSheet sCamera_Icon_SpriteSheet =
{
    .data = sCamera_Icon_Gfx,
    .size = 0x400,
    .tag = GFX_TAG_ICON_CAMERA
};

static const struct CompressedSpriteSheet sNotification_Icon_SpriteSheet =
{
    .data = sNotification_Icon_Gfx,
    .size = 0x100,
    .tag = GFX_TAG_ICON_NOTIFICATION
};

static const struct CompressedSpriteSheet sVideo_Icon_SpriteSheet =
{
    .data = sVideo_Icon_Gfx,
    .size = 0x400 * 8,
    .tag = GFX_TAG_ICON_VIDEO
};

static const struct CompressedSpriteSheet sCall_Window_Corner_SpriteSheet =
{
    .data = sCall_Window_Corner_Gfx,
    .size = 0x1000,
    .tag = GFX_TAG_CALL_WINDOW_CORNER
};

static const struct CompressedSpriteSheet sCall_Window_Edge_SpriteSheet =
{
    .data = sCall_Window_Edge_Gfx,
    .size = 0x1000,
    .tag = GFX_TAG_CALL_WINDOW_EDGE
};

static const struct CompressedSpriteSheet sCall_Window_UI_SpriteSheet =
{
    .data = sCall_Window_UI_Gfx,
    .size = 0x800,
    .tag = GFX_TAG_CALL_WINDOW_UI
};

static const struct CompressedSpriteSheet sCall_Window_Scalable_SpriteSheet =
{
    .data = sCall_Window_Scalable_Gfx,
    .size = 0x800,
    .tag = GFX_TAG_CALL_WINDOW_SCALABLE
};

static const struct SpriteTemplate sSettings_Icon_SpriteTemplate =
{
    .tileTag = GFX_TAG_ICON_SETTINGS,
    .paletteTag = PAL_TAG_ICON_SETTINGS,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .images = NULL, // sSettings_Icon_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpritePalette sSettings_Icon_SpritePalette =
{
    .data = sSettings_Icon_Pals,
    .tag = PAL_TAG_ICON_SETTINGS
};

static const struct SpriteTemplate sCamera_Icon_SpriteTemplate =
{
    .tileTag = GFX_TAG_ICON_CAMERA,
    .paletteTag = PAL_TAG_ICON_CAMERA,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .images = NULL, // sCamera_Icon_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpritePalette sCamera_Icon_SpritePalette =
{
    .data = sCamera_Icon_Pals,
    .tag = PAL_TAG_ICON_CAMERA
};

static const struct SpriteTemplate sNotification_Icon_SpriteTemplate =
{
    .tileTag = GFX_TAG_ICON_NOTIFICATION,
    .paletteTag = PAL_TAG_ICON_NOTIFICATION,
    .oam = &gOamData_AffineOff_ObjNormal_16x16,
    .images = NULL, // sNotification_Icon_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpritePalette sNotification_Icon_SpritePalette =
{
    .data = sNotification_Icon_Pals,
    .tag = PAL_TAG_ICON_NOTIFICATION
};

static const union AnimCmd sVideo_Icon_No_Anim[] =
{
    ANIMCMD_FRAME( 0, 0),
    ANIMCMD_END
};

static const union AnimCmd sVideo_Icon_Ringing_Anim[] =
{
    ANIMCMD_FRAME( 0, 0),
    ANIMCMD_FRAME(16, 6),
    ANIMCMD_FRAME(32, 3),
    ANIMCMD_FRAME(48, 6),
    ANIMCMD_FRAME( 0, 0),
    ANIMCMD_END
};

static const union AnimCmd sVideo_Icon_Selecting_Anim[] =
{
    ANIMCMD_FRAME( 0, 0),
    ANIMCMD_FRAME( 64,3),
    ANIMCMD_FRAME( 80,6),
    ANIMCMD_FRAME( 96,6),
    ANIMCMD_FRAME( 112,6),
    ANIMCMD_FRAME( 0,0),
    ANIMCMD_END
};

static const union AnimCmd *const sVideo_Icon_Anims[] =
{
    sVideo_Icon_No_Anim,
    sVideo_Icon_Ringing_Anim,
    sVideo_Icon_Selecting_Anim
};

static const struct SpriteFrameImage sVideo_Icon_PicTable[] =
{
    obj_frame_tiles(sVideo_Icon_Gfx)
};

static const struct SpriteTemplate sVideo_Icon_SpriteTemplate =
{
    .tileTag = GFX_TAG_ICON_VIDEO,
    .paletteTag = PAL_TAG_ICON_VIDEO,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .anims = sVideo_Icon_Anims,
    .images = sVideo_Icon_PicTable,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct SpritePalette sVideo_Icon_SpritePalette =
{
    .data = sVideo_Icon_Pals,
    .tag = PAL_TAG_ICON_VIDEO
};

static const struct SpriteTemplate sCall_Window_Corner_SpriteTemplate =
{
    .tileTag = GFX_TAG_CALL_WINDOW_CORNER,
    .paletteTag = PAL_TAG_CALL_WINDOW,
    .oam = &gOamData_AffineOff_ObjNormal_64x64,
    .images = NULL, // sCall_Window_Corner_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sCall_Window_Edge_SpriteTemplate =
{
    .tileTag = GFX_TAG_CALL_WINDOW_EDGE,
    .paletteTag = PAL_TAG_CALL_WINDOW,
    .oam = &gOamData_AffineOff_ObjNormal_64x32,
    .images = NULL, // sCall_Window_Edge_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sCall_Window_UI_SpriteTemplate =
{
    .tileTag = GFX_TAG_CALL_WINDOW_UI,
    .paletteTag = PAL_TAG_CALL_WINDOW,
    .oam = &gOamData_AffineOff_ObjNormal_64x32,
    .images = NULL, // sCall_Window_UI_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sCall_Window_Scalable_SpriteTemplate =
{
    .tileTag = GFX_TAG_CALL_WINDOW_SCALABLE,
    .paletteTag = PAL_TAG_CALL_WINDOW,
    .oam = &gOamData_AffineDouble_ObjNormal_64x32,
    .images = NULL, // sCall_Window_Scalable_Gfx,
    .callback = SpriteCallbackDummy
};

static const struct SpritePalette sCall_Window_SpritePalette =
{
    .data = sCall_Window_Pals,
    .tag = PAL_TAG_CALL_WINDOW
};

static const struct MenuAction sMenuActions_Gender[] = {
    {gText_Boy, {NULL}},
    {gText_Girl, {NULL}}
};

static const u8 *const sMalePresetNames[] = {
    COMPOUND_STRING("STU"),
    COMPOUND_STRING("MILTON"),
    COMPOUND_STRING("TOM"),
    COMPOUND_STRING("KENNY"),
    COMPOUND_STRING("REID"),
    COMPOUND_STRING("JUDE"),
    COMPOUND_STRING("JAXSON"),
    COMPOUND_STRING("EASTON"),
    COMPOUND_STRING("WALKER"),
    COMPOUND_STRING("TERU"),
    COMPOUND_STRING("JOHNNY"),
    COMPOUND_STRING("BRETT"),
    COMPOUND_STRING("SETH"),
    COMPOUND_STRING("TERRY"),
    COMPOUND_STRING("CASEY"),
    COMPOUND_STRING("DARREN"),
    COMPOUND_STRING("LANDON"),
    COMPOUND_STRING("COLLIN"),
    COMPOUND_STRING("STANLEY"),
    COMPOUND_STRING("QUINCY")
};

static const u8 *const sFemalePresetNames[] = {
    COMPOUND_STRING("KIMMY"),
    COMPOUND_STRING("TIARA"),
    COMPOUND_STRING("BELLA"),
    COMPOUND_STRING("JAYLA"),
    COMPOUND_STRING("ALLIE"),
    COMPOUND_STRING("LIANNA"),
    COMPOUND_STRING("SARA"),
    COMPOUND_STRING("MONICA"),
    COMPOUND_STRING("CAMILA"),
    COMPOUND_STRING("AUBREE"),
    COMPOUND_STRING("RUTHIE"),
    COMPOUND_STRING("HAZEL"),
    COMPOUND_STRING("NADINE"),
    COMPOUND_STRING("TANJA"),
    COMPOUND_STRING("YASMIN"),
    COMPOUND_STRING("NICOLA"),
    COMPOUND_STRING("LILLIE"),
    COMPOUND_STRING("TERRA"),
    COMPOUND_STRING("LUCY"),
    COMPOUND_STRING("HALIE")
};

// The number of male vs. female names is assumed to be the same.
// If they aren't, the smaller of the two sizes will be used and any extra names will be ignored.
#define NUM_PRESET_NAMES min(ARRAY_COUNT(sMalePresetNames), ARRAY_COUNT(sFemalePresetNames))

#define BIRCH_DLG_BASE_TILE_NUM 0x69

#define KUKUI_1_BASE_TILE_NUM   0x091 // 0xB5
#define KUKUI_2_BASE_TILE_NUM   0x146 // 0xB5
#define PC_BG_BASE_TILE_NUM     0x1FB // 0x141
#define CALL_BG_1_BASE_TILE_NUM 0x33C // 0x107
#define CALL_BG_2_BASE_TILE_NUM 0x443 // 0x107
#define BLANK_TILE_2            0x54A // 0x1

#define KUKUI_1_SCREEN_INDEX 29
#define KUKUI_2_SCREEN_INDEX 26
#define CALL_BG_1_SCREEN_INDEX 30
#define CALL_BG_2_SCREEN_INDEX 27

#define USED_KUKUI_BTN(freeTileNum) ((freeTileNum == KUKUI_1_BASE_TILE_NUM) ? KUKUI_2_BASE_TILE_NUM : KUKUI_1_BASE_TILE_NUM)
#define USED_CALL_BG_BTN(freeTileNum) ((freeTileNum == CALL_BG_1_BASE_TILE_NUM) ? CALL_BG_2_BASE_TILE_NUM : CALL_BG_1_BASE_TILE_NUM)
#define USED_KUKUI_SI(freeScreenIndex) ((freeScreenIndex == KUKUI_1_SCREEN_INDEX) ? KUKUI_2_SCREEN_INDEX : KUKUI_1_SCREEN_INDEX)
#define USED_CALL_BG_SI(freeScreenIndex) ((freeScreenIndex == CALL_BG_1_SCREEN_INDEX) ? CALL_BG_2_SCREEN_INDEX : CALL_BG_1_SCREEN_INDEX)

static void CB2_KukuiCall(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static EWRAM_DATA s16 sLastVCount;
static EWRAM_DATA bool8 sShouldLayerFade;
static EWRAM_DATA bool8 sLayerFadeActive;
static EWRAM_DATA u16 sLayerFadeTargetY, sLayerFadeY;
static EWRAM_DATA u8 sLayerFadeDelay, sLayerFadeDeltaY, sLayerFadeDelayCounter;
static EWRAM_DATA bool8 sLayerFadeDec;
static EWRAM_DATA bool8 sShouldUpdateLayerFade;

static bool8 IsLayerFadeActive()
{
    return sShouldLayerFade || sLayerFadeActive;
}

static void BeginLayerFace(u16 targets, s8 delay, u8 startY, u8 targetY)
{
    sLayerFadeDeltaY = 2;

    if (delay < 0)
    {
        sLayerFadeDeltaY += (delay * -1);
        delay = 0;
    }

    sLayerFadeDelay = delay * 2;
    sLayerFadeDelayCounter = delay * 2;
    sLayerFadeY = startY;
    sLayerFadeTargetY = targetY;

    if (startY < targetY)
        sLayerFadeDec = FALSE;
    else
        sLayerFadeDec = TRUE;

    SetGpuReg(REG_OFFSET_BLDCNT, targets | BLDCNT_EFFECT_BLEND);
    sLayerFadeActive = FALSE;
    sShouldLayerFade = TRUE;
}

static void UpdateLayerFade(void)
{
    if (sShouldLayerFade)
    {
        sLayerFadeActive = TRUE;
        sShouldLayerFade = FALSE;
    }

    if (!sLayerFadeActive)
        return;

    if (sLayerFadeDelayCounter < sLayerFadeDelay)
    {
        sLayerFadeDelayCounter++;
        return;
    }
    sLayerFadeDelayCounter = 0;

    if (sLayerFadeY == sLayerFadeTargetY)
    {
        sLayerFadeActive = FALSE;
    }
    else
    {
        s8 val;

        if (!sLayerFadeDec)
        {
            val = sLayerFadeY;
            val += sLayerFadeDeltaY;
            if (val > sLayerFadeTargetY)
                val = sLayerFadeTargetY;
            sLayerFadeY = val;
        }
        else
        {
            val = sLayerFadeY;
            val -= sLayerFadeDeltaY;
            if (val < sLayerFadeTargetY)
                val = sLayerFadeTargetY;
            sLayerFadeY = val;
        }
    }
}

static void UpdateLayerFadeRegs(void)
{
    REG_BLDALPHA = BLDALPHA_BLEND(16 - sLayerFadeY, sLayerFadeY);
}

static void VBlankCB_KukuiCall(void)
{
    sLastVCount = -1;

    UpdateLayerFade();

    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void HBlankCB_KukuiCall(void)
{
    u16 vCount = REG_VCOUNT;

    if (vCount >= 111)
    {
        sShouldUpdateLayerFade = TRUE;

        s16 blend = ((130 - vCount) * 16) / (130 - 111);
        if (blend < 0) blend = 0;
        REG_BLDALPHA = BLDALPHA_BLEND(16 - blend, blend);
    }
    else if (sShouldUpdateLayerFade)
    {
        sShouldUpdateLayerFade = FALSE;
        UpdateLayerFadeRegs();
    }
}

static void LoadMainMenuWindowFrameTiles(u8 bgId, u16 tileOffset)
{
    LoadBgTiles(bgId, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, tileOffset);
    LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
}

EWRAM_DATA static u8 sCallWindowCornerNWSpriteId, sCallWindowEdgeNSpriteId, sCallWindowCornerNESpriteId, sCallWindowCornerSESpriteId, sCallWindowEdgeSSpriteId, sCallWindowCornerSWSpriteId;

#define tTimer                      data[0]
#define tCount                      data[1]
#define tFreeKukuiBaseTileNum       data[2]
#define tFreeCallBgBaseTileNum      data[3]
#define tFreeKukuiScreenIndex       data[4]
#define tFreeCallBgScreenIndex      data[5]
#define tPlayerGender               data[6]
#define tSettingsIconSpriteId       data[7]
#define tCameraIconSpriteId         data[8]
#define tNotificationIconSpriteId   data[9]
#define tVideoIconSpriteId          data[10]
#define tCallWindowUISpriteId       data[11]
#define tCallWindowScalable0SpriteId data[12]
#define tCallWindowScalable1SpriteId data[13]
#define tCallWindowScalable2SpriteId data[14]
#define tCallWindowScalable3SpriteId data[15]

// For loading a new mapping for a pre-loaded tileset (such as shifting the image)
static void LoadTilemapAtOffset(u8 bgId, u16 tilesOffset, u8 charBaseIndex, const u32 *tilemap, u8 tilemapHeight, u16 screenIndex, u16 palOffset)
{
    DecompressDataWithHeaderVram(tilemap, (u8 *)(BG_SCREEN_ADDR(screenIndex)));

    for(u16 i = 0; i < (32 * tilemapHeight); i++)
    {
        u16 * tilemapPtr = (u16 *)(BG_SCREEN_ADDR(screenIndex));
        u16 tileIndex = tilemapPtr[i] & 0x03FF;
        tileIndex += tilesOffset - (charBaseIndex * 0x200);
        tilemapPtr[i] = (tilemapPtr[i] & 0x0C00) | tileIndex | (palOffset << 12);
    }
    
    SetBgAttribute(bgId, BG_ATTR_CHARBASEINDEX, charBaseIndex);
    SetBgAttribute(bgId, BG_ATTR_MAPBASEINDEX, screenIndex);
}

// For loading a new image with a pre-loaded palette (such as swapping between frames of the same animated subject)
static void LoadTilesAndMapAtOffset(u8 bgId, const u32 *tiles, u16 tilesOffset, u8 charBaseIndex, const u32 *tilemap, u8 tilemapHeight, u16 screenIndex, u16 palOffset)
{
    DecompressDataWithHeaderVram(tiles, (u8 *)VRAM + (TILE_SIZE_4BPP * tilesOffset));
    LoadTilemapAtOffset(bgId, tilesOffset, charBaseIndex, tilemap, tilemapHeight, screenIndex, palOffset);
}

// For loading a new image wholesale - new tiles near start of VRAM, new tilemap near end of VRAM, and a new palette
static void LoadTilesMapAndPalAtOffset(u8 bgId, const u32 *tiles, u16 tilesOffset, u8 charBaseIndex, const u32 *tilemap, u8 tilemapHeight, u16 screenIndex, const void* pal, u16 palOffset, bool8 unfade)
{
    LoadTilesAndMapAtOffset(bgId, tiles, tilesOffset, charBaseIndex, tilemap, tilemapHeight, screenIndex, palOffset);

    FastUnsafeCopy32(&gPlttBufferUnfaded[BG_PLTT_ID(palOffset)], pal, PLTT_SIZE_4BPP);

    if (unfade)
        FastUnsafeCopy32(&gPlttBufferFaded[BG_PLTT_ID(palOffset)], pal, PLTT_SIZE_4BPP);
}

static void CopyPartialTilemap(u32 dstScreen, u32 srcScreen, u8 startHeight, u8 rows)
{
    FastUnsafeCopy32((u8 *)(BG_SCREEN_ADDR(dstScreen) + (32 * startHeight * 2)), (u8 *)(BG_SCREEN_ADDR(srcScreen) + (32 * startHeight * 2)), rows * 32 * 2);
}

void CB2_NewGameKukuiCall_FromNewMainMenu(void)
{
    u8 taskId;
    u8 spriteId;
    u16 savedIme;

    ResetBgsAndClearDma3BusyFlags(0);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();

    LoadTilesMapAndPalAtOffset(0, sComputer_Background_Tiles, PC_BG_BASE_TILE_NUM, 0, sComputer_Background_Tilemap, 20, 31, sComputer_Background_Pals, 0, TRUE);

    ResetTasks();
    taskId = CreateTask(Task_KukuiCall_GettingACall, 0);
    gTasks[taskId].tFreeKukuiBaseTileNum = KUKUI_1_BASE_TILE_NUM;
    gTasks[taskId].tFreeCallBgBaseTileNum = CALL_BG_1_BASE_TILE_NUM;
    gTasks[taskId].tFreeKukuiScreenIndex = KUKUI_2_SCREEN_INDEX;
    gTasks[taskId].tFreeCallBgScreenIndex = CALL_BG_1_SCREEN_INDEX;
    gTasks[taskId].tTimer = 60 * 3;
    gTasks[taskId].tCount = 1;
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    // ResetAllPicSprites();
    AddComputerBackgroundObjects(taskId);
    // PlayBGM(MUS_ROUTE122);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_TGT2_BG3 | BLDCNT_EFFECT_BLEND);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
    SetGpuReg(REG_OFFSET_BLDY, 0);
    // ShowBg(0);
    ShowBg(1);
    // ShowBg(2);
    ShowBg(3);
    sLastVCount = -1;
    sLayerFadeActive = FALSE;
    sShouldLayerFade = FALSE; 
    sShouldUpdateLayerFade = TRUE;
    sLayerFadeY = 0;
    SetHBlankCallback(HBlankCB_KukuiCall);
    SetVBlankCallback(VBlankCB_KukuiCall);
    EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);
    SetMainCallback2(CB2_KukuiCall);
    InitWindows(sNewGameKukuiCallTextWindows);
    // LoadMessageBoxGfx(0, BIRCH_DLG_BASE_TILE_NUM, BG_PLTT_ID(15));
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void AddComputerBackgroundObjects(u8 taskId)
{
    u8 settingsIconSpriteId;
    u8 cameraIconSpriteId;
    u8 notificationIconSpriteId;
    u8 videoIconSpriteId;
    u8 callWindowUISpriteId;

    LoadCompressedSpriteSheet(&sSettings_Icon_SpriteSheet);
    LoadCompressedSpriteSheet(&sCamera_Icon_SpriteSheet);
    LoadCompressedSpriteSheet(&sNotification_Icon_SpriteSheet);
    u32 temp = LoadCompressedSpriteSheetByTemplate(&sVideo_Icon_SpriteTemplate, 0);
    
    LoadCompressedSpriteSheet(&sCall_Window_Corner_SpriteSheet);
    LoadCompressedSpriteSheet(&sCall_Window_Edge_SpriteSheet);
    LoadCompressedSpriteSheet(&sCall_Window_UI_SpriteSheet);
    LoadCompressedSpriteSheet(&sCall_Window_Scalable_SpriteSheet);

    LoadSpritePalette(&sSettings_Icon_SpritePalette);
    settingsIconSpriteId = CreateSprite(&sSettings_Icon_SpriteTemplate, 197 + 16, 41 + 16, 1);
    gSprites[settingsIconSpriteId].callback = SpriteCB_Null;
    gSprites[settingsIconSpriteId].oam.priority = 1;
    gSprites[settingsIconSpriteId].invisible = FALSE;
    gTasks[taskId].tSettingsIconSpriteId = settingsIconSpriteId;
    
    LoadSpritePalette(&sCamera_Icon_SpritePalette);
    cameraIconSpriteId = CreateSprite(&sCamera_Icon_SpriteTemplate, 197 + 16, 20 + 16, 1);
    gSprites[cameraIconSpriteId].callback = SpriteCB_Null;
    gSprites[cameraIconSpriteId].oam.priority = 1;
    gSprites[cameraIconSpriteId].invisible = FALSE;
    gTasks[taskId].tCameraIconSpriteId = cameraIconSpriteId;

    LoadSpritePalette(&sNotification_Icon_SpritePalette);
    notificationIconSpriteId = CreateSprite(&sNotification_Icon_SpriteTemplate, 214 + 8, 1 + 8, 1);
    gSprites[notificationIconSpriteId].callback = SpriteCB_Null;
    gSprites[notificationIconSpriteId].oam.priority = 0;
    gSprites[notificationIconSpriteId].invisible = TRUE;
    gTasks[taskId].tNotificationIconSpriteId = notificationIconSpriteId;

    LoadSpritePalette(&sVideo_Icon_SpritePalette);
    videoIconSpriteId = CreateSprite(&sVideo_Icon_SpriteTemplate, 197 + 16, -1 + 16, 1);
    gSprites[videoIconSpriteId].callback = SpriteCB_Null;
    gSprites[videoIconSpriteId].oam.priority = 1;
    gSprites[videoIconSpriteId].invisible = FALSE;
    gTasks[taskId].tVideoIconSpriteId = videoIconSpriteId;

    LoadSpritePalette(&sCall_Window_SpritePalette);
    
    sCallWindowCornerNWSpriteId = CreateSprite(&sCall_Window_Corner_SpriteTemplate, 32, 32, 1);
    gSprites[sCallWindowCornerNWSpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowCornerNWSpriteId].oam.priority = 1;
    gSprites[sCallWindowCornerNWSpriteId].invisible = TRUE;

    sCallWindowCornerNESpriteId = CreateSprite(&sCall_Window_Corner_SpriteTemplate, 160, 32, 1);
    gSprites[sCallWindowCornerNESpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowCornerNESpriteId].oam.priority = 1;
    SetSpriteOamFlipBits(&gSprites[sCallWindowCornerNESpriteId], TRUE, FALSE);
    gSprites[sCallWindowCornerNESpriteId].invisible = TRUE;

    sCallWindowCornerSESpriteId = CreateSprite(&sCall_Window_Corner_SpriteTemplate, 160, 88, 1);
    gSprites[sCallWindowCornerSESpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowCornerSESpriteId].oam.priority = 1;
    SetSpriteOamFlipBits(&gSprites[sCallWindowCornerSESpriteId], TRUE, TRUE);
    gSprites[sCallWindowCornerSESpriteId].invisible = TRUE;

    sCallWindowCornerSWSpriteId = CreateSprite(&sCall_Window_Corner_SpriteTemplate, 32, 88, 1);
    gSprites[sCallWindowCornerSWSpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowCornerSWSpriteId].oam.priority = 1;
    SetSpriteOamFlipBits(&gSprites[sCallWindowCornerSWSpriteId], FALSE, TRUE);
    gSprites[sCallWindowCornerSWSpriteId].invisible = TRUE;
    
    sCallWindowEdgeNSpriteId = CreateSprite(&sCall_Window_Edge_SpriteTemplate, 96, 16, 1);
    gSprites[sCallWindowEdgeNSpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowEdgeNSpriteId].oam.priority = 1;
    gSprites[sCallWindowEdgeNSpriteId].invisible = TRUE;

    sCallWindowEdgeSSpriteId = CreateSprite(&sCall_Window_Edge_SpriteTemplate, 96, 104, 1);
    gSprites[sCallWindowEdgeSSpriteId].callback = SpriteCB_Null;
    gSprites[sCallWindowEdgeSSpriteId].oam.priority = 1;
    SetSpriteOamFlipBits(&gSprites[sCallWindowEdgeSSpriteId], FALSE, TRUE);
    gSprites[sCallWindowEdgeSSpriteId].invisible = TRUE;
    
    callWindowUISpriteId = CreateSprite(&sCall_Window_UI_SpriteTemplate, 152, 16, 1);
    gSprites[callWindowUISpriteId].callback = SpriteCB_Null;
    gSprites[callWindowUISpriteId].oam.priority = 0;
    gSprites[callWindowUISpriteId].invisible = TRUE;
    gTasks[taskId].tCallWindowUISpriteId = callWindowUISpriteId;

    u32 matrixNum = AllocOamMatrix();
    gTasks[taskId].tCallWindowScalable0SpriteId = CreateSpriteAtEnd(&sCall_Window_Scalable_SpriteTemplate, 213, 15, 1);
    gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].callback = SpriteCB_Null;
    gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].oam.priority = 0;
    gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].oam.matrixNum = matrixNum;
    gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].invisible = TRUE;

    gTasks[taskId].tCallWindowScalable1SpriteId = CreateSpriteAtEnd(&sCall_Window_Scalable_SpriteTemplate, 213, 15, 1);
    gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].callback = SpriteCB_Null;
    gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].oam.priority = 0;
    gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].oam.matrixNum = matrixNum;
    gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].invisible = TRUE;

    gTasks[taskId].tCallWindowScalable2SpriteId = CreateSpriteAtEnd(&sCall_Window_Scalable_SpriteTemplate, 213, 15, 1);
    gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].callback = SpriteCB_Null;
    gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].oam.priority = 0;
    gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].oam.matrixNum = matrixNum;
    gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].invisible = TRUE;

    gTasks[taskId].tCallWindowScalable3SpriteId = CreateSpriteAtEnd(&sCall_Window_Scalable_SpriteTemplate, 213, 15, 1);
    gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].callback = SpriteCB_Null;
    gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].oam.priority = 0;
    gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].oam.matrixNum = matrixNum;
    gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].invisible = TRUE;

    SetOamMatrixRotationScaling(matrixNum, 10, 10, 0);

    #ifndef NDEBUG
        MgbaPrintf(MGBA_LOG_ERROR, "Sprite Ids: %d, %d, %d, %d, %d", notificationIconSpriteId, videoIconSpriteId, cameraIconSpriteId, settingsIconSpriteId, callWindowUISpriteId);
        MgbaPrintf(MGBA_LOG_ERROR, "TEMP: %d", temp);
    #endif
}

static void Task_KukuiCall_GettingACall(u8 taskId)
{
    #ifndef NDEBUG
        // MgbaPrintf(MGBA_LOG_ERROR, "Getting a Call, timer %d", gTasks[taskId].tTimer);
    #endif

    if (gTasks[taskId].tTimer)
    {
        if (gTasks[taskId].tTimer == 120 || gTasks[taskId].tTimer == 100 || gTasks[taskId].tTimer == 80)
        {
            StartSpriteAnim(&gSprites[gTasks[taskId].tVideoIconSpriteId], 1);
            PlaySE(SE_POKENAV_CALL);
        }
        if (gTasks[taskId].tTimer == 105 || gTasks[taskId].tTimer == 85 || gTasks[taskId].tTimer == 65)
        {
            m4aSongNumStop(SE_POKENAV_CALL);
        }
            
        gTasks[taskId].tTimer--;
    }
    else
    {
        if (gTasks[taskId].tCount == 2)
        {
            DrawDialogFrameWithCustomTile(0, TRUE, BIRCH_DLG_BASE_TILE_NUM);
            StringExpandPlaceholders(gStringVar4, gText_Kukui_YouHaveACall);
            AddTextPrinterForMessage(TRUE);
        }

        gTasks[taskId].tCount++;
        gTasks[taskId].tTimer = 120;
    }

    if (gTasks[taskId].tCount > 2)
    {
        if (!RunTextPrintersAndIsPrinter0Active())
        {
            gTasks[taskId].tTimer = 120;
            gTasks[taskId].tCount = 0;
            gTasks[taskId].func = Task_LaunchCall;
        }
    }
}


#define timerInitial 120
#define timerStartAnim (timerInitial - 30)
#define timerStartScaling (timerStartAnim - 15)
#define timerEndScaling (timerStartScaling - 20)
#define frameEndScaling (timerStartScaling - timerEndScaling)

// TODO - fade scalable sprites to white during their translation and transformation
static void Task_LaunchCall(u8 taskId)
{
    if (!gTasks[taskId].tCount)
    {
        RunTextPrinters();

        if (gTasks[taskId].tTimer == timerStartAnim)
        {
            StartSpriteAnim(&gSprites[gTasks[taskId].tVideoIconSpriteId], 2);
        }
        else if (gTasks[taskId].tTimer == timerStartScaling)
        {
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].invisible = FALSE;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].invisible = FALSE;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].invisible = FALSE;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].invisible = FALSE;
        }
        else if (gTasks[taskId].tTimer < timerStartScaling && gTasks[taskId].tTimer >= timerEndScaling)
        {
            u8 frame = frameEndScaling - (gTasks[taskId].tTimer - timerEndScaling);
            
            // Initial Size = ~0x
            // Actual Size = 2x = 92x56 (2x2 of 46x28 sprites)
            // Final Size = 4x = 184x112 (2x2 of 92x56 sprites)
            // linear size increase - scale is effectively finalScale * frame/frameEndScaling
            s16 scale = (frame * 0x200) / (frameEndScaling);
            SetOamMatrixRotationScaling(gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].oam.matrixNum, scale, scale, 0);

            // Initial Position = 213x15
            // Final Position = 96x52
            // currently linear translation - TODO - try exponential/parabolic decay?
            s16 cX = 208 + ((96 - 213) * frame / frameEndScaling); // x-coordinate of the collective sprite center
            s16 cY = 13 + ((52 - 15) * frame / frameEndScaling); // y-coordinate
            s16 sX = 23 * frame / frameEndScaling; // x-spacing of the individual sprite centers from the collective center
            s16 sY = 14 * frame / frameEndScaling; // y-spacing
            
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].x2 = cX;
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].y2 = cY;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].x2 = cX;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].y2 = cY;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].x2 = cX;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].y2 = cY;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].x2 = cX;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].y2 = cY;

            // truly not sure why the centers only need to shift a half-dimension NW, but 3 half-dimensions SE
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].x = -sX;
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].y = -sY;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].x = 3*sX;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].y = -sY;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].x = 3*sX;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].y = 3*sY;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].x = -sX;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].y = 3*sY;

            if (gTasks[taskId].tTimer == timerEndScaling)
            {
                gSprites[sCallWindowCornerNESpriteId].invisible = FALSE;
                gSprites[sCallWindowCornerNWSpriteId].invisible = FALSE;
                gSprites[sCallWindowCornerSWSpriteId].invisible = FALSE;
                gSprites[sCallWindowCornerSESpriteId].invisible = FALSE;
                gSprites[sCallWindowEdgeNSpriteId].invisible = FALSE;
                gSprites[sCallWindowEdgeSSpriteId].invisible = FALSE;
                gSprites[gTasks[taskId].tCallWindowUISpriteId].invisible = FALSE;
                gSprites[gTasks[taskId].tNotificationIconSpriteId].invisible = FALSE;
            }
        }
        else if (gTasks[taskId].tTimer == timerEndScaling - 1)
        {
            gSprites[gTasks[taskId].tCallWindowScalable0SpriteId].invisible = TRUE;
            gSprites[gTasks[taskId].tCallWindowScalable1SpriteId].invisible = TRUE;
            gSprites[gTasks[taskId].tCallWindowScalable2SpriteId].invisible = TRUE;
            gSprites[gTasks[taskId].tCallWindowScalable3SpriteId].invisible = TRUE;

            FillPalette(0xFFFF, BG_PLTT_ID(1), PLTT_SIZE_4BPP);
            FillPalette(0xFFFF, BG_PLTT_ID(2), PLTT_SIZE_4BPP);

            DmaFill16(3, BLANK_TILE_2 - 0x200, BG_SCREEN_ADDR(CALL_BG_1_SCREEN_INDEX), 0x800);
            DmaFill16(3, BLANK_TILE_2 - 0x200, BG_SCREEN_ADDR(CALL_BG_2_SCREEN_INDEX), 0x800);
            LoadTilesMapAndPalAtOffset(1, sKukui1_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui1_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, sKukui_Pals, 1, FALSE);
            LoadTilesMapAndPalAtOffset(2, sCall_Background1_Tiles, gTasks[taskId].tFreeCallBgBaseTileNum, 1, sCall_Background1_Tilemap, 14, gTasks[taskId].tFreeCallBgScreenIndex, sCall_Background_Pals, 2, FALSE);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);

            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeCallBgBaseTileNum = USED_CALL_BG_BTN(gTasks[taskId].tFreeCallBgBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);
            gTasks[taskId].tFreeCallBgScreenIndex = USED_CALL_BG_SI(gTasks[taskId].tFreeCallBgScreenIndex);

            gTasks[taskId].tCount = 1;
            gTasks[taskId].tTimer = 1;
        }

        RunTextPrinters();

        gTasks[taskId].tTimer--;
    }
    else
    {
        UpdatePaletteFade();

        if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
        {
            if (gTasks[taskId].tCount == 1)
            {
                ShowBg(1);
                ShowBg(2);
            }
            else if (gTasks[taskId].tCount == 2)
            {
                PlaySE(SE_POKENAV_ON);
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 0, 16, 14, RGB_WHITE);
            }
            else if (gTasks[taskId].tCount == 32)
            {
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 5, 14, 6, RGB_WHITE);
            }
            else if (gTasks[taskId].tCount == 37)
            {
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 3, 6, 10, RGB_WHITE);
            }
            else if (gTasks[taskId].tCount == 67)
            {
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 5, 10, 0, RGB_WHITE);
            }
            else if (gTasks[taskId].tCount == 127)
            {
                BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
            }
            else if (gTasks[taskId].tCount == 128)
            {
                LoadTilesAndMapAtOffset(1, sKukui2_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui2_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
                CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
                
                gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
                gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

                ShowBg(1);
                ShowBg(2);
            }
            else if (gTasks[taskId].tCount == 129)
            {
                BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
                PlayBGM(MUS_ROUTE122);

                StringExpandPlaceholders(gStringVar4, gText_Kukui_JustASec);
                AddTextPrinterForMessage(TRUE);
            }
            else if (gTasks[taskId].tCount == 189)
            {
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 0, 0, 16, RGB_WHITE);
            }
            else if (gTasks[taskId].tCount == 190)
            {
                LoadTilesAndMapAtOffset(1, sKukui3_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui3_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
                LoadTilesAndMapAtOffset(2, sCall_Background2_Tiles, gTasks[taskId].tFreeCallBgBaseTileNum, 1, sCall_Background2_Tilemap, 14, gTasks[taskId].tFreeCallBgScreenIndex, 2);
                CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);

                gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
                gTasks[taskId].tFreeCallBgBaseTileNum = USED_CALL_BG_BTN(gTasks[taskId].tFreeCallBgBaseTileNum);
                gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);
                gTasks[taskId].tFreeCallBgScreenIndex = USED_CALL_BG_SI(gTasks[taskId].tFreeCallBgScreenIndex);
                
                ShowBg(1);
                ShowBg(2);
            }
            else if (gTasks[taskId].tCount == 191)
            {
                gTasks[taskId].tTimer = 1;
                BeginNormalPaletteFade(1 << 1 | 1 << 2, 0, 16, 0, RGB_WHITE);
            }

            gTasks[taskId].tCount++;
        }

        if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
        {
            gTasks[taskId].tTimer = 0;
            gTasks[taskId].tCount = 0;
            gTasks[taskId].func = Task_HeyThere;
        }
    }
}

static void Task_HeyThere(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 30)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            LoadTilesAndMapAtOffset(1, sKukui4_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui4_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);

            StringExpandPlaceholders(gStringVar4, gText_Kukui_HeyThere);
            AddTextPrinterForMessage(TRUE);
        }
        else if (gTasks[taskId].tCount == 92)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 93)
        {
            LoadTilesAndMapAtOffset(1, sKukui5_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui5_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 94)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            gTasks[taskId].tTimer = 1;
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_AlolaIsARegion;
    }
}

static void Task_AlolaIsARegion(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 30)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            LoadTilesAndMapAtOffset(1, sKukui6_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui6_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);

            gTasks[taskId].tTimer = 1;
            StringExpandPlaceholders(gStringVar4, gText_Kukui_AlolaIsARegion);
            AddTextPrinterForMessage(TRUE);
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_CoolPokemon;
    }
}

static void Task_CoolPokemon(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 30)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            LoadTilesAndMapAtOffset(1, sKukui5_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui5_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);

            StringExpandPlaceholders(gStringVar4, gText_Kukui_CoolPokemon);
            AddTextPrinterForMessage(TRUE);
        }
        else if (gTasks[taskId].tCount == 92)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 93)
        {
            LoadTilesAndMapAtOffset(1, sKukui7_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui7_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 94)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            gTasks[taskId].tTimer = 1;
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_AllOver;
    }
}

static void Task_AllOver(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 30)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            LoadTilesAndMapAtOffset(1, sKukui8_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui8_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            PlaySE(SE_BALL_THROW);
        }
        else if (gTasks[taskId].tCount == 62)
        {
            PlaySE(SE_BALL_OPEN);
        }
        else if (gTasks[taskId].tCount == 87)
        {
            PlayCry_Normal(SPECIES_ROCKRUFF, 0);
        }
        else if (gTasks[taskId].tCount == 92)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 93)
        {
            DmaFill16(3, 0, BG_SCREEN_ADDR(gTasks[taskId].tFreeCallBgScreenIndex), 0x800);
            LoadTilesAndMapAtOffset(1, sKukui5_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui5a_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            LoadTilesMapAndPalAtOffset(0, sRockruff_Tiles, gTasks[taskId].tFreeCallBgBaseTileNum, 0, sRockruff_Tilemap, 14, gTasks[taskId].tFreeCallBgScreenIndex, sRockruff_Pals, 3, TRUE);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 94)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
        }
        else if (gTasks[taskId].tCount == 95)
        {
            BeginNormalPaletteFade(1 << 3, 0, 16, 0, RGB_RED);
            BeginLayerFace(BLDCNT_TGT1_BG0 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            StringExpandPlaceholders(gStringVar4, gText_Kukui_AllOver);
            AddTextPrinterForMessage(TRUE);
            gTasks[taskId].tTimer = 1;
            ShowBg(0);
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_LoveOurPokemon;
    }
}

static void Task_LoveOurPokemon(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 0)
        {
            StringExpandPlaceholders(gStringVar4, gText_Kukui_LoveOurPokemon);
            AddTextPrinterForMessage(TRUE);
        }
        else if (gTasks[taskId].tCount == 30)
        {
            BeginLayerFace(BLDCNT_TGT1_BG0 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            HideBg(0);
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            LoadTilesAndMapAtOffset(1, sKukui6_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui6_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            LoadTilemapAtOffset(0, gTasks[taskId].tFreeCallBgBaseTileNum, 0, sRockruff_a_Tilemap, 14, gTasks[taskId].tFreeCallBgScreenIndex, 3);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 33)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
        }
        else if (gTasks[taskId].tCount == 34)
        {
            BeginLayerFace(BLDCNT_TGT1_BG0 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            ShowBg(0);
            gTasks[taskId].tTimer = 1;
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
         gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_AndYouAre;
    }
}

static void Task_AndYouAre(u8 taskId)
{
    UpdatePaletteFade();

    if (!gTasks[taskId].tTimer && !gPaletteFade.active && !IsLayerFadeActive())
    {
        if (gTasks[taskId].tCount == 25)
        {
            PlayCry_Normal(SPECIES_ROCKRUFF, 0);
        }
        else if (gTasks[taskId].tCount == 30)
        {
            BeginNormalPaletteFade(1 << 3, 0, 0, 8, RGB_RED);
        }
        else if (gTasks[taskId].tCount == 31)
        {
            BeginNormalPaletteFade(1 << 3, 0, 9, 16, RGB_RED);
            BeginLayerFace(BLDCNT_TGT1_BG0 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 32)
        {
            HideBg(0);
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 33)
        {
            LoadTilesAndMapAtOffset(1, sKukui5_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui5_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 34)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
            StringExpandPlaceholders(gStringVar4, gText_Kukui_AndYouAre);
            AddTextPrinterForMessage(TRUE);
            gTasks[taskId].tTimer = 1;
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active() && gTasks[taskId].tTimer)
    {
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tCount = 0;
        gTasks[taskId].func = Task_TestLoop;
    }
}

static void Task_TestLoop(u8 taskId)
{
    UpdatePaletteFade();

    if (!gPaletteFade.active && !IsLayerFadeActive())
    {
        if (!gTasks[taskId].tCount)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 1)
        {
            if (!gTasks[taskId].tTimer)
            {
                LoadTilesAndMapAtOffset(1, sKukui5_Tiles, gTasks[taskId].tFreeKukuiBaseTileNum, 0, sKukui5_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
                gTasks[taskId].tFreeKukuiBaseTileNum = USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum);
                gTasks[taskId].tTimer = 1;
            }
            else
                LoadTilemapAtOffset(1, USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum), 0, sKukui5_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);
            
            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 2)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
        }
        else if (gTasks[taskId].tCount == 62)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 0, 16);
        }
        else if (gTasks[taskId].tCount == 63)
        {
            LoadTilemapAtOffset(1, USED_KUKUI_BTN(gTasks[taskId].tFreeKukuiBaseTileNum), 0, sKukui5a_Tilemap, 14, gTasks[taskId].tFreeKukuiScreenIndex, 1);
            CopyPartialTilemap(gTasks[taskId].tFreeKukuiScreenIndex, USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex), 14, 6);

            gTasks[taskId].tFreeKukuiScreenIndex = USED_KUKUI_SI(gTasks[taskId].tFreeKukuiScreenIndex);

            ShowBg(2);
            ShowBg(1);
        }
        else if (gTasks[taskId].tCount == 64)
        {
            BeginLayerFace(BLDCNT_TGT1_BG1 | BLDCNT_TGT2_BG_ALL, 0, 16, 0);
        }
        else if (gTasks[taskId].tCount == 124)
        {
            gTasks[taskId].tCount = -1;
        }

        gTasks[taskId].tCount++;
    }

    if (!RunTextPrintersAndIsPrinter0Active())
    {
        StringExpandPlaceholders(gStringVar4, gText_Kukui_JustASec);
        AddTextPrinterForMessage(TRUE);
    }
}

// static void Task_NewGameBirchSpeech_ThisIsAPokemon(u8 taskId)
// {
//     if (!gPaletteFade.active && !RunTextPrintersAndIsPrinter0Active())
//     {
//         gTasks[taskId].func = Task_NewGameBirchSpeech_MainSpeech;
//         StringExpandPlaceholders(gStringVar4, gText_ThisIsAPokemon);
//         AddTextPrinterWithCallbackForMessage(TRUE, NewGameBirchSpeech_WaitForThisIsPokemonText);
//         sKukuiCallMainTaskId = taskId;
//     }
// }

// static void Task_NewGameBirchSpeech_MainSpeech(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//     {
//         StringExpandPlaceholders(gStringVar4, gText_Birch_MainSpeech);
//         NewGameBirchSpeech_PrintDialogue();
//         gTasks[taskId].func = Task_NewGameBirchSpeech_AndYouAre;
//     }
// }

// #define tState data[0]

// static void Task_NewGameBirchSpeechSub_InitPokeBall(u8 taskId)
// {
//     u8 spriteId = gTasks[sKukuiCallMainTaskId].tLotadSpriteId;

//     gSprites[spriteId].x = 100;
//     gSprites[spriteId].y = 75;
//     gSprites[spriteId].invisible = FALSE;
//     gSprites[spriteId].data[0] = 0;

//     CreatePokeballSpriteToReleaseMon(spriteId, gSprites[spriteId].oam.paletteNum, 138, 52, 0, 0, 32, PALETTES_BG, SPECIES_ROCKRUFF);
//     gTasks[taskId].func = Task_NewGameBirchSpeechSub_WaitForLotad;
//     gTasks[sKukuiCallMainTaskId].tTimer = 0;
// }

// static void Task_NewGameBirchSpeechSub_WaitForLotad(u8 taskId)
// {
//     s16 *data = gTasks[taskId].data;
//     struct Sprite *sprite = &gSprites[gTasks[sKukuiCallMainTaskId].tLotadSpriteId];

//     switch (tState)
//     {
//     case 0:
//         if (sprite->callback != SpriteCallbackDummy)
//             return;
//         sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
//         break;
//     case 1:
//         if (gTasks[sKukuiCallMainTaskId].tTimer >= 96)
//         {
//             DestroyTask(taskId);
//             if (gTasks[sKukuiCallMainTaskId].tTimer < 0x4000)
//                 gTasks[sKukuiCallMainTaskId].tTimer++;
//         }
//         return;
//     }
//     tState++;
//     if (gTasks[sKukuiCallMainTaskId].tTimer < 0x4000)
//         gTasks[sKukuiCallMainTaskId].tTimer++;
// }

// #undef tState

// static void Task_NewGameBirchSpeech_AndYouAre(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//     {
//         sStartedPokeBallTask = FALSE;
//         StringExpandPlaceholders(gStringVar4, gText_Birch_AndYouAre);
//         NewGameBirchSpeech_PrintDialogue();
//         gTasks[taskId].func = Task_NewGameBirchSpeech_StartBirchLotadPlatformFade;
//     }
// }

// static void Task_NewGameBirchSpeech_StartBirchLotadPlatformFade(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//     {
//         gSprites[gTasks[taskId].tBirchSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         gSprites[gTasks[taskId].tLotadSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         NewGameBirchSpeech_StartFadeOutTarget1InTarget2(taskId, 2);
//         NewGameBirchSpeech_StartFadePlatformIn(taskId, 1);
//         gTasks[taskId].tTimer = 64;
//         gTasks[taskId].func = Task_NewGameBirchSpeech_SlidePlatformAway;
//     }
// }

// static void Task_NewGameBirchSpeech_SlidePlatformAway(u8 taskId)
// {
//     if (gTasks[taskId].tBG1HOFS != -60)
//     {
//         gTasks[taskId].tBG1HOFS -= 2;
//         SetGpuReg(REG_OFFSET_BG1HOFS, gTasks[taskId].tBG1HOFS);
//     }
//     else
//     {
//         gTasks[taskId].tBG1HOFS = -60;
//         gTasks[taskId].func = Task_NewGameBirchSpeech_StartPlayerFadeIn;
//     }
// }

// static void Task_NewGameBirchSpeech_StartPlayerFadeIn(u8 taskId)
// {
//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tBirchSpriteId].invisible = TRUE;
//         gSprites[gTasks[taskId].tLotadSpriteId].invisible = TRUE;
//         if (gTasks[taskId].tTimer)
//         {
//             gTasks[taskId].tTimer--;
//         }
//         else
//         {
//             u8 spriteId = gTasks[taskId].tBrendanSpriteId;

//             gSprites[spriteId].x = 180;
//             gSprites[spriteId].y = 60;
//             gSprites[spriteId].invisible = FALSE;
//             gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//             gTasks[taskId].tPlayerSpriteId = spriteId;
//             gTasks[taskId].tPlayerGender = MALE;
//             NewGameBirchSpeech_StartFadeInTarget1OutTarget2(taskId, 2);
//             NewGameBirchSpeech_StartFadePlatformOut(taskId, 1);
//             gTasks[taskId].func = Task_NewGameBirchSpeech_WaitForPlayerFadeIn;
//         }
//     }
// }

// static void Task_NewGameBirchSpeech_WaitForPlayerFadeIn(u8 taskId)
// {
//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
//         gTasks[taskId].func = Task_NewGameBirchSpeech_BoyOrGirl;
//     }
// }

// static void Task_NewGameBirchSpeech_BoyOrGirl(u8 taskId)
// {
//     NewGameBirchSpeech_ClearWindow(0);
//     StringExpandPlaceholders(gStringVar4, gText_Birch_BoyOrGirl);
//     NewGameBirchSpeech_PrintDialogue();
//     gTasks[taskId].func = Task_NewGameBirchSpeech_WaitToShowGenderMenu;
// }

// static void Task_NewGameBirchSpeech_WaitToShowGenderMenu(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//     {
//         NewGameBirchSpeech_ShowGenderMenu();
//         gTasks[taskId].func = Task_NewGameBirchSpeech_ChooseGender;
//     }
// }

// static void Task_NewGameBirchSpeech_ChooseGender(u8 taskId)
// {
//     enum Gender gender = NewGameBirchSpeech_ProcessGenderMenuInput();
//     enum Gender gender2;

//     switch (gender)
//     {
//     case MALE:
//         PlaySE(SE_SELECT);
//         gSaveBlock2Ptr->playerGender = gender;
//         NewGameBirchSpeech_ClearGenderWindow(1, 1);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_WhatsYourName;
//         break;
//     case FEMALE:
//         PlaySE(SE_SELECT);
//         gSaveBlock2Ptr->playerGender = gender;
//         NewGameBirchSpeech_ClearGenderWindow(1, 1);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_WhatsYourName;
//         break;
//     default: //repeat task if nothing is selected
//         break;
//     }
//     gender2 = Menu_GetCursorPos();
//     if (gender2 != gTasks[taskId].tPlayerGender)
//     {
//         gTasks[taskId].tPlayerGender = gender2;
//         gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         NewGameBirchSpeech_StartFadeOutTarget1InTarget2(taskId, 0);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_SlideOutOldGenderSprite;
//     }
// }

// static void Task_NewGameBirchSpeech_SlideOutOldGenderSprite(u8 taskId)
// {
//     u8 spriteId = gTasks[taskId].tPlayerSpriteId;
//     if (gTasks[taskId].tIsDoneFadingSprites == 0)
//     {
//         gSprites[spriteId].x += 4;
//     }
//     else
//     {
//         gSprites[spriteId].invisible = TRUE;
//         if (gTasks[taskId].tPlayerGender != MALE)
//             spriteId = gTasks[taskId].tMaySpriteId;
//         else
//             spriteId = gTasks[taskId].tBrendanSpriteId;
//         gSprites[spriteId].x = DISPLAY_WIDTH;
//         gSprites[spriteId].y = 60;
//         gSprites[spriteId].invisible = FALSE;
//         gTasks[taskId].tPlayerSpriteId = spriteId;
//         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         NewGameBirchSpeech_StartFadeInTarget1OutTarget2(taskId, 0);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_SlideInNewGenderSprite;
//     }
// }

// static void Task_NewGameBirchSpeech_SlideInNewGenderSprite(u8 taskId)
// {
//     u8 spriteId = gTasks[taskId].tPlayerSpriteId;

//     if (gSprites[spriteId].x > 180)
//     {
//         gSprites[spriteId].x -= 4;
//     }
//     else
//     {
//         gSprites[spriteId].x = 180;
//         if (gTasks[taskId].tIsDoneFadingSprites)
//         {
//             gSprites[spriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
//             gTasks[taskId].func = Task_NewGameBirchSpeech_ChooseGender;
//         }
//     }
// }

// static void Task_NewGameBirchSpeech_WhatsYourName(u8 taskId)
// {
//     NewGameBirchSpeech_ClearWindow(0);
//     StringExpandPlaceholders(gStringVar4, gText_Birch_WhatsYourName);
//     NewGameBirchSpeech_PrintDialogue();
//     gTasks[taskId].func = Task_NewGameBirchSpeech_WaitForWhatsYourNameToPrint;
// }

// static void Task_NewGameBirchSpeech_WaitForWhatsYourNameToPrint(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//         gTasks[taskId].func = Task_NewGameBirchSpeech_WaitPressBeforeNameChoice;
// }

// static void Task_NewGameBirchSpeech_WaitPressBeforeNameChoice(u8 taskId)
// {
//     if ((JOY_NEW(A_BUTTON)) || (JOY_NEW(B_BUTTON)))
//     {
//         BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_StartNamingScreen;
//     }
// }

// static void Task_NewGameBirchSpeech_StartNamingScreen(u8 taskId)
// {
//     if (!gPaletteFade.active)
//     {
//         FreeAllWindowBuffers();
//         FreeAndDestroyMonPicSprite(gTasks[taskId].tLotadSpriteId);
//         NewGameBirchSpeech_SetDefaultPlayerName(Random() % NUM_PRESET_NAMES);
//         DestroyTask(taskId);
//         DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName, gSaveBlock2Ptr->playerGender, 0, 0, CB2_NewGameBirchSpeech_ReturnFromNamingScreen);
//     }
// }

// static void Task_NewGameBirchSpeech_SoItsPlayerName(u8 taskId)
// {
//     NewGameBirchSpeech_ClearWindow(0);
//     StringExpandPlaceholders(gStringVar4, gText_Birch_SoItsPlayer);
//     NewGameBirchSpeech_PrintDialogue();
//     gTasks[taskId].func = Task_NewGameBirchSpeech_CreateNameYesNo;
// }

// static void Task_NewGameBirchSpeech_CreateNameYesNo(u8 taskId)
// {
//     if (!RunTextPrintersAndIsPrinter0Active())
//     {
//         CreateYesNoMenuParameterized(2, 1, 0xF3, 0xDF, 2, 15);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_ProcessNameYesNoMenu;
//     }
// }

// static void Task_NewGameBirchSpeech_ProcessNameYesNoMenu(u8 taskId)
// {
//     switch (Menu_ProcessInputNoWrapClearOnChoose())
//     {
//     case 0:
//         PlaySE(SE_SELECT);
//         gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         NewGameBirchSpeech_StartFadeOutTarget1InTarget2(taskId, 2);
//         NewGameBirchSpeech_StartFadePlatformIn(taskId, 1);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_SlidePlatformAway2;
//         break;
//     case MENU_B_PRESSED:
//     case 1:
//         PlaySE(SE_SELECT);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_BoyOrGirl;
//     }
// }

// static void Task_NewGameBirchSpeech_SlidePlatformAway2(u8 taskId)
// {
//     if (gTasks[taskId].tBG1HOFS)
//     {
//         gTasks[taskId].tBG1HOFS += 2;
//         SetGpuReg(REG_OFFSET_BG1HOFS, gTasks[taskId].tBG1HOFS);
//     }
//     else
//     {
//         gTasks[taskId].func = Task_NewGameBirchSpeech_ReshowBirchLotad;
//     }
// }

// static void Task_NewGameBirchSpeech_ReshowBirchLotad(u8 taskId)
// {
//     u8 spriteId;

//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tBrendanSpriteId].invisible = TRUE;
//         gSprites[gTasks[taskId].tMaySpriteId].invisible = TRUE;
//         spriteId = gTasks[taskId].tBirchSpriteId;
//         gSprites[spriteId].x = 136;
//         gSprites[spriteId].y = 60;
//         gSprites[spriteId].invisible = FALSE;
//         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         spriteId = gTasks[taskId].tLotadSpriteId;
//         gSprites[spriteId].x = 100;
//         gSprites[spriteId].y = 75;
//         gSprites[spriteId].invisible = FALSE;
//         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         NewGameBirchSpeech_StartFadeInTarget1OutTarget2(taskId, 2);
//         NewGameBirchSpeech_StartFadePlatformOut(taskId, 1);
//         NewGameBirchSpeech_ClearWindow(0);
//         StringExpandPlaceholders(gStringVar4, gText_Birch_YourePlayer);
//         NewGameBirchSpeech_PrintDialogue();
//         gTasks[taskId].func = Task_NewGameBirchSpeech_WaitForSpriteFadeInAndTextPrinter;
//     }
// }

// static void Task_NewGameBirchSpeech_WaitForSpriteFadeInAndTextPrinter(u8 taskId)
// {
//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tBirchSpriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
//         gSprites[gTasks[taskId].tLotadSpriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
//         if (!RunTextPrintersAndIsPrinter0Active())
//         {
//             gSprites[gTasks[taskId].tBirchSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//             gSprites[gTasks[taskId].tLotadSpriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//             NewGameBirchSpeech_StartFadeOutTarget1InTarget2(taskId, 2);
//             NewGameBirchSpeech_StartFadePlatformIn(taskId, 1);
//             gTasks[taskId].tTimer = 64;
//             gTasks[taskId].func = Task_NewGameBirchSpeech_AreYouReady;
//         }
//     }
// }

// static void Task_NewGameBirchSpeech_AreYouReady(u8 taskId)
// {
//     u8 spriteId;

//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tBirchSpriteId].invisible = TRUE;
//         gSprites[gTasks[taskId].tLotadSpriteId].invisible = TRUE;
//         if (gTasks[taskId].tTimer)
//         {
//             gTasks[taskId].tTimer--;
//             return;
//         }
//         if (gSaveBlock2Ptr->playerGender != MALE)
//             spriteId = gTasks[taskId].tMaySpriteId;
//         else
//             spriteId = gTasks[taskId].tBrendanSpriteId;
//         gSprites[spriteId].x = 120;
//         gSprites[spriteId].y = 60;
//         gSprites[spriteId].invisible = FALSE;
//         gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
//         gTasks[taskId].tPlayerSpriteId = spriteId;
//         NewGameBirchSpeech_StartFadeInTarget1OutTarget2(taskId, 2);
//         NewGameBirchSpeech_StartFadePlatformOut(taskId, 1);
//         StringExpandPlaceholders(gStringVar4, gText_Birch_AreYouReady);
//         NewGameBirchSpeech_PrintDialogue();
//         gTasks[taskId].func = Task_NewGameBirchSpeech_ShrinkPlayer;
//     }
// }

// static void Task_NewGameBirchSpeech_ShrinkPlayer(u8 taskId)
// {
//     u8 spriteId;

//     if (gTasks[taskId].tIsDoneFadingSprites)
//     {
//         gSprites[gTasks[taskId].tPlayerSpriteId].oam.objMode = ST_OAM_OBJ_NORMAL;
//         if (!RunTextPrintersAndIsPrinter0Active())
//         {
//             spriteId = gTasks[taskId].tPlayerSpriteId;
//             gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
//             gSprites[spriteId].affineAnims = sSpriteAffineAnimTable_PlayerShrink;
//             InitSpriteAffineAnim(&gSprites[spriteId]);
//             StartSpriteAffineAnim(&gSprites[spriteId], 0);
//             gSprites[spriteId].callback = SpriteCB_MovePlayerDownWhileShrinking;
//             BeginNormalPaletteFade(PALETTES_BG, 0, 0, 16, RGB_BLACK);
//             FadeOutBGM(4);
//             gTasks[taskId].func = Task_NewGameBirchSpeech_WaitForPlayerShrink;
//         }
//     }
// }

// static void Task_NewGameBirchSpeech_WaitForPlayerShrink(u8 taskId)
// {
//     u8 spriteId = gTasks[taskId].tPlayerSpriteId;

//     if (gSprites[spriteId].affineAnimEnded)
//         gTasks[taskId].func = Task_NewGameBirchSpeech_FadePlayerToWhite;
// }

// static void Task_NewGameBirchSpeech_FadePlayerToWhite(u8 taskId)
// {
//     u8 spriteId;

//     if (!gPaletteFade.active)
//     {
//         spriteId = gTasks[taskId].tPlayerSpriteId;
//         gSprites[spriteId].callback = SpriteCB_Null;
//         SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
//         BeginNormalPaletteFade(PALETTES_OBJECTS, 0, 0, 16, RGB_WHITEALPHA);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_Cleanup;
//     }
// }

// static void Task_NewGameBirchSpeech_Cleanup(u8 taskId)
// {
//     if (!gPaletteFade.active)
//     {
//         FreeAllWindowBuffers();
//         FreeAndDestroyMonPicSprite(gTasks[taskId].tLotadSpriteId);
//         ResetAllPicSprites();
//         SetMainCallback2(CB2_NewGame); // comment this out and uncomment the two lines below for a clock at newgame
//         //SetMainCallback2(CB2_StartWallClock);
//         //gMain.savedCallback = CB2_NewGame;
//         DestroyTask(taskId);
//     }
// }

// static void CB2_NewGameBirchSpeech_ReturnFromNamingScreen(void)
// {
//     u8 taskId;
//     u8 spriteId;
//     u16 savedIme;

//     ResetBgsAndClearDma3BusyFlags(0);
//     SetGpuReg(REG_OFFSET_DISPCNT, 0);
//     SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
//     InitBgsFromTemplates(0, sMainMenuBgTemplates, ARRAY_COUNT(sMainMenuBgTemplates));
//     InitBgFromTemplate(&sBirchBgTemplate);
//     SetVBlankCallback(NULL);
//     SetGpuReg(REG_OFFSET_BG2CNT, 0);
//     SetGpuReg(REG_OFFSET_BG1CNT, 0);
//     SetGpuReg(REG_OFFSET_BG0CNT, 0);
//     SetGpuReg(REG_OFFSET_BG2HOFS, 0);
//     SetGpuReg(REG_OFFSET_BG2VOFS, 0);
//     SetGpuReg(REG_OFFSET_BG1HOFS, 0);
//     SetGpuReg(REG_OFFSET_BG1VOFS, 0);
//     SetGpuReg(REG_OFFSET_BG0HOFS, 0);
//     SetGpuReg(REG_OFFSET_BG0VOFS, 0);
//     DmaFill16(3, 0, VRAM, VRAM_SIZE);
//     DmaFill32(3, 0, OAM, OAM_SIZE);
//     DmaFill16(3, 0, PLTT, PLTT_SIZE);
//     ResetPaletteFade();
//     DecompressDataWithHeaderVram(sBirchSpeechShadowGfx, (u8 *)VRAM);
//     DecompressDataWithHeaderVram(sBirchSpeechBgMap, (u8 *)(BG_SCREEN_ADDR(7)));
//     LoadPalette(sBirchSpeechBgPals, BG_PLTT_ID(0), 2 * PLTT_SIZE_4BPP);
//     LoadPalette(&sBirchSpeechBgGradientPal[1], BG_PLTT_ID(0) + 1, PLTT_SIZEOF(8));
//     ResetTasks();
//     taskId = CreateTask(Task_NewGameBirchSpeech_ReturnFromNamingScreenShowTextbox, 0);
//     gTasks[taskId].tTimer = 5;
//     gTasks[taskId].tBG1HOFS = -60;
//     ScanlineEffect_Stop();
//     ResetSpriteData();
//     FreeAllSpritePalettes();
//     ResetAllPicSprites();
//     AddBirchSpeechObjects(taskId);
//     if (gSaveBlock2Ptr->playerGender != MALE)
//     {
//         gTasks[taskId].tPlayerGender = FEMALE;
//         spriteId = gTasks[taskId].tMaySpriteId;
//     }
//     else
//     {
//         gTasks[taskId].tPlayerGender = MALE;
//         spriteId = gTasks[taskId].tBrendanSpriteId;
//     }
//     gSprites[spriteId].x = 180;
//     gSprites[spriteId].y = 60;
//     gSprites[spriteId].invisible = FALSE;
//     gTasks[taskId].tPlayerSpriteId = spriteId;
//     SetGpuReg(REG_OFFSET_BG1HOFS, -60);
//     BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
//     SetGpuReg(REG_OFFSET_WIN0H, 0);
//     SetGpuReg(REG_OFFSET_WIN0V, 0);
//     SetGpuReg(REG_OFFSET_WININ, 0);
//     SetGpuReg(REG_OFFSET_WINOUT, 0);
//     SetGpuReg(REG_OFFSET_BLDCNT, 0);
//     SetGpuReg(REG_OFFSET_BLDALPHA, 0);
//     SetGpuReg(REG_OFFSET_BLDY, 0);
//     ShowBg(0);
//     ShowBg(1);
//     savedIme = REG_IME;
//     REG_IME = 0;
//     REG_IE |= 1;
//     REG_IME = savedIme;
//     SetVBlankCallback(VBlankCB_MainMenu);
//     SetMainCallback2(CB2_MainMenu);
//     InitWindows(sNewGameKukuiCallTextWindows);
//     LoadMainMenuWindowFrameTiles(0, 0xF3);
//     LoadMessageBoxGfx(0, BIRCH_DLG_BASE_TILE_NUM, BG_PLTT_ID(15));
//     PutWindowTilemap(0);
//     CopyWindowToVram(0, COPYWIN_FULL);
// }

static void SpriteCB_Null(struct Sprite *sprite)
{
}

// static void SpriteCB_MovePlayerDownWhileShrinking(struct Sprite *sprite)
// {
//     u32 y;

//     y = (sprite->y << 16) + sprite->data[0] + 0xC000;
//     sprite->y = y >> 16;
//     sprite->data[0] = y;
// }

// static u8 NewGameBirchSpeech_CreateLotadSprite(u8 x, u8 y)
// {
//     return CreateMonPicSprite_Affine(SPECIES_ROCKRUFF, FALSE, 0, MON_PIC_AFFINE_FRONT, x, y, 14, TAG_NONE);
// }

#undef tTimer
#undef tCount
#undef tFreeKukuiBaseTileNum
#undef tPlayerSpriteId
#undef tBG1HOFS
#undef tIsDoneFadingSprites
#undef tPlayerGender
#undef tSettingsIconSpriteId
#undef tCameraIconSpriteId
#undef tNotificationIconSpriteId
#undef tVideoIconSpriteId
#undef tCallWindowCornerSpriteId
#undef tCallWindowEdgeSpriteId
#undef tCallWindowUISpriteId
#undef tCallWindowScalableSpriteId

// #define tMainTask data[0]
// #define tAlphaCoeff1 data[1]
// #define tAlphaCoeff2 data[2]
// #define tDelay data[3]
// #define tDelayTimer data[4]

// static void Task_NewGameBirchSpeech_FadeOutTarget1InTarget2(u8 taskId)
// {
//     int alphaCoeff2;

//     if (gTasks[taskId].tAlphaCoeff1 == 0)
//     {
//         gTasks[gTasks[taskId].tMainTask].tIsDoneFadingSprites = TRUE;
//         DestroyTask(taskId);
//     }
//     else if (gTasks[taskId].tDelayTimer)
//     {
//         gTasks[taskId].tDelayTimer--;
//     }
//     else
//     {
//         gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
//         gTasks[taskId].tAlphaCoeff1--;
//         gTasks[taskId].tAlphaCoeff2++;
//         alphaCoeff2 = gTasks[taskId].tAlphaCoeff2 << 8;
//         SetGpuReg(REG_OFFSET_BLDALPHA, gTasks[taskId].tAlphaCoeff1 + alphaCoeff2);
//     }
// }

// static void NewGameBirchSpeech_StartFadeOutTarget1InTarget2(u8 taskId, u8 delay)
// {
//     u8 taskId2;

//     SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT1_OBJ);
//     SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
//     SetGpuReg(REG_OFFSET_BLDY, 0);
//     gTasks[taskId].tIsDoneFadingSprites = 0;
//     taskId2 = CreateTask(Task_NewGameBirchSpeech_FadeOutTarget1InTarget2, 0);
//     gTasks[taskId2].tMainTask = taskId;
//     gTasks[taskId2].tAlphaCoeff1 = 16;
//     gTasks[taskId2].tAlphaCoeff2 = 0;
//     gTasks[taskId2].tDelay = delay;
//     gTasks[taskId2].tDelayTimer = delay;
// }

// static void Task_NewGameBirchSpeech_FadeInTarget1OutTarget2(u8 taskId)
// {
//     int alphaCoeff2;

//     if (gTasks[taskId].tAlphaCoeff1 == 16)
//     {
//         gTasks[gTasks[taskId].tMainTask].tIsDoneFadingSprites = TRUE;
//         DestroyTask(taskId);
//     }
//     else if (gTasks[taskId].tDelayTimer)
//     {
//         gTasks[taskId].tDelayTimer--;
//     }
//     else
//     {
//         gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
//         gTasks[taskId].tAlphaCoeff1++;
//         gTasks[taskId].tAlphaCoeff2--;
//         alphaCoeff2 = gTasks[taskId].tAlphaCoeff2 << 8;
//         SetGpuReg(REG_OFFSET_BLDALPHA, gTasks[taskId].tAlphaCoeff1 + alphaCoeff2);
//     }
// }

// static void NewGameBirchSpeech_StartFadeInTarget1OutTarget2(u8 taskId, u8 delay)
// {
//     u8 taskId2;

//     SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT1_OBJ);
//     SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
//     SetGpuReg(REG_OFFSET_BLDY, 0);
//     gTasks[taskId].tIsDoneFadingSprites = 0;
//     taskId2 = CreateTask(Task_NewGameBirchSpeech_FadeInTarget1OutTarget2, 0);
//     gTasks[taskId2].tMainTask = taskId;
//     gTasks[taskId2].tAlphaCoeff1 = 0;
//     gTasks[taskId2].tAlphaCoeff2 = 16;
//     gTasks[taskId2].tDelay = delay;
//     gTasks[taskId2].tDelayTimer = delay;
// }

// #undef tMainTask
// #undef tAlphaCoeff1
// #undef tAlphaCoeff2
// #undef tDelay
// #undef tDelayTimer

// #undef tIsDoneFadingSprites

// #define tMainTask data[0]
// #define tPalIndex data[1]
// #define tDelayBefore data[2]
// #define tDelay data[3]
// #define tDelayTimer data[4]

// static void Task_NewGameBirchSpeech_FadePlatformIn(u8 taskId)
// {
//     if (gTasks[taskId].tDelayBefore)
//     {
//         gTasks[taskId].tDelayBefore--;
//     }
//     else if (gTasks[taskId].tPalIndex == 8)
//     {
//         DestroyTask(taskId);
//     }
//     else if (gTasks[taskId].tDelayTimer)
//     {
//         gTasks[taskId].tDelayTimer--;
//     }
//     else
//     {
//         gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
//         gTasks[taskId].tPalIndex++;
//         LoadPalette(&sBirchSpeechBgGradientPal[gTasks[taskId].tPalIndex], BG_PLTT_ID(0) + 1, PLTT_SIZEOF(8));
//     }
// }

// static void NewGameBirchSpeech_StartFadePlatformIn(u8 taskId, u8 delay)
// {
//     u8 taskId2;

//     taskId2 = CreateTask(Task_NewGameBirchSpeech_FadePlatformIn, 0);
//     gTasks[taskId2].tMainTask = taskId;
//     gTasks[taskId2].tPalIndex = 0;
//     gTasks[taskId2].tDelayBefore = 8;
//     gTasks[taskId2].tDelay = delay;
//     gTasks[taskId2].tDelayTimer = delay;
// }

// static void Task_NewGameBirchSpeech_FadePlatformOut(u8 taskId)
// {
//     if (gTasks[taskId].tDelayBefore)
//     {
//         gTasks[taskId].tDelayBefore--;
//     }
//     else if (gTasks[taskId].tPalIndex == 0)
//     {
//         DestroyTask(taskId);
//     }
//     else if (gTasks[taskId].tDelayTimer)
//     {
//         gTasks[taskId].tDelayTimer--;
//     }
//     else
//     {
//         gTasks[taskId].tDelayTimer = gTasks[taskId].tDelay;
//         gTasks[taskId].tPalIndex--;
//         LoadPalette(&sBirchSpeechBgGradientPal[gTasks[taskId].tPalIndex], BG_PLTT_ID(0) + 1, PLTT_SIZEOF(8));
//     }
// }

// static void NewGameBirchSpeech_StartFadePlatformOut(u8 taskId, u8 delay)
// {
//     u8 taskId2;

//     taskId2 = CreateTask(Task_NewGameBirchSpeech_FadePlatformOut, 0);
//     gTasks[taskId2].tMainTask = taskId;
//     gTasks[taskId2].tPalIndex = 8;
//     gTasks[taskId2].tDelayBefore = 8;
//     gTasks[taskId2].tDelay = delay;
//     gTasks[taskId2].tDelayTimer = delay;
// }

// #undef tMainTask
// #undef tPalIndex
// #undef tDelayBefore
// #undef tDelay
// #undef tDelayTimer

// static void NewGameBirchSpeech_ShowGenderMenu(void)
// {
//     DrawMainMenuWindowBorder(&sNewGameKukuiCallTextWindows[1], 0xF3);
//     FillWindowPixelBuffer(1, PIXEL_FILL(1));
//     PrintMenuTable(1, ARRAY_COUNT(sMenuActions_Gender), sMenuActions_Gender);
//     InitMenuInUpperLeftCornerNormal(1, ARRAY_COUNT(sMenuActions_Gender), 0);
//     PutWindowTilemap(1);
//     CopyWindowToVram(1, COPYWIN_FULL);
// }

// static s8 NewGameBirchSpeech_ProcessGenderMenuInput(void)
// {
//     return Menu_ProcessInputNoWrap();
// }

// void NewGameBirchSpeech_SetDefaultPlayerName(u8 nameId)
// {
//     const u8 *name;
//     u8 i;

//     if (gSaveBlock2Ptr->playerGender == MALE)
//         name = sMalePresetNames[nameId];
//     else
//         name = sFemalePresetNames[nameId];
//     for (i = 0; i < PLAYER_NAME_LENGTH; i++)
//         gSaveBlock2Ptr->playerName[i] = name[i];
//     gSaveBlock2Ptr->playerName[PLAYER_NAME_LENGTH] = EOS;
// }

// static void NewGameBirchSpeech_ClearGenderWindowTilemap(u8 bg, u8 x, u8 y, u8 width, u8 height, u8 unused)
// {
//     FillBgTilemapBufferRect(bg, 0, x + 255, y + 255, width + 2, height + 2, 2);
// }

// static void NewGameBirchSpeech_ClearGenderWindow(u8 windowId, bool8 copyToVram)
// {
//     CallWindowFunction(windowId, NewGameBirchSpeech_ClearGenderWindowTilemap);
//     FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
//     ClearWindowTilemap(windowId);
//     if (copyToVram == TRUE)
//         CopyWindowToVram(windowId, COPYWIN_FULL);
// }

// static void NewGameBirchSpeech_ClearWindow(u8 windowId)
// {
//     u8 bgColor = GetFontAttribute(FONT_NORMAL, FONTATTR_COLOR_BACKGROUND);
//     u8 maxCharWidth = GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_WIDTH);
//     u8 maxCharHeight = GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT);
//     u8 winWidth = GetWindowAttribute(windowId, WINDOW_WIDTH);
//     u8 winHeight = GetWindowAttribute(windowId, WINDOW_HEIGHT);

//     FillWindowPixelRect(windowId, bgColor, 0, 0, maxCharWidth * winWidth, maxCharHeight * winHeight);
//     CopyWindowToVram(windowId, COPYWIN_GFX);
// }

// static void NewGameBirchSpeech_WaitForThisIsPokemonText(struct TextPrinterTemplate *printer, u16 renderCmd)
// {
//     // Wait for Birch's "This is a Pokémon" text to reach the pause
//     // Then start the PokéBall release (if it hasn't been started already)
//     if (*(printer->currentChar - 2) == EXT_CTRL_CODE_PAUSE && !sStartedPokeBallTask)
//     {
//         sStartedPokeBallTask = TRUE;
//         CreateTask(Task_NewGameBirchSpeechSub_InitPokeBall, 0);
//     }
// }

// void CreateYesNoMenuParameterized(u8 x, u8 y, u16 baseTileNum, u16 baseBlock, u8 yesNoPalNum, u8 winPalNum)
// {
//     struct WindowTemplate template = CreateWindowTemplate(0, x + 1, y + 1, 5, 4, winPalNum, baseBlock);
//     CreateYesNoMenu(&template, baseTileNum, yesNoPalNum, 0);
// }

// static void Task_NewGameBirchSpeech_ReturnFromNamingScreenShowTextbox(u8 taskId)
// {
//     if (gTasks[taskId].tTimer-- <= 0)
//     {
//         DrawDialogFrameWithCustomTile(0, TRUE, BIRCH_DLG_BASE_TILE_NUM);
//         DrawNamePlateWithCustomTile(3, TRUE, BIRCH_DLG_BASE_TILE_NUM);
//         gTasks[taskId].func = Task_NewGameBirchSpeech_SoItsPlayerName;
//     }
// }

// #undef tTimer

static void NewGameKukuiCall_PrintNameplate(void)
{
    int strLen;
    const u8 colors[3] = {0, 1, 14};

    StringExpandPlaceholders(gStringVar1, gText_Birch_Nameplate);
    strLen = GetStringWidth(FONT_SMALL, gStringVar1, -1);

    if (strLen > 0)
    {
        strLen = GetDialogFramePlateWidth() / 2 - strLen / 2;
        gNamePlateBuffer[0] = EXT_CTRL_CODE_BEGIN;
        gNamePlateBuffer[1] = EXT_CTRL_CODE_CLEAR_TO;
        gNamePlateBuffer[2] = strLen;
        StringExpandPlaceholders(&gNamePlateBuffer[3], gStringVar1);
    }
    else
    {
        StringExpandPlaceholders(&gNamePlateBuffer[0], gStringVar1);
    }

    FillDialogFramePlate(1);
    AddTextPrinterParameterized3(1, FONT_SMALL, 0, 0, colors, 0, gNamePlateBuffer);

    DrawNamePlateWithCustomTile(1, TRUE, BIRCH_DLG_BASE_TILE_NUM);
}
