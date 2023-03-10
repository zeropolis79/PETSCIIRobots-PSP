#include <iostream>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <errno.h>
#include <fatms.h>
#include <geman.h>
#include <libgu.h>
#include <libgum.h>
#include <libwave.h>
#include <displaysvc.h>
#include <ctrlsvc.h>
#include <libfpu.h>
#include <impose.h>
#include <utility/utility_common.h>
#include <psperror.h>
#include <malloc.h>
#include "PT2.3A_replay_cia.h"
#include "PlatformPSP.h"

SCE_MODULE_INFO(petrobots, 0, 1, 0 );
int sce_newlib_heap_kb_size = 18430;
unsigned int sce_user_main_thread_stack_kb_size = 16;
unsigned int sce_user_main_thread_attribute = SCE_KERNEL_TH_USE_VFPU;
static const SceChar8 *SOUND_THREAD_NAME = "Sound";

#define DISPLAYLIST_SIZE (409600 / sizeof(int))
#define CACHE_SIZE 344064

static ScePspFMatrix4 matrix_stack[8 + 8 + 32 + 0];
static char cache[CACHE_SIZE];
static int cacheSize = 0;

#define LARGEST_MODULE_SIZE 105654
#define TOTAL_SAMPLE_SIZE 75755
#define AUDIO_BUFFER_SIZE 256
#define SAMPLERATE 44100

extern uint8_t tileset[];
extern uint32_t font[];
extern uint32_t faces[];
extern uint32_t tiles[];
extern uint32_t sprites[];
extern uint32_t animTiles[];
extern uint32_t items[];
extern uint32_t keys[];
extern uint32_t health[];
extern uint32_t introScreen[];
extern uint32_t gameScreen[];
extern uint32_t gameOver[];
extern uint8_t levelA[];
extern uint8_t levelAEnd[];
extern uint8_t levelB[];
extern uint8_t levelBEnd[];
extern uint8_t levelC[];
extern uint8_t levelCEnd[];
extern uint8_t levelD[];
extern uint8_t levelDEnd[];
extern uint8_t levelE[];
extern uint8_t levelEEnd[];
extern uint8_t levelF[];
extern uint8_t levelFEnd[];
extern uint8_t levelG[];
extern uint8_t levelGEnd[];
extern uint8_t levelH[];
extern uint8_t levelHEnd[];
extern uint8_t levelI[];
extern uint8_t levelIEnd[];
extern uint8_t levelJ[];
extern uint8_t levelJEnd[];
extern uint8_t levelK[];
extern uint8_t levelKEnd[];
extern uint8_t levelL[];
extern uint8_t levelLEnd[];
extern uint8_t levelM[];
extern uint8_t levelMEnd[];
extern uint8_t levelN[];
extern uint8_t levelNEnd[];
extern uint8_t moduleSoundFX[];
extern uint8_t moduleSoundFXEnd[];
extern uint8_t moduleMetalHeads[];
extern uint8_t moduleMetalHeadsEnd[];
extern uint8_t moduleWin[];
extern uint8_t moduleWinEnd[];
extern uint8_t moduleLose[];
extern uint8_t moduleLoseEnd[];
extern uint8_t moduleMetallicBopAmiga[];
extern uint8_t moduleMetallicBopAmigaEnd[];
extern uint8_t moduleGetPsyched[];
extern uint8_t moduleGetPsychedEnd[];
extern uint8_t moduleRobotAttack[];
extern uint8_t moduleRobotAttackEnd[];
extern uint8_t moduleRushinIn[];
extern uint8_t moduleRushinInEnd[];
extern int8_t soundExplosion[];
extern int8_t soundExplosionEnd[];
extern int8_t soundMedkit[];
extern int8_t soundMedkitEnd[];
extern int8_t soundEMP[];
extern int8_t soundEMPEnd[];
extern int8_t soundMagnet[];
extern int8_t soundMagnetEnd[];
extern int8_t soundShock[];
extern int8_t soundShockEnd[];
extern int8_t soundMove[];
extern int8_t soundMoveEnd[];
extern int8_t soundPlasma[];
extern int8_t soundPlasmaEnd[];
extern int8_t soundPistol[];
extern int8_t soundPistolEnd[];
extern int8_t soundItemFound[];
extern int8_t soundItemFoundEnd[];
extern int8_t soundError[];
extern int8_t soundErrorEnd[];
extern int8_t soundCycleWeapon[];
extern int8_t soundCycleWeaponEnd[];
extern int8_t soundCycleItem[];
extern int8_t soundCycleItemEnd[];
extern int8_t soundDoor[];
extern int8_t soundDoorEnd[];
extern int8_t soundMenuBeep[];
extern int8_t soundMenuBeepEnd[];
extern int8_t soundShortBeep[];
extern int8_t soundShortBeepEnd[];

uint32_t* images[] = { introScreen, gameScreen, gameOver };

static int8_t tileSpriteMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1, 49, 50, 57, 58, 59, 60, -1, -1, -1, -1, -1, -1, -1, 48,
    -1, -1, -1, 73, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  0,  3, -1, 53, 54, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 76, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
static int8_t animTileMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16,
    -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,  8, 10, -1, -1, 12, 14, -1, -1, 20, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
static const char* moduleFilenames[] = {
    "mod.soundfx",
    "mod.metal heads",
    "mod.win",
    "mod.lose",
    "mod.metallic bop amiga",
    "mod.get psyched",
    "mod.robot attack",
    "mod.rushin in"
};

static uint8_t standardControls[] = {
    0, // MOVE UP orig: 56 (8)
    0, // MOVE DOWN orig: 50 (2)
    0, // MOVE LEFT orig: 52 (4)
    0, // MOVE RIGHT orig: 54 (6)
    0, // FIRE UP
    0, // FIRE DOWN
    0, // FIRE LEFT
    0, // FIRE RIGHT
    0, // CYCLE WEAPONS
    0, // CYCLE ITEMS
    0, // USE ITEM
    0, // SEARCH OBEJCT
    0, // MOVE OBJECT
    0, // LIVE MAP
    0, // LIVE MAP ROBOTS
    0, // PAUSE
    0, // MUSIC
    0, // CHEAT
    0, // CURSOR UP
    0, // CURSOR DOWN
    0, // CURSOR LEFT
    0, // CURSOR RIGHT
    0, // SPACE
    0, // RETURN
    0, // YES
    0 // NO
};

void debug(const char* message, ...)
{
    FILE* f = fopen(SCE_FATMS_ALIAS_NAME "/log.txt", "a");
    va_list argList;
    va_start(argList, message);
    vfprintf(f, message, argList);
    fflush(f);
    fclose(f);
    va_end(argList);
}

uint32_t paletteIntro[] = {
    0xff000000,
    0xff443300,
    0xff775533,
    0xff997755,
    0xffccaa88,
    0xff882222,
    0xffcc7766,
    0xffee8888,
    0xffaa5577,
    0xff3311aa,
    0xff6644cc,
    0xff4488ee,
    0xff33bbee,
    0xff88eeee,
    0xffeeeeee,
    0xff55bb77
};

uint32_t paletteGame[] = {
    0xff000000,
    0xffffffff,
    0xff775544,
    0xff998877,
    0xffccbbaa,
    0xff993300,
    0xffbb6633,
    0xffffaa00,
    0xff006655,
    0xff009977,
    0xff00ddaa,
    0xff004477,
    0xff0077bb,
    0xff00ccff,
    0xff99aaee,
    0xff0000ee
};

#define LIVE_MAP_ORIGIN_X ((PLATFORM_SCREEN_WIDTH - 56 - 128 * 3) / 2)
#define LIVE_MAP_ORIGIN_Y ((PLATFORM_SCREEN_HEIGHT - 32 - 64 * 3) / 2)

uint8_t tileLiveMap[] = {
     0,13, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1,13, 1,
     1, 1, 1, 1, 1, 1, 1, 2, 8, 1, 1, 1, 1, 6,14,14,
    15,13,14,15,15,13, 5,15, 6,13,13,12, 6,13,13,12,
     1, 1, 1,12, 1, 9, 9, 6, 1, 9,15, 6,10,10, 1, 1,
     1, 1, 7,13, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     2, 2,13,13,13,13,13,13, 1, 1, 1,13, 4, 4, 4, 0,
     1, 1, 4,13, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2,
     1, 1,13,10, 1, 1,13, 9, 1, 1, 4, 5,13,13,13, 4,
     1, 1, 1, 1,15, 5,15,15, 1, 1, 6, 6,15,15, 6, 5,
     2, 2, 2, 5,13,13, 1, 7, 5, 3, 1, 7, 4, 4, 4,13,
     1, 1, 1, 1, 1, 4, 4, 2, 3, 3, 3, 1, 3, 2, 3, 3,
     3, 3, 3, 1, 4, 4, 9, 9, 4, 4, 2, 2, 5,11,12,12,
     8, 8, 8, 9, 3, 3, 2,10, 1, 1, 1, 9, 1, 1, 1,10,
     1, 1, 1,13, 8, 8,13,13, 8, 8,13,13, 3, 3,13,13,
    13,13, 5,13,13,13,13,13,13,13,13,13,13,13,13,13
};
uint8_t liveMapToPlane1[256];
uint8_t liveMapToPlane2[256];
uint8_t liveMapToPlane3[256];
uint8_t liveMapToPlane4[256];
uint8_t unitTypes[48];
uint8_t unitX[48];
uint8_t unitY[48];

PlatformPSP::PlatformPSP() :
    eDRAMAddress((uint8_t*)sceGeEdramGetAddr()),
    interrupt(0),
    framesPerSecond_(60),
    moduleData(new uint8_t[LARGEST_MODULE_SIZE]),
    loadedModule(ModuleSoundFX),
    effectChannel(0),
    audioBuffer(new int16_t[AUDIO_BUFFER_SIZE]),
    audioOutputBuffer(new SceShort16[AUDIO_BUFFER_SIZE * 2 * 2]),
    audioOutputBufferOffset(0),
    audioThreadId(0),
    displayList(new int[DISPLAYLIST_SIZE]),
    joystickStateToReturn(0),
    joystickState(0),
    palette(paletteIntro),
    cursorX(-1),
    cursorY(-1),
    cursorShape(ShapeUse),
    scaleX(1.0f),
    scaleY(1.0f),
    fadeBaseColor(0),
    fadeIntensity(0),
    drawToBuffer0(false),
    swapBuffers(false),
    isDirty(false)
{
    // Clear the first two bytes of effect samples to enable the 2-byte no-loop loop
    *((uint16_t*)soundExplosion) = 0;
    *((uint16_t*)soundMedkit) = 0;
    *((uint16_t*)soundEMP) = 0;
    *((uint16_t*)soundMagnet) = 0;
    *((uint16_t*)soundShock) = 0;
    *((uint16_t*)soundMove) = 0;
    *((uint16_t*)soundPlasma) = 0;
    *((uint16_t*)soundPistol) = 0;
    *((uint16_t*)soundItemFound) = 0;
    *((uint16_t*)soundError) = 0;
    *((uint16_t*)soundCycleWeapon) = 0;
    *((uint16_t*)soundCycleItem) = 0;
    *((uint16_t*)soundDoor) = 0;
    *((uint16_t*)soundMenuBeep) = 0;
    *((uint16_t*)soundShortBeep) = 0;

    // Increase thread priority
    sceKernelChangeThreadPriority(SCE_KERNEL_TH_SELF, 40);

    // Set the impose language
    sceImposeSetLanguageMode(SCE_UTILITY_LANG_ENGLISH, SCE_UTILITY_CTRL_ASSIGN_CROSS_IS_ENTER);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALONLY);

    if (sceWaveInit() != SCE_OK) {
        debug("Couldn't initialize libwave\n");
    }

    int ret = sceWaveAudioSetFormat(0, SCE_WAVE_AUDIO_FMT_S16_STEREO);
    if (ret != SCE_OK && ret != (int)SCE_AUDIO_ERROR_OUTPUT_BUSY) {
        debug("Couldn't set audio format\n");
    }

    sceWaveAudioSetSample(0, AUDIO_BUFFER_SIZE);
    sceWaveAudioSetVolume(0, SCE_WAVE_AUDIO_VOL_MAX, SCE_WAVE_AUDIO_VOL_MAX);

    sceGuInit();

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);
    sceGuDispBuffer(SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
    sceGuDepthBuffer(SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);

    sceGuOffset(SCEGU_SCR_OFFSETX, SCEGU_SCR_OFFSETY);
    sceGuViewport(2048, 2048, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
    sceGuDepthRange(100, 65535);

    sceGuScissor(0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT);
    sceGuEnable(SCEGU_SCISSOR_TEST);
    sceGuDisable(SCEGU_CLIP_PLANE);

    sceGuDepthFunc(SCEGU_LEQUAL);
    sceGuEnable(SCEGU_TEXTURE);

    sceGuColor(0xffffffff);
    sceGuClearColor(0xff000000);
    sceGuClearDepth(0);
    sceGuClearStencil(0);
    sceGuClear(SCEGU_CLEAR_ALL);

    sceGuTexEnvColor(0xffffff);
    sceGuTexScale(1.0f, 1.0f);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexWrap(SCEGU_CLAMP, SCEGU_CLAMP);
    sceGuTexFilter(SCEGU_NEAREST, SCEGU_NEAREST);
    sceGuTexMode(SCEGU_PF8888, 0, 0, SCEGU_TEXBUF_NORMAL);
    sceGuModelColor(0x00000000, 0xffffffff, 0xffffffff, 0xffffffff);

    sceGuFrontFace(SCEGU_CW);
    sceGuShadeModel(SCEGU_SMOOTH);
    sceGuDisable(SCEGU_DEPTH_TEST);
    sceGuEnable(SCEGU_BLEND);
    sceGuDisable(SCEGU_FOG);
    sceGuDisable(SCEGU_LIGHTING);
    sceGuBlendFunc(SCEGU_ADD, SCEGU_SRC_ALPHA, SCEGU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuTexFunc(SCEGU_TEX_MODULATE, SCEGU_RGBA);

    sceGumSetMatrixStack(matrix_stack, 8, 8, 32, 0);
    sceGumMatrixMode(SCEGU_MATRIX_WORLD);
    sceGumLoadIdentity();
    sceGumMatrixMode(SCEGU_MATRIX_VIEW);
    sceGumLoadIdentity();
    sceGumMatrixMode(SCEGU_MATRIX_TEXTURE);
    sceGumLoadIdentity();
    sceGumMatrixMode(SCEGU_MATRIX_PROJECTION);
    sceGumLoadIdentity();
    sceGumOrtho(0, SCEGU_SCR_WIDTH, 0, SCEGU_SCR_HEIGHT, -1.0f, 1.0f);
    sceGumMatrixMode(SCEGU_MATRIX_WORLD);

    sceGuDisplay(SCEGU_DISPLAY_ON);

    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    platform = this;

    PlatformPSP* p = this;
    audioThreadId = sceKernelCreateThread(SOUND_THREAD_NAME, audioThread, SCE_KERNEL_USER_HIGHEST_PRIORITY, 1024, 0, NULL);
    sceKernelStartThread(audioThreadId, sizeof(PlatformPSP *), &p);
    sceKernelStartThread(sceKernelCreateThread("update_thread", callbackThread, 0x11, 2048, 0, NULL), sizeof(PlatformPSP*), &p);
    sceDisplaySetVblankCallback(0, vblankHandler, p);
}

PlatformPSP::~PlatformPSP()
{
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);
    sceGuTerm();

    sceDisplaySetVblankCallback(0, 0, 0);

    if (audioThreadId != -1) {
        sceKernelWaitThreadEnd(audioThreadId, NULL);
    }

    sceWaveExit();

    delete[] displayList;
    delete[] audioOutputBuffer;
    delete[] audioBuffer;
    delete[] moduleData;

    sceKernelExitGame();
}

int PlatformPSP::callbackThread(SceSize args, void* argp)
{
    PlatformPSP* platform = *((PlatformPSP**)argp);
    sceKernelRegisterExitCallback(sceKernelCreateCallback("Exit Callback", exitCallback, platform));

    // Display callback notifications
    while (!platform->quit) {
        sceDisplayWaitVblankStartCB();
    }

    return 0;
}

int PlatformPSP::exitCallback(int arg1, int arg2, void* common)
{
    PlatformPSP* platform = (PlatformPSP*)common;
    platform->quit = true;

    return 0;
}

SceInt32 PlatformPSP::audioThread(SceSize args, SceVoid *argb)
{
    PlatformPSP* platform = *((PlatformPSP**)argb);

    // Render loop
    while (!platform->quit) {
        // Process each audio channel
        processAudio(platform->audioBuffer, AUDIO_BUFFER_SIZE, SAMPLERATE);

        // Render to the actual output buffer
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            int16_t sample = platform->audioBuffer[i];
            sample = sample < -8192 ? INT16_MIN :
                    (sample >= 8192 ? INT16_MAX : (sample << 2));
            platform->audioOutputBuffer[platform->audioOutputBufferOffset + i * 2] = sample;
            platform->audioOutputBuffer[platform->audioOutputBufferOffset + i * 2 + 1] = sample;
        }

        // Queue the output buffer for playback
        sceWaveAudioWriteBlocking(0, SCE_WAVE_AUDIO_VOL_MAX, SCE_WAVE_AUDIO_VOL_MAX, platform->audioOutputBuffer + platform->audioOutputBufferOffset);

        // Write the next data to the other part of the output buffer
        platform->audioOutputBufferOffset ^= AUDIO_BUFFER_SIZE * 2;
    }

    return 0;
}

void PlatformPSP::vblankHandler(int idx, void* cookie)
{
    PlatformPSP* platform = (PlatformPSP*)cookie;

    if (idx == 0) {
        if (platform->swapBuffers) {
            sceDisplaySetFrameBuf(platform->eDRAMAddress + (uint32_t)(platform->drawToBuffer0 ? SCEGU_VRAM_BP32_0 : SCEGU_VRAM_BP32_1), 512, SCE_DISPLAY_PIXEL_RGBA8888, SCE_DISPLAY_UPDATETIMING_NEXTHSYNC);
            platform->drawToBuffer0 = !platform->drawToBuffer0;
            platform->swapBuffers = false;
        }

        if (platform->interrupt) {
            (*platform->interrupt)();
        }
    }
}

void PlatformPSP::drawRectangle(uint32_t color, uint32_t* texture, uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    if (texture) {
        sceGuEnable(SCEGU_TEXTURE);
        sceGuTexImage(0, texture[2], texture[3], texture[2], texture + 4);
    } else {
        sceGuDisable(SCEGU_TEXTURE);
    }
    sceGuColor(color);

    int oldCacheSize = cacheSize;
    float* data = (float*)(cache + cacheSize);
    if (texture) {
        data[0 * 5 + 0] = tx / (float)texture[2];
        data[0 * 5 + 1] = ty / (float)texture[3];
        data[0 * 5 + 2] = x;
        data[0 * 5 + 3] = (SCEGU_SCR_HEIGHT / scaleY) - y;
        data[0 * 5 + 4] = 0;
        data[1 * 5 + 0] = (tx + width) / (float)texture[2];
        data[1 * 5 + 1] = (ty + height) / (float)texture[3];
        data[1 * 5 + 2] = x + width;
        data[1 * 5 + 3] = (SCEGU_SCR_HEIGHT / scaleY) - (y + height);
        data[1 * 5 + 4] = 0;
        cacheSize += 2 * 5 * sizeof(float);

        sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
        sceGumDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_TEXTURE_FLOAT | SCEGU_VERTEX_FLOAT, 2, 0, data);
    } else {
        data[0 * 3 + 0] = x;
        data[0 * 3 + 1] = (SCEGU_SCR_HEIGHT / scaleY) - y;
        data[0 * 3 + 2] = 0;
        data[1 * 3 + 0] = x + width;
        data[1 * 3 + 1] = (SCEGU_SCR_HEIGHT / scaleY) - (y + height);
        data[1 * 3 + 2] = 0;
        cacheSize += 2 * 3 * sizeof(float);

        sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
        sceGumDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_VERTEX_FLOAT, 2, 0, data);
    }

    isDirty = true;
}

void PlatformPSP::undeltaSamples(uint8_t* module, uint32_t moduleSize)
{
    uint8_t numPatterns = 0;
    for (int i = 0; i < module[950]; i++) {
        numPatterns = MAX(numPatterns, module[952 + i]);
    }
    numPatterns++;

    int8_t* samplesStart = (int8_t*)(module + 1084 + (numPatterns << 10));
    int8_t* samplesEnd = (int8_t*)(module + moduleSize);

    int8_t sample = 0;
    for (int8_t* sampleData = samplesStart; sampleData < samplesEnd; sampleData++) {
        int8_t delta = *sampleData;
        sample += delta;
        *sampleData = sample;
    }
}

void PlatformPSP::setSampleData(uint8_t* module)
{
    mt_SampleStarts[15 + 0] = soundExplosion;
    mt_SampleStarts[15 + 1] = soundShortBeep;
    mt_SampleStarts[15 + 2] = soundMedkit;
    mt_SampleStarts[15 + 3] = soundEMP;
    mt_SampleStarts[15 + 4] = soundMagnet;
    mt_SampleStarts[15 + 5] = soundShock;
    mt_SampleStarts[15 + 6] = soundMove;
    mt_SampleStarts[15 + 7] = soundShock;
    mt_SampleStarts[15 + 8] = soundPlasma;
    mt_SampleStarts[15 + 9] = soundPistol;
    mt_SampleStarts[15 + 10] = soundItemFound;
    mt_SampleStarts[15 + 11] = soundError;
    mt_SampleStarts[15 + 12] = soundCycleWeapon;
    mt_SampleStarts[15 + 13] = soundCycleItem;
    mt_SampleStarts[15 + 14] = soundDoor;
    mt_SampleStarts[15 + 15] = soundMenuBeep;

    SampleData* sampleData = (SampleData*)(module + 20);
    putWord((uint8_t*)&sampleData[15 + 0].length, 0, (uint16_t)(soundExplosionEnd - soundExplosion) >> 1);
    putWord((uint8_t*)&sampleData[15 + 1].length, 0, (uint16_t)(soundShortBeepEnd - soundShortBeep) >> 1);
    putWord((uint8_t*)&sampleData[15 + 2].length, 0, (uint16_t)(soundMedkitEnd - soundMedkit) >> 1);
    putWord((uint8_t*)&sampleData[15 + 3].length, 0, (uint16_t)(soundEMPEnd - soundEMP) >> 1);
    putWord((uint8_t*)&sampleData[15 + 4].length, 0, (uint16_t)(soundMagnetEnd - soundMagnet) >> 1);
    putWord((uint8_t*)&sampleData[15 + 5].length, 0, (uint16_t)(soundShockEnd - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 6].length, 0, (uint16_t)(soundMoveEnd - soundMove) >> 1);
    putWord((uint8_t*)&sampleData[15 + 7].length, 0, (uint16_t)(soundShockEnd - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 8].length, 0, (uint16_t)(soundPlasmaEnd - soundPlasma) >> 1);
    putWord((uint8_t*)&sampleData[15 + 9].length, 0, (uint16_t)(soundPistolEnd - soundPistol) >> 1);
    putWord((uint8_t*)&sampleData[15 + 10].length, 0, (uint16_t)(soundItemFoundEnd - soundItemFound) >> 1);
    putWord((uint8_t*)&sampleData[15 + 11].length, 0, (uint16_t)(soundErrorEnd - soundError) >> 1);
    putWord((uint8_t*)&sampleData[15 + 12].length, 0, (uint16_t)(soundCycleWeaponEnd - soundCycleWeapon) >> 1);
    putWord((uint8_t*)&sampleData[15 + 13].length, 0, (uint16_t)(soundCycleItemEnd - soundCycleItem) >> 1);
    putWord((uint8_t*)&sampleData[15 + 14].length, 0, (uint16_t)(soundDoorEnd - soundDoor) >> 1);
    putWord((uint8_t*)&sampleData[15 + 15].length, 0, (uint16_t)(soundMenuBeepEnd - soundMenuBeep) >> 1);
    for (int i = 0; i < 16; i++) {
        sampleData[15 + i].volume = 64;
    }
}

uint8_t* PlatformPSP::standardControls() const
{
    return ::standardControls;
}

void PlatformPSP::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

int PlatformPSP::framesPerSecond()
{
    return framesPerSecond_;
}

uint8_t PlatformPSP::readKeyboard()
{
    return 0xff;
}

void PlatformPSP::keyRepeat()
{
    joystickStateToReturn = joystickState;
}

void PlatformPSP::clearKeyBuffer()
{
    joystickStateToReturn = 0;
}

bool PlatformPSP::isKeyOrJoystickPressed(bool gamepad)
{
    return joystickState != 0 && joystickState != JoystickPlay;
}

uint16_t PlatformPSP::readJoystick(bool gamepad)
{
    uint16_t state = 0;

    // Read new input
    SceCtrlData ct;
    if (sceCtrlReadBufferPositive(&ct, 1) >= 0) {
        if (ct.Buttons & SCE_CTRL_LEFT) {
            state |= JoystickLeft;
        }
        if (ct.Buttons & SCE_CTRL_RIGHT) {
            state |= JoystickRight;
        }
        if (ct.Buttons & SCE_CTRL_UP) {
            state |= JoystickUp;
        }
        if (ct.Buttons & SCE_CTRL_DOWN) {
            state |= JoystickDown;
        }
        if (ct.Buttons & SCE_CTRL_SQUARE) {
            state |= JoystickGreen;
        }
        if (ct.Buttons & SCE_CTRL_CIRCLE) {
            state |= JoystickBlue;
        }
        if (ct.Buttons & SCE_CTRL_TRIANGLE) {
            state |= JoystickYellow;
        }
        if (ct.Buttons & SCE_CTRL_CROSS) {
            state |= JoystickRed;
        }
        if (ct.Buttons & SCE_CTRL_L) {
            state |= JoystickReverse;
        }
        if (ct.Buttons & SCE_CTRL_R) {
            state |= JoystickForward;
        }
        if (ct.Buttons & SCE_CTRL_START) {
            state |= JoystickExtra;
        }
        if (ct.Buttons & SCE_CTRL_SELECT) {
            state |= JoystickPlay;
        }
    }

    if (joystickState != state) {
        // Don't return Play button press
        joystickStateToReturn = state != JoystickPlay ? state : 0;
        joystickState = state;
    }

    uint16_t result = joystickStateToReturn;
    joystickStateToReturn = 0;
    return result;
}

struct FilenameMapping {
    const char* filename;
    uint8_t* data;
    uint32_t size;
};

#define FILENAME_MAPPINGS 22

static FilenameMapping filenameMappings[FILENAME_MAPPINGS] = {
    { "level-A", levelA, levelAEnd - levelA },
    { "level-B", levelB, levelBEnd - levelB },
    { "level-C", levelC, levelCEnd - levelC },
    { "level-D", levelD, levelDEnd - levelD },
    { "level-E", levelE, levelEEnd - levelE },
    { "level-F", levelF, levelFEnd - levelF },
    { "level-G", levelG, levelGEnd - levelG },
    { "level-H", levelH, levelHEnd - levelH },
    { "level-I", levelI, levelIEnd - levelI },
    { "level-J", levelJ, levelJEnd - levelJ },
    { "level-K", levelK, levelKEnd - levelK },
    { "level-L", levelL, levelLEnd - levelL },
    { "level-M", levelM, levelMEnd - levelM },
    { "level-N", levelN, levelNEnd - levelN },
    { "mod.soundfx", moduleSoundFX, moduleSoundFXEnd - moduleSoundFX },
    { "mod.metal heads", moduleMetalHeads, moduleMetalHeadsEnd - moduleMetalHeads },
    { "mod.win", moduleWin, moduleWinEnd - moduleWin },
    { "mod.lose", moduleLose, moduleLoseEnd - moduleLose },
    { "mod.metallic bop amiga", moduleMetallicBopAmiga, moduleMetallicBopAmigaEnd - moduleMetallicBopAmiga },
    { "mod.get psyched", moduleGetPsyched, moduleGetPsychedEnd - moduleGetPsyched },
    { "mod.robot attack", moduleRobotAttack, moduleRobotAttackEnd - moduleRobotAttack },
    { "mod.rushin in", moduleRushinIn, moduleRushinInEnd - moduleRushinIn }
};

uint32_t PlatformPSP::load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset)
{
    for (int i = 0; i < FILENAME_MAPPINGS; i++) {
        if (strcmp(filename, filenameMappings[i].filename) == 0) {
            uint32_t availableSize = MIN(size, filenameMappings[i].size - offset);
            memcpy(destination, filenameMappings[i].data + offset, availableSize);
            return availableSize;
        }
    }

    return 0;
}

uint8_t* PlatformPSP::loadTileset(const char* filename)
{
    return tileset;
}

void PlatformPSP::displayImage(Image image)
{
    sceGumLoadIdentity();
    scaleX = 1.0f;
    scaleY = 1.0f;

    sceGuScissor(0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT);

    this->clearRect(0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);

    if (image == ImageGame) {
        palette = paletteGame;

        drawRectangle(0xffffffff, images[image], 320 - 56, 0, PLATFORM_SCREEN_WIDTH - 56, 0, 56, 128);

        for (int y = 128; y < (PLATFORM_SCREEN_HEIGHT - 32); y += 40) {
            drawRectangle(0xffffffff, images[image], 320 - 56, 128, PLATFORM_SCREEN_WIDTH - 56, y, 56, MIN(40, PLATFORM_SCREEN_HEIGHT - 32 - y));
        }

        drawRectangle(0xffffffff, images[image], 320 - 56, 168, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32, 56, 32);

        drawRectangle(0xffffffff, images[image], 0, 168, 0, PLATFORM_SCREEN_HEIGHT - 32, 104, 8);

        for (int x = 104; x < (PLATFORM_SCREEN_WIDTH - 56); x += 160) {
            drawRectangle(0xffffffff, images[image], 104, 168, x, PLATFORM_SCREEN_HEIGHT - 32, MIN(160, PLATFORM_SCREEN_WIDTH - 56 - x), 8);
        }

        sceGuScissor(0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32);
    } else {
        palette = paletteIntro;

        scaleX = SCEGU_SCR_WIDTH / 320.0f;
        scaleY = SCEGU_SCR_HEIGHT / 200.0f;

        ScePspFVector3 vec;
        vec.x = scaleX;
        vec.y = scaleY;
        vec.z = 1.0f;
        sceGumScale(&vec);

        drawRectangle(0xffffffff, images[image], 0, 0, 0, 0, images[image][0], images[image][1]);
    }
}

void PlatformPSP::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
}

void PlatformPSP::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    if (transparent) {
        if (tileSpriteMap[tile] >= 0) {
            renderSprite(tileSpriteMap[tile] + variant, x, y);
            return;
        }
    } else {
        if (animTileMap[tile] >= 0) {
            renderAnimTile(animTileMap[tile] + variant, x, y);
            return;
        }
    }

    drawRectangle(0xffffffff, tiles, (tile & 15) * 24, (tile >> 4) * 24, x, y, 24, 24);
}

void PlatformPSP::renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant)
{
    if (animTileMap[backgroundTile] >= 0) {
        backgroundTile = animTileMap[backgroundTile] + backgroundVariant;
        drawRectangle(0xffffffff, animTiles, (backgroundTile >> 4) * 24, (backgroundTile & 15) * 24, x, y, 24, 24);
    } else {
        drawRectangle(0xffffffff, tiles, (backgroundTile & 15) * 24, (backgroundTile >> 4) * 24, x, y, 24, 24);
    }

    if (tileSpriteMap[foregroundTile] >= 0) {
        uint8_t sprite = tileSpriteMap[foregroundTile] + foregroundVariant;
        drawRectangle(0xffffffff, sprites, (sprite >> 4) * 24, (sprite & 15) * 24, x, y, 24, 24);
    } else {
        drawRectangle(0xffffffff, tiles, (foregroundTile & 15) * 24, (foregroundTile >> 4) * 24, x, y, 24, 24);
    }
}

void PlatformPSP::renderSprite(uint8_t sprite, uint16_t x, uint16_t y)
{
    drawRectangle(0xffffffff, sprites, (sprite >> 4) * 24, (sprite & 15) * 24, x, y, 24, 24);
}

void PlatformPSP::renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y)
{
    drawRectangle(0xffffffff, animTiles, (animTile >> 4) * 24, (animTile & 15) * 24, x, y, 24, 24);
}

void PlatformPSP::renderItem(uint8_t item, uint16_t x, uint16_t y)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    drawRectangle(0xffffffff, items, 0, item * 21, x, y, 48, 21);

    sceGuEnable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::renderKey(uint8_t key, uint16_t x, uint16_t y)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    drawRectangle(0xffffffff, keys, 0, key * 14, x, y, 16, 14);

    sceGuEnable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::renderHealth(uint8_t amount, uint16_t x, uint16_t y)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    drawRectangle(0xffffffff, health, 0, amount * 51, x, y, 48, 51);

    sceGuEnable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::renderFace(uint8_t face, uint16_t x, uint16_t y)
{
    drawRectangle(0xffffffff, faces, 0, face * 24, x, y, 16, 24);
}

void PlatformPSP::renderLiveMap(uint8_t* map)
{
    clearRect(0, 0, PLATFORM_SCREEN_WIDTH - 56, LIVE_MAP_ORIGIN_Y);
    clearRect(0, LIVE_MAP_ORIGIN_Y, LIVE_MAP_ORIGIN_X, PLATFORM_SCREEN_HEIGHT - 32 - 2 * LIVE_MAP_ORIGIN_Y);
    clearRect(PLATFORM_SCREEN_WIDTH - 56 - LIVE_MAP_ORIGIN_X, LIVE_MAP_ORIGIN_Y, LIVE_MAP_ORIGIN_X, PLATFORM_SCREEN_HEIGHT - 32 - 2 * LIVE_MAP_ORIGIN_Y);
    clearRect(0, PLATFORM_SCREEN_HEIGHT - 32 - LIVE_MAP_ORIGIN_Y, PLATFORM_SCREEN_WIDTH - 56, LIVE_MAP_ORIGIN_Y);

    sceGuEnable(SCEGU_TEXTURE);
    sceGuTexFilter(SCEGU_LINEAR, SCEGU_LINEAR);
    sceGuTexImage(0, tiles[2], tiles[3], tiles[2], tiles + 4);
    sceGuColor(0xffffffff);

    int oldCacheSize = cacheSize;
    float* dataStart = (float*)(cache + cacheSize);
    float* data = dataStart;
    for (int mapY = 0; mapY < 64; mapY++) {
        for (int mapX = 0; mapX < 128; mapX++) {
            int tile = *map++;
            int x = LIVE_MAP_ORIGIN_X + mapX * 3;
            int y = LIVE_MAP_ORIGIN_Y + mapY * 3;
            int tx = (tile & 15) * 24;
            int ty = (tile >> 4) * 24;

            *data++ = tx / (float)tiles[2];
            *data++ = ty / (float)tiles[3];
            *data++ = x;
            *data++ = (SCEGU_SCR_HEIGHT / scaleY) - y;
            *data++ = 0;

            *data++ = (tx + 24) / (float)tiles[2];
            *data++ = (ty + 24) / (float)tiles[3];
            *data++ = x + 3;
            *data++ = (SCEGU_SCR_HEIGHT / scaleY) - (y + 3);
            *data++ = 0;
        }
    }

    cacheSize += 64 * 128 * 2 * 5 * sizeof(float);

    sceKernelDcacheWritebackRange(dataStart, cacheSize - oldCacheSize);
    sceGumDrawArrayN(SCEGU_PRIM_RECTANGLES, SCEGU_TEXTURE_FLOAT | SCEGU_VERTEX_FLOAT, 2, 64 * 128, 0, dataStart);

    for (int i = 0; i < 48; i++) {
        unitTypes[i] = 255;
    }

    sceGuTexFilter(SCEGU_NEAREST, SCEGU_NEAREST);

    isDirty = true;
}

void PlatformPSP::renderLiveMapTile(uint8_t* map, uint8_t mapX, uint8_t mapY)
{
    sceGuEnable(SCEGU_TEXTURE);
    sceGuTexFilter(SCEGU_LINEAR, SCEGU_LINEAR);

    int tile = map[(mapY << 7) + mapX];
    int x = LIVE_MAP_ORIGIN_X + mapX * 3;
    int y = LIVE_MAP_ORIGIN_Y + mapY * 3;
    int tx = (tile & 15) * 24;
    int ty = (tile >> 4) * 24;

    sceGuTexImage(0, tiles[2], tiles[3], tiles[2], tiles + 4);
    sceGuColor(0xffffffff);

    int oldCacheSize = cacheSize;
    float* data = (float*)(cache + cacheSize);
    data[0 * 5 + 0] = tx / (float)tiles[2];
    data[0 * 5 + 1] = ty / (float)tiles[3];
    data[0 * 5 + 2] = x;
    data[0 * 5 + 3] = (SCEGU_SCR_HEIGHT / scaleY) - y;
    data[0 * 5 + 4] = 0;
    data[1 * 5 + 0] = (tx + 24) / (float)tiles[2];
    data[1 * 5 + 1] = (ty + 24) / (float)tiles[3];
    data[1 * 5 + 2] = x + 3;
    data[1 * 5 + 3] = (SCEGU_SCR_HEIGHT / scaleY) - (y + 3);
    data[1 * 5 + 4] = 0;
    cacheSize += 2 * 5 * sizeof(float);

    sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
    sceGumDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_TEXTURE_FLOAT | SCEGU_VERTEX_FLOAT, 2, 0, data);

    sceGuTexFilter(SCEGU_NEAREST, SCEGU_NEAREST);

    isDirty = true;
}

void PlatformPSP::renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots)
{
    for (int i = 0; i < 48; i++) {
        if ((i < 28 || unitTypes[i] == 22) && (unitX[i] != ::unitX[i] || unitY[i] != ::unitY[i] || (i > 0 && (!showRobots || unitTypes[i] == 22 || unitTypes[i] != ::unitTypes[i])) || (i == 0 && playerColor != ::unitTypes[i]))) {
            // Remove old dot if any
            if (::unitTypes[i] != 255) {
                renderLiveMapTile(map, ::unitX[i], ::unitY[i]);

                if (i > 0 && !showRobots) {
                    ::unitTypes[i] = 255;
                }
            }

            if (i == 0 ||
                (unitTypes[i] == 22 && (unitX[i] != unitX[0] || unitY[i] != unitY[0])) ||
                (showRobots &&
                 (unitTypes[i] == 1 ||
                 (unitTypes[i] >= 2 && unitTypes[i] <= 5) ||
                 (unitTypes[i] >= 17 && unitTypes[i] <= 18) ||
                 unitTypes[i] == 9))) {
                // Render new dot
                int x = unitX[i];
                int y = unitY[i];
                drawRectangle(palette[(i > 0 || playerColor == 1) ? 1 : 0], 0, 0, 0, LIVE_MAP_ORIGIN_X + x * 3, LIVE_MAP_ORIGIN_Y + y * 3, 3, 3);

                ::unitTypes[i] = i == 0 ? playerColor : unitTypes[i];
                ::unitX[i] = unitX[i];
                ::unitY[i] = unitY[i];
            }
        }
    }

    isDirty = true;
}

void PlatformPSP::showCursor(uint16_t x, uint16_t y)
{
    cursorX = x * 24 - 2;
    cursorY = y * 24 -2;

    isDirty = true;
}

void PlatformPSP::hideCursor()
{
    if (cursorX != -1) {
        cursorX = -1;

        isDirty = true;
    }
}

void PlatformPSP::setCursorShape(CursorShape shape)
{
    cursorShape = shape;
}

void PlatformPSP::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    sceGuCopyImage(SCEGU_PF8888, sourceX, sourceY, width, height, SCEGU_VRAM_WIDTH, eDRAMAddress + (uint32_t)SCEGU_VRAM_BP32_2, destinationX, destinationY, SCEGU_VRAM_WIDTH, eDRAMAddress + (uint32_t)SCEGU_VRAM_BP32_2);

    sceGuEnable(SCEGU_SCISSOR_TEST);

    isDirty = true;
}

void PlatformPSP::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    drawRectangle(0xff000000, 0, 0, 0, x, y, width, height);

    sceGuEnable(SCEGU_SCISSOR_TEST);

    isDirty = true;
}

void PlatformPSP::startFadeScreen(uint16_t color, uint16_t intensity)
{
    uint32_t r = (color & 0xf00) >> 8;
    uint32_t g = (color & 0x0f0) << 4;
    uint32_t b = (color & 0x00f) << 16;
    uint32_t bgr = r |  g | b;
    fadeBaseColor = bgr | (bgr << 4);
    fadeIntensity = intensity;

    isDirty = true;
}

void PlatformPSP::fadeScreen(uint16_t intensity, bool immediate)
{
    if (fadeIntensity != intensity) {
        if (immediate) {
            fadeIntensity = intensity;

            isDirty = true;
         } else {
            int16_t fadeDelta = intensity > fadeIntensity ? 1 : -1;
            do {
                fadeIntensity += fadeDelta;

                isDirty = true;

                this->renderFrame(true);
            } while (fadeIntensity != intensity);
        }
    }
}

void PlatformPSP::stopFadeScreen()
{
    fadeIntensity = 15;
    isDirty = true;
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    if (scaleX == 1.0f && (address % SCREEN_WIDTH_IN_CHARACTERS) == (SCREEN_WIDTH_IN_CHARACTERS - 7)) {
        return;
    }

    if (value > 127) {
        value &= 127;
        drawRectangle(0xff55bb77, 0, 0, 0, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, (address / SCREEN_WIDTH_IN_CHARACTERS) << 3, 8, 8);
        drawRectangle(0xff000000, font, (value >> 3) & 0x8, (value << 3) & 0x1ff, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, (address / SCREEN_WIDTH_IN_CHARACTERS) << 3, 8, 8);
    } else {
        sceGuDisable(SCEGU_BLEND);
        drawRectangle(0xff55bb77, font, (value >> 3) & 0x8, (value << 3) & 0x1ff, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, (address / SCREEN_WIDTH_IN_CHARACTERS) << 3, 8, 8);
        sceGuEnable(SCEGU_BLEND);
    }

    sceGuEnable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    sceGuDisable(SCEGU_SCISSOR_TEST);

    if (scaleX == 1.0f && (address % SCREEN_WIDTH_IN_CHARACTERS) == (SCREEN_WIDTH_IN_CHARACTERS - 7)) {
        return;
    }

    if (value > 127) {
        value &= 127;
        drawRectangle(palette[color], 0, 0, 0, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset, 8, 8);
        drawRectangle(0xff000000, font, (value >> 3) & 0x8, (value << 3) & 0x1ff, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset, 8, 8);
    } else {
        sceGuDisable(SCEGU_BLEND);
        drawRectangle(palette[color], font, (value >> 3) & 0x8, (value << 3) & 0x1ff, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset, 8, 8);
        sceGuEnable(SCEGU_BLEND);
    }

    sceGuEnable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::loadModule(Module module)
{
    if (loadedModule != module) {
        uint32_t moduleSize = load(moduleFilenames[module], moduleData, LARGEST_MODULE_SIZE, 0);
        undeltaSamples(moduleData, moduleSize);
        setSampleData(moduleData);
        loadedModule = module;
    }
}

void PlatformPSP::playModule(Module module)
{
    stopModule();
    stopSample();

    loadModule(module);
    mt_init(moduleData);

    mt_Enable = true;
}

void PlatformPSP::pauseModule()
{
    mt_speed = 0;
    mt_music();
    mt_Enable = false;
    channel0.volume = 0;
    channel1.volume = 0;
    channel2.volume = 0;
    channel3.volume = 0;
}

void PlatformPSP::stopModule()
{
    mt_end();
}

void PlatformPSP::playSample(uint8_t sample)
{
    ChanInput* input = &mt_chaninputs[effectChannel < 2 ? effectChannel : (5 - effectChannel)];

    effectChannel++;
    effectChannel &= 3;

    putWord((uint8_t*)&input->note, 0, 0x1000 + 320);
    if (sample < 16) {
        putWord((uint8_t*)&input->cmd, 0, sample << 12);
    } else if (sample == 16) {
        putWord((uint8_t*)&input->cmd, 0, 1 << 12);
    } else {
        putWord((uint8_t*)&input->cmd, 0, 15 << 12);
    }
}

void PlatformPSP::stopSample()
{
    for (int i = 0; i < 4; i++) {
        mt_chaninputs[i].note = 0;
        mt_chaninputs[i].cmd = 0;
    }
}

void PlatformPSP::renderFrame(bool waitForNextFrame)
{
    if (!isDirty) {
        return;
    }

    while (swapBuffers);

    sceGuCopyImage(SCEGU_PF8888, 0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_WIDTH, eDRAMAddress + (uint32_t)SCEGU_VRAM_BP32_2, 0, 0, SCEGU_VRAM_WIDTH, eDRAMAddress + (uint32_t)(drawToBuffer0 ? SCEGU_VRAM_BP32_0 : SCEGU_VRAM_BP32_1));
    sceGuDrawBuffer(SCEGU_PF8888, drawToBuffer0 ? SCEGU_VRAM_BP32_0 : SCEGU_VRAM_BP32_1, SCEGU_VRAM_WIDTH);
    sceGuDisable(SCEGU_SCISSOR_TEST);

    if (cursorX != -1) {
        drawRectangle(0xffffffff, 0, 0, 0, cursorX, cursorY, 28, 2);
        drawRectangle(0xffffffff, 0, 0, 0, cursorX, cursorY + 2, 2, 24);
        drawRectangle(0xffffffff, 0, 0, 0, cursorX + 26, cursorY + 2, 2, 24);
        drawRectangle(0xffffffff, 0, 0, 0, cursorX, cursorY + 26, 28, 2);
        if (cursorShape != ShapeUse) {
            renderSprite(cursorShape == ShapeSearch ? 83 : 85, cursorX + 2, cursorY + 2);
        }
    }

    if (fadeIntensity != 15) {
        uint32_t intensity = (15 - fadeIntensity) << 24;
        uint32_t abgr = intensity | (intensity << 4) | fadeBaseColor;
        drawRectangle(abgr, 0, 0, 0, 0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
    }
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    swapBuffers = true;
    cacheSize = 0;

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));
    sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);
    sceGuEnable(SCEGU_SCISSOR_TEST);

    isDirty = false;
}
