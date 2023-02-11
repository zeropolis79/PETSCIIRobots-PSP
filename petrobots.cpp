/**
 * PETSCII Robots (Portable version)
 * by David Murray 2020
 * dfwgreencars@gmail.com
 * ported by Vesa Halttunen 2021-2022
 * vesuri@jormas.com
 */

#include "PlatformPSP.h"
#include "petrobots.h"

uint8_t* DESTRUCT_PATH; // Destruct path array (256 bytes)
uint8_t* TILE_ATTRIB;   // Tile attrib array (256 bytes)

// These arrays can go anywhere in RAM
uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
uint8_t UNIT_DIRECTION[32]; // Movement direction of unit (32 bytes)
uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
uint8_t MAP_PRECALC[MAP_WINDOW_SIZE];    // Stores pre-calculated objects for map window (77 bytes)
uint8_t MAP_PRECALC_DIRECTION[MAP_WINDOW_SIZE];    // Stores pre-calculated object directions for map window (77 bytes)
uint8_t MAP_PRECALC_TYPE[MAP_WINDOW_SIZE];    // Stores pre-calculated object types for map window (77 bytes)
uint8_t PREVIOUS_MAP_BACKGROUND[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_BACKGROUND_VARIANT[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_FOREGROUND[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_FOREGROUND_VARIANT[MAP_WINDOW_SIZE];

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
uint8_t KEY_CONFIG[26];
enum KEYS {
    KEY_MOVE_UP,
    KEY_MOVE_DOWN,
    KEY_MOVE_LEFT,
    KEY_MOVE_RIGHT,
    KEY_FIRE_UP,
    KEY_FIRE_DOWN,
    KEY_FIRE_LEFT,
    KEY_FIRE_RIGHT,
    KEY_CYCLE_WEAPONS,
    KEY_CYCLE_ITEMS,
    KEY_USE,
    KEY_SEARCH,
    KEY_MOVE,
    KEY_LIVE_MAP,
    KEY_LIVE_MAP_ROBOTS,
    KEY_PAUSE,
    KEY_MUSIC,
    KEY_CHEAT,
    KEY_CURSOR_UP,
    KEY_CURSOR_DOWN,
    KEY_CURSOR_LEFT,
    KEY_CURSOR_RIGHT,
    KEY_SPACE,
    KEY_RETURN,
    KEY_YES,
    KEY_NO
};

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
uint8_t MAP_DATA[8960];
uint8_t* UNIT_TYPE = MAP_DATA;
uint8_t* UNIT_LOC_X = MAP_DATA + 1 * 64;
uint8_t* UNIT_LOC_Y = MAP_DATA + 2 * 64;
uint8_t* UNIT_A = MAP_DATA + 3 * 64;
uint8_t* UNIT_B = MAP_DATA + 4 * 64;
uint8_t* UNIT_C = MAP_DATA + 5 * 64;
uint8_t* UNIT_D = MAP_DATA + 6 * 64;
int8_t* UNIT_HEALTH = (int8_t*)MAP_DATA + 7 * 64;
uint8_t* MAP = MAP_DATA + 8 * 64 + 256;
// END OF MAP FILE

uint8_t TILE;           // The tile number to be plotted
uint8_t DIRECTION;      // The direction of the tile to be plotted
uint8_t WALK_FRAME;     // Player walking animation frame
uint8_t DEMATERIALIZE_FRAME; // Dematerialize animation frame
uint8_t MAP_X;          // Current X location on map
uint8_t MAP_Y;          // Current Y location on map
uint8_t MAP_WINDOW_X;   // Top left location of what is displayed in map window
uint8_t MAP_WINDOW_Y;   // Top left location of what is displayed in map window
uint8_t DECNUM;         // a decimal number to be displayed onscreen as 3 digits.
uint8_t ATTRIB;         // Tile attribute value
uint8_t UNIT;           // Current unit being processed
uint8_t TEMP_A;         // used within some routines
uint8_t TEMP_B;         // used within some routines
uint8_t TEMP_C;         // used within some routines
uint8_t TEMP_D;         // used within some routines
uint8_t CURSOR_X;       // For on-screen cursor
uint8_t CURSOR_Y;       // For on-screen cursor
uint8_t CURSOR_ON;      // Is cursor active or not? 1=yes 0=no
uint8_t REDRAW_WINDOW;  // 1=yes 0=no
uint8_t MOVE_RESULT;    // 1=Move request success, 0=fail.
uint8_t UNIT_FIND;      // 255=no unit present.
uint8_t MOVE_TYPE;      // %00000001=WALK %00000010=HOVER
uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

uint8_t* MAP_SOURCE;    // $FD
uint8_t SCREEN_MEMORY[SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS]; // $8000

int main(int argc, char *argv[])
{
    PlatformClass platformInstance;

    if (!platform) {
        return 1;
    }

    convertToPETSCII(INTRO_MESSAGE);
    convertToPETSCII(MSG_CANTMOVE);
    convertToPETSCII(MSG_BLOCKED);
    convertToPETSCII(MSG_SEARCHING);
    convertToPETSCII(MSG_NOTFOUND);
    convertToPETSCII(MSG_FOUNDKEY);
    convertToPETSCII(MSG_FOUNDGUN);
    convertToPETSCII(MSG_FOUNDEMP);
    convertToPETSCII(MSG_FOUNDBOMB);
    convertToPETSCII(MSG_FOUNDPLAS);
    convertToPETSCII(MSG_FOUNDMED);
    convertToPETSCII(MSG_FOUNDMAG);
    convertToPETSCII(MSG_MUCHBET);
    convertToPETSCII(MSG_EMPUSED);
    convertToPETSCII(MSG_TERMINATED);
    convertToPETSCII(MSG_TRANS1);
    convertToPETSCII(MSG_ELEVATOR);
    convertToPETSCII(MSG_LEVELS);
    convertToPETSCII(MSG_PAUSED);
    convertToPETSCII(MSG_MUSICON);
    convertToPETSCII(MSG_MUSICOFF);
    convertToPETSCII(MAP_NAMES);
    convertToPETSCII(LOAD_MSG2);
    convertToPETSCII(INTRO_OPTIONS);
    convertToPETSCII(DIFF_LEVEL_WORDS);
    convertToPETSCII(WIN_MSG);
    convertToPETSCII(LOS_MSG);
    convertToPETSCII(CONTROLTEXT);
    convertToPETSCII(CINEMA_MESSAGE);

#if (MAP_WINDOW_SIZE != 77)
    for (int i = 0; i < PLATFORM_MAP_WINDOW_TILES_HEIGHT; i++) {
        MAP_CHART[i] = i * 3 * SCREEN_WIDTH_IN_CHARACTERS;
    }
#endif

    platform->stopNote(); // RESET SOUND TO ZERO
    TILE_LOAD_ROUTINE();
    SETUP_INTERRUPT();
    SET_CONTROLS(); // copy initial key controls
    while (!platform->quit) {
        INTRO_SCREEN();
    }
    return 0;
}

void INIT_GAME()
{
    SCREEN_SHAKE = 0;
    LIVE_MAP_ON = 0;
    RESET_KEYS_AMMO();
    platform->fadeScreen(0, false);
    DISPLAY_GAME_SCREEN();
    DISPLAY_LOAD_MESSAGE2();
    platform->fadeScreen(15, false);
    MAP_LOAD_ROUTINE();
    START_IN_GAME_MUSIC();
    SET_DIFF_LEVEL();
    ANIMATE_PLAYER();
    CACULATE_AND_REDRAW();
    INVALIDATE_PREVIOUS_MAP();
    DRAW_MAP_WINDOW();
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    UNIT_TYPE[0] = 1;
    SET_INITIAL_TIMERS();
    PRINT_INTRO_MESSAGE();
    KEYTIMER = 30;
    MAIN_GAME_LOOP();
}

#define TILENAME 0
char MAPNAME[] = "level-a";
uint8_t KEYS = 0; // bit0=spade bit2=heart bit3=star
uint8_t AMMO_PISTOL = 0; // how much ammo for the pistol
uint8_t AMMO_PLASMA = 0; // how many shots of the plasmagun
uint8_t INV_BOMBS = 0; // How many bombs do we have
uint8_t INV_EMP = 0; // How many EMPs do we have
uint8_t INV_MEDKIT = 0; // How many medkits do we have?
uint8_t INV_MAGNET = 0; // How many magnets do we have?
uint8_t SELECTED_WEAPON = 0; // 0=none 1=pistol 2=plasmagun
uint8_t SELECTED_ITEM = 0; // 0=none 1=bomb 2=emp 3=medkit 4=magnet
uint8_t SELECT_TIMEOUT = 0; // can only change weapons once it hits zero
uint8_t ANIMATE = 1; // 0=DISABLED 1=ENABLED
uint8_t BIG_EXP_ACT = 0; // 0=No explosion active 1=big explosion active
uint8_t MAGNET_ACT = 0; // 0=no magnet active 1=magnet active
uint8_t PLASMA_ACT = 0; // 0=No plasma fire active 1=plasma fire active
uint8_t RANDOM = 0; // used for random number generation
uint8_t BORDER = 0; // Used for border flash timing
uint8_t SCREEN_SHAKE = 0; // 1=shake 0=no shake
uint8_t CONTROL = 2; // 0=keyboard 1=custom keys 2=snes
uint16_t BORDER_COLOR = 0xf00; // Used for border flash coloring
char INTRO_MESSAGE[] = "welcome to psp-robots!\xff"
                       "by david murray 2021\xff"
                       "psp port by vesa halttunen";
char MSG_CANTMOVE[] = "can't move that!";
char MSG_BLOCKED[] = "blocked!";
char MSG_SEARCHING[] = "searching";
char MSG_NOTFOUND[] = "nothing found here.";
char MSG_FOUNDKEY[] = "you found a key card!";
char MSG_FOUNDGUN[] = "you found a pistol!";
char MSG_FOUNDEMP[] = "you found an emp device!";
char MSG_FOUNDBOMB[] = "you found a timebomb!";
char MSG_FOUNDPLAS[] = "you found a plasma gun!";
char MSG_FOUNDMED[] = "you found a medkit!";
char MSG_FOUNDMAG[] = "you found a magnet!";
char MSG_MUCHBET[] = "ahhh, much better!";
char MSG_EMPUSED[] = "emp activated!\xff"
                     "nearby robots are rebooting.";
char MSG_TERMINATED[] = "you're terminated!";
char MSG_TRANS1[] = "transporter will not activate\xff"
                    "until all robots destroyed.";
char MSG_ELEVATOR[] = "[ elevator panel ]  down\xff"
                      "[  select level  ]  opens";
char MSG_LEVELS[] = "[                ]  door";
char MSG_PAUSED[] = "game paused.\xff"
                    "exit game (y/n)";
char MSG_MUSICON[] = "music on.";
char MSG_MUSICOFF[] = "music off.";
uint8_t SELECTED_MAP = 0;
char MAP_NAMES[] = "01-research lab "
                   "02-headquarters "
                   "03-the village  "
                   "04-the islands  "
                   "05-downtown     "
                   "06-pi university"
                   "07-more islands "
                   "08-robot hotel  "
                   "09-forest moon  "
                   "10-death tower  "
                   "11-river death  "
                   "12-bunker       "
                   "13-castle robot "
                   "14-rocket center"
                   ;
#define MAP_COUNT 14

uint8_t MUSIC_ON = 1; // 0=off 1=on

// Displays loading message for map.
void DISPLAY_LOAD_MESSAGE2()
{
    int Y;
    for (Y = 0; Y != 12; Y++) {
        writeToScreenMemory((PLATFORM_SCREEN_HEIGHT - 32) / 2 / 8 * SCREEN_WIDTH_IN_CHARACTERS + (PLATFORM_SCREEN_WIDTH - 320) / 2 / 8 + Y, LOAD_MSG2[Y]);
    }
    char* name = CALC_MAP_NAME();
    for (Y = 0; Y != 16; Y++) {
        writeToScreenMemory((PLATFORM_SCREEN_HEIGHT - 32) / 2 / 8 * SCREEN_WIDTH_IN_CHARACTERS + (PLATFORM_SCREEN_WIDTH - 320) / 2 / 8 + 12 + Y, name[Y]);
    }
}

char LOAD_MSG2[] = "loading map:";

void SETUP_INTERRUPT()
{
    platform->setInterrupt(&RUNIRQ);
}

// This is the routine that runs every 60 seconds from the IRQ.
// BGTIMER1 is always set to 1 every cycle, after which the main
// program will reset it to 0 when it is done with it's work for
// that cycle.  BGTIMER2 is a count-down to zero and then stays
// there.
void RUNIRQ()
{
    UPDATE_GAME_CLOCK();
    ANIMATE_WATER();
    BGTIMER1 = 1;
    if (BGTIMER2 != 0) {
        BGTIMER2--;
    }
    if (KEYTIMER != 0) {
        KEYTIMER--;
    }
    if (BORDER != 0) {
        BORDER--;
        platform->fadeScreen(15 - BORDER);
    }
    // Back to usual IRQ routine
}
uint8_t BGTIMER1 = 0;
uint8_t BGTIMER2 = 0;
uint8_t KEYTIMER = 0; // Used for repeat of movement

// Since the PET has no real-time clock, and the Jiffy clock
// is a pain to read from assembly language, I have created my own.
void UPDATE_GAME_CLOCK()
{
    if (CLOCK_ACTIVE != 1) {
        return;
    }
    CYCLES++;
    if (CYCLES != platform->framesPerSecond()) { // 60 for ntsc or 50 for pal
        return;
    }
    CYCLES = 0;
    SECONDS++;
    if (SECONDS != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES++;
    if (MINUTES != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES = 0;
    HOURS++;
}

uint8_t HOURS = 0;
uint8_t MINUTES = 0;
uint8_t SECONDS = 0;
uint8_t CYCLES = 0;
uint8_t CLOCK_ACTIVE = 0;

// This routine spaces out the timers so that not everything
// is running out once. It also starts the game_clock.
void SET_INITIAL_TIMERS()
{
    CLOCK_ACTIVE = 1;
    for (int X = 1; X != 48; X++) {
        UNIT_TIMER_A[X] = X;
        UNIT_TIMER_B[X] = 0;
    }
}

void MAIN_GAME_LOOP()
{
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] != 1) { // Is player unit alive
            GAME_OVER();
            return;
        }
        KEY_REPEAT(platform->isKeyOrJoystickPressed(CONTROL == 2 ? true : false));
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(CONTROL == 2 ? true : false);
        // Keyboard controls here.
        if (A != 0xff) {
            KEYTIMER = 5;
            if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                UNIT = 0;
                MOVE_TYPE = 1; // %00000001
                REQUEST_WALK_RIGHT();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_LEFT();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_DOWN();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_UP();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CYCLE_WEAPONS]) {
                CYCLE_WEAPON();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_CYCLE_ITEMS]) {
                CYCLE_ITEM();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_MOVE]) {
                MOVE_OBJECT();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_SEARCH]) {
                SEARCH_OBJECT();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_USE]) {
                USE_ITEM();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_FIRE_UP]) {
                FIRE_UP();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_LEFT]) {
                FIRE_LEFT();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_DOWN]) {
                FIRE_DOWN();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_RIGHT]) {
                FIRE_RIGHT();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_PAUSE]) { // RUN/STOP
                done = PAUSE_GAME();
            } else if (A == KEY_CONFIG[KEY_CHEAT]) { // SHIFT-C
                CHEATER();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_MUSIC]) { // SHIFT-M
                TOGGLE_MUSIC();
                CLEAR_KEY_BUFFER();
            }
            else if (A == KEY_CONFIG[KEY_LIVE_MAP]) {
                TOGGLE_LIVE_MAP();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_LIVE_MAP_ROBOTS]) {
                TOGGLE_LIVE_MAP_ROBOTS();
                CLEAR_KEY_BUFFER();
            }
        }
        // SNES CONTROLLER starts here
        if (B != 0) {
            // first we start with the 4 directional buttons.
            if ((CONTROL == 2 && (B & Platform::JoystickPlay) == 0) ||
                (CONTROL != 2 && (B & Platform::JoystickBlue) == 0)) {
                if (CONTROL != 2 && B & Platform::JoystickRed) {
                    if (B & Platform::JoystickLeft) {
                        FIRE_LEFT();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickRight) {
                        FIRE_RIGHT();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickUp) {
                        FIRE_UP();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickDown) {
                        FIRE_DOWN();
                        KEYTIMER = 20;
                    }
                } else {
                    if (B & Platform::JoystickLeft) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_LEFT();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickRight) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_RIGHT();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickUp) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_UP();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickDown) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_DOWN();
                        AFTER_MOVE_SNES();
                    }
                }
            }
            // Now check for non-repeating buttons
            if (CONTROL == 2) {
                if (B & Platform::JoystickPlay) {
                    if (B & Platform::JoystickReverse) {
                        CYCLE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickForward) {
                        CYCLE_WEAPON();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickLeft) {
                        TOGGLE_LIVE_MAP();
                        CLEAR_KEY_BUFFER();
                    }
                    if (B & Platform::JoystickDown) {
                        TOGGLE_LIVE_MAP_ROBOTS();
                        CLEAR_KEY_BUFFER();
                    }
                    if (B & Platform::JoystickBlue) {
                        done = PAUSE_GAME();
                    }
                    if (B & Platform::JoystickRed) {
                        TOGGLE_MUSIC();
                        CLEAR_KEY_BUFFER();
                    }
                } else {
                    if (B & Platform::JoystickGreen) {
                        FIRE_LEFT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickBlue) {
                        FIRE_RIGHT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickYellow) {
                        FIRE_UP();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickRed) {
                        FIRE_DOWN();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickReverse) {
                        SEARCH_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickForward) {
                        MOVE_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B == Platform::JoystickExtra) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
                }
            } else {
                if (B & Platform::JoystickBlue) {
                    if (B & Platform::JoystickLeft) {
                        CYCLE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickRight) {
                        CYCLE_WEAPON();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickUp) {
                        MOVE_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickDown) {
                        SEARCH_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B == Platform::JoystickBlue) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
                }
            }
        }
    }
}

// This routine handles things that are in common to
// all 4 directions of movement.
void AFTER_MOVE_SNES()
{
    if (MOVE_RESULT == 1) {
        ANIMATE_PLAYER();
    }
    CACULATE_AND_REDRAW();
    if (KEY_FAST == 0) {
        KEYTIMER = 15;
        KEY_FAST = 1;
    } else {
        KEYTIMER = 6;
    }
}

void TOGGLE_MUSIC()
{
    if (MUSIC_ON == 1) {
        PRINT_INFO(MSG_MUSICOFF);
        MUSIC_ON = 0;
        platform->playModule(Platform::ModuleSoundFX);
    } else {
        PRINT_INFO(MSG_MUSICON);
        MUSIC_ON = 1;
        START_IN_GAME_MUSIC();
    }
}

void START_IN_GAME_MUSIC()
{
    platform->playModule(MUSIC_ON == 1 ? LEVEL_MUSIC[SELECTED_MAP] : Platform::ModuleSoundFX);
}

Platform::Module LEVEL_MUSIC[] = {
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2
};

// TEMP ROUTINE TO GIVE ME ALL ITEMS AND WEAPONS
void CHEATER()
{
    PLAY_SOUND(12);
    KEYS = 7;
    AMMO_PISTOL = 100;
    AMMO_PLASMA = 100;
    INV_BOMBS = 100;
    INV_EMP = 100;
    INV_MEDKIT = 100;
    INV_MAGNET = 100;
    SELECTED_WEAPON = 1;
    SELECTED_ITEM = 1;
    REDRAW_WINDOW = 1;
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    DISPLAY_ITEM();
}

bool PAUSE_GAME()
{
    PLAY_SOUND(15);
    // pause clock
    CLOCK_ACTIVE = 0;
    // display message to user
    SCROLL_INFO();
    PRINT_INFO(MSG_PAUSED);
//    for (BGTIMER1 = 0; BGTIMER1 != 1;); // to prevent double-tap of run/stop
    CLEAR_KEY_BUFFER();
    platform->renderFrame();
    while (!platform->quit) {
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(CONTROL == 2 ? true : false);
        if (A == KEY_CONFIG[KEY_PAUSE] || // RUN/STOP
            A == KEY_CONFIG[KEY_NO] ||
            (B & Platform::JoystickBlue)) { // N-KEY
            SCROLL_INFO();
            SCROLL_INFO();
            SCROLL_INFO();
            CLEAR_KEY_BUFFER();
            CLOCK_ACTIVE = 1;
            PLAY_SOUND(15);
            return false;
        } else if (A == KEY_CONFIG[KEY_YES] || (B & Platform::JoystickRed)) { // Y-KEY
            UNIT_TYPE[0] = 0; // make player dead
            PLAY_SOUND(15);
            GOM4();
            return true;
        }
        platform->renderFrame(true);
    }
    return false;
}

void CLEAR_KEY_BUFFER()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    KEYTIMER = 20;
}

void USE_ITEM()
{
    // check select timeout to prevent accidental double-tap
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    // First figure out which item to use.
    switch (SELECTED_ITEM) {
    case 1: // BOMB
        USE_BOMB();
        break;
    case 2: // EMP
        USE_EMP();
        break;
    case 3: // MEDKIT
        USE_MEDKIT();
        break;
    case 4: // MAGNET
        USE_MAGNET();
        break;
    default:
        break;
    }
}

void USE_BOMB()
{
    platform->setCursorShape(Platform::ShapeUse);
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        // Now scan for any units at that location:
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) { // 255 means no unit found.
            for (int X = 28; X != 32; X++) { // Start of weapons units
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 6; // bomb AI
                    UNIT_TILE[X] = 130; // bomb tile
                    UNIT_LOC_X[X] = MAP_X;
                    UNIT_LOC_Y[X] = MAP_Y;
                    UNIT_TIMER_A[X] = 100; // How long until explosion?
                    UNIT_A[X] = 0;
                    INV_BOMBS--;
                    DISPLAY_ITEM();
                    REDRAW_WINDOW = 1;
                    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used, pet version only
                    PLAY_SOUND(6); // SOUND PLAY
                    return;
                }
            }
            return; // no slots available right now, abort.
        }
    }
    BOMB_MAGNET_COMMON2();
}

void USE_MAGNET()
{
    if (MAGNET_ACT != 0) { // only one magnet active at a time.
        return;
    }
    platform->setCursorShape(Platform::ShapeUse);
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 20; // MAGNET AI
                UNIT_TILE[X] = 134; // MAGNET tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 1; // How long until ACTIVATION
                UNIT_TIMER_B[X] = 255; // how long does it live -A
                UNIT_A[X] = 3; // how long does it live -B
                MAGNET_ACT = 1; // only one magnet allowed at a time.
                INV_MAGNET--;
                DISPLAY_ITEM();
                REDRAW_WINDOW = 1;
                PLAY_SOUND(6); // move sound, SOUND PLAY
                return;
            }
        }
        return; // no slots available right now, abort.
    }
    BOMB_MAGNET_COMMON2();
}

bool BOMB_MAGNET_COMMON1()
{
    platform->hideCursor();
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x01) == 0x01; // %00000001 is that spot available for something to move onto it?
}

void BOMB_MAGNET_COMMON2()
{
    PRINT_INFO(MSG_BLOCKED);
    PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
}

void USE_EMP()
{
    EMP_FLASH();
//    REDRAW_WINDOW = 0;  // attempt to delay window redrawing (pet only)
    PLAY_SOUND(3);  // EMP sound, SOUND PLAY
    INV_EMP--;
    DISPLAY_ITEM();
    for (int X = 1; X != 28; X++) { // start with unit#1 (skip player)
        if (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
            UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
            UNIT_LOC_X[X] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) &&  // NOW CHECK VERTICAL
            UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
            UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1)) {
            UNIT_TIMER_A[X] = 255;
            // test to see if unit is above water
            MAP_X = UNIT_LOC_X[X];
            MAP_Y = UNIT_LOC_Y[X];
            GET_TILE_FROM_MAP();
            if (TILE == 204) {  // WATER
                UNIT_TYPE[X] = 5;
                UNIT_TIMER_A[X] = 5;
                UNIT_TIMER_B[X] = 3;
                UNIT_A[X] = 60; // how long to show sparks.
                UNIT_TILE[X] = 140; // Electrocuting tile
            }
        }
    }
    PRINT_INFO(MSG_EMPUSED);
    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used
}

void USE_MEDKIT()
{
    if (UNIT_HEALTH[0] == 12) {    // Do we even need the medkit?
        return;
    }
    // Now figure out how many HP we need to be healthy.
    TEMP_A = 12 - UNIT_HEALTH[0];  // how many we need.
    if (INV_MEDKIT >= TEMP_A) {
        // we had more than we need, so go to full health.
        UNIT_HEALTH[0] = 12;
        INV_MEDKIT -= TEMP_A;
    } else {
        // we had less than we need, so we'll use what is available.
        UNIT_HEALTH[0] += INV_MEDKIT;
        INV_MEDKIT = 0;
    }
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_ITEM();
    PLAY_SOUND(2); // MEDKIT SOUND, SOUND PLAY
    PRINT_INFO(MSG_MUCHBET);
}

void FIRE_UP()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_UP_PISTOL();
    } else {
        FIRE_UP_PLASMA();
    }
    UNIT_DIRECTION[0] = 0;
}

void FIRE_UP_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_UP_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_DOWN_PISTOL();
    } else {
        FIRE_DOWN_PLASMA();
    }
    UNIT_DIRECTION[0] = 1;
}

void FIRE_DOWN_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_LEFT_PISTOL();
    } else {
        FIRE_LEFT_PLASMA();
    }
    UNIT_DIRECTION[0] = 2;
}

void FIRE_LEFT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_RIGHT_PISTOL();
    } else {
        FIRE_RIGHT_PLASMA();
    }
    UNIT_DIRECTION[0] = 3;
}

void FIRE_RIGHT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void AFTER_FIRE(int X)
{
    UNIT_TIMER_A[X] = 0;
    UNIT_LOC_X[X] = UNIT_LOC_X[0];
    UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
    UNIT = X;
    if (SELECTED_WEAPON != 2) {
        PLAY_SOUND(9); // PISTOL-SOUND, SOUND PLAY
        AMMO_PISTOL--;
        DISPLAY_WEAPON();
    } else {
        PLAY_SOUND(8); // PLASMA-GUN-SOUND, SOUND PLAY
        AMMO_PLASMA--;
        DISPLAY_WEAPON();
    }
}

// This routine checks KEYTIMER to see if it has
// reached zero yet.  If so, it clears the LSTX
// variable used by the kernal, so that it will
// register a new keypress.
void KEY_REPEAT(bool keyDown)
{
    if (KEYTIMER != 0) {
        return;
    }
    if (keyDown) { // no key pressed
        platform->keyRepeat(); // clear LSTX register
        KEYTIMER = 6;
    } else {
        // No key pressed, reset all to defaults
        KEY_FAST = 0;
        KEYTIMER = 6;
    }
}

// This routine handles things that are in common to
// all 4 directions of movement.
void AFTER_MOVE()
{
    if (MOVE_RESULT == 1) {
        ANIMATE_PLAYER();
    }
    CACULATE_AND_REDRAW();
    // now reset key-repeat rate
    if (KEY_FAST == 0) {
        // FIRST REPEAT
        KEYTIMER = 13;
        KEY_FAST++;
    } else {
        // SUBSEQUENT REPEATS
        KEYTIMER = 6;
    }
}

uint8_t KEY_FAST = 0; // 0=DEFAULT STATE

// This routine is invoked when the user presses S to search
// an object such as a crate, chair, or plant.
void SEARCH_OBJECT()
{
    platform->setCursorShape(Platform::ShapeSearch);
    USER_SELECT_OBJECT();
    REDRAW_WINDOW = 1;
    // first check of object is searchable
    CALC_COORDINATES();
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x40) == 0) { // %01000000 can search attribute
        platform->hideCursor();
    } else {
        // is the tile a crate?
        if (TILE == 41 || TILE == 45 || TILE == 199) { // BIG CRATE / small CRATE / "Pi" CRATE
            TILE = DESTRUCT_PATH[TILE];
            PLOT_TILE_TO_MAP();
        }
        // Now check if there is an object there.
        PRINT_INFO(MSG_SEARCHING);
        for (SEARCHBAR = 0; SEARCHBAR != 8; SEARCHBAR++) {
            for (BGTIMER2 = 18; BGTIMER2 != 0;) { // delay time between search periods
                if (BGTIMER1 != 1) {
                    platform->renderFrame(true);
                }
                PET_SCREEN_SHAKE();
                BACKGROUND_TASKS();
            }
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 9 + SEARCHBAR, 46); // PERIOD
        }
        platform->hideCursor();
        CALC_COORDINATES();
        CHECK_FOR_HIDDEN_UNIT();
        if (UNIT_FIND == 255) {
            PRINT_INFO(MSG_NOTFOUND);
            return;
        }
        TEMP_A = UNIT_TYPE[UNIT_FIND];  // store object type
        TEMP_B = UNIT_A[UNIT_FIND]; // store secondary info
        UNIT_TYPE[UNIT_FIND] = 0; // DELETE ITEM ONCE FOUND
        // ***NOW PROCESS THE ITEM FOUND***
        PLAY_SOUND(10); // ITEM-FOUND-SOUND, SOUND PLAY
        if (TEMP_A == 128) {    // key
            if (TEMP_B == 0) {  // WHICH SORT OF KEY?
                KEYS |= 0x01; // %00000001 Add spade key
            } else if (TEMP_B == 1) {
                KEYS |= 0x02; // %00000010 Add heart key
            } else {
                KEYS |= 0x04; // %00000100 Add star key
            }
            PRINT_INFO(MSG_FOUNDKEY);
            DISPLAY_KEYS();
        } else if (TEMP_A == 129) { // TIME BOMB
            INV_BOMBS += TEMP_B;
            PRINT_INFO(MSG_FOUNDBOMB);
            DISPLAY_ITEM();
        } else if (TEMP_A == 130) { // EMP
            INV_EMP += TEMP_B;
            PRINT_INFO(MSG_FOUNDEMP);
            DISPLAY_ITEM();
        } else if (TEMP_A == 131) { // PISTOL
            if (AMMO_PISTOL + TEMP_B > 255) { // If we rolled over past 255
                AMMO_PISTOL = 255; // set it to 255.
            } else {
                AMMO_PISTOL += TEMP_B;
            }
            PRINT_INFO(MSG_FOUNDGUN);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 132) { // PLASMA GUN
            AMMO_PLASMA += TEMP_B;
            PRINT_INFO(MSG_FOUNDPLAS);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 133) { // MEDKIT
            INV_MEDKIT += TEMP_B;
            PRINT_INFO(MSG_FOUNDMED);
            DISPLAY_ITEM();
        } else if (TEMP_A == 134) { // MAGNET
            INV_MAGNET += TEMP_B;
            PRINT_INFO(MSG_FOUNDMAG);
            DISPLAY_ITEM();
        }
        // ADD CODE HERE FOR OTHER OBJECT TYPES
    }
}

uint8_t SEARCHBAR = 0; // to count how many periods to display.

// combines cursor location with window location
// to determine coordinates for MAP_X and MAP_Y
void CALC_COORDINATES()
{
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
}

// This routine is called by routines such as the move, search,
// or use commands.  It displays a cursor and allows the user
// to pick a direction of an object.
void USER_SELECT_OBJECT()
{
    PLAY_SOUND(16); // beep sound, SOUND PLAY
#if (MAP_WINDOW_SIZE == 77)
    CURSOR_X = PLATFORM_MAP_WINDOW_TILES_WIDTH / 2;
    CURSOR_Y = PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2;
#else
    CURSOR_X = UNIT_LOC_X[0] - MAP_WINDOW_X;
    CURSOR_Y = UNIT_LOC_Y[0] - MAP_WINDOW_Y;
#endif
    if (LIVE_MAP_ON == 0) {
        platform->showCursor(CURSOR_X, CURSOR_Y);
    }
    // First ask user which object to move
    while (!platform->quit) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
            platform->hideCursor();
            return;
        }
        uint8_t A = platform->readKeyboard();
        // SNES controls for this routine
        uint16_t B = platform->readJoystick(CONTROL == 2 ? true : false);
        if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
            UNIT_DIRECTION[0] = 3;
            CURSOR_X++;
            if (LIVE_MAP_ON == 0) {
                platform->showCursor(CURSOR_X, CURSOR_Y);
            }
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
            UNIT_DIRECTION[0] = 2;
            CURSOR_X--;
            if (LIVE_MAP_ON == 0) {
                platform->showCursor(CURSOR_X, CURSOR_Y);
            }
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
            UNIT_DIRECTION[0] = 1;
            CURSOR_Y++;
            if (LIVE_MAP_ON == 0) {
                platform->showCursor(CURSOR_X, CURSOR_Y);
            }
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
            UNIT_DIRECTION[0] = 0;
            CURSOR_Y--;
            if (LIVE_MAP_ON == 0) {
                platform->showCursor(CURSOR_X, CURSOR_Y);
            }
            return;
        }
    }
}

void MOVE_OBJECT()
{
    platform->setCursorShape(Platform::ShapeMove);
    USER_SELECT_OBJECT();
    // now test that object to see if it
    // is allowed to be moved.
    platform->hideCursor();
    CALC_COORDINATES();
    CHECK_FOR_HIDDEN_UNIT();
    MOVTEMP_U = UNIT_FIND;
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x04) == 0) { // can it be moved?
        PRINT_INFO(MSG_CANTMOVE);
        PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
        return;
    }
    MOVTEMP_O = TILE; // Store which tile it is we are moving
    MOVTEMP_X = MAP_X; // Store original location of object
    MOVTEMP_Y = MAP_Y;
    if (LIVE_MAP_ON == 0) {
        platform->showCursor(CURSOR_X, CURSOR_Y);
    }
    // NOW ASK THE USER WHICH DIRECTION TO MOVE IT TO
    while (!platform->quit) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
            platform->hideCursor();
            return;
        }
        // keyboard control
        uint8_t A = platform->readKeyboard();
        // SNES controls
        uint16_t B = platform->readJoystick(CONTROL == 2 ? true : false);
        if (A != 0xff || B != 0) {
            if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
                CURSOR_X++;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
                CURSOR_X--;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                CURSOR_Y++;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
                CURSOR_Y--;
                break;
            }
        }
    }
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    platform->hideCursor();
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    if (TILE_ATTRIB[TILE] & 0x20) { // %00100000 is that spot available for something to move onto it?
        // Now scan for any units at that location:
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) { // 255 means no unit found.
            PLAY_SOUND(6); // move sound, SOUND PLAY
            MOVTEMP_D = MAP_SOURCE[0]; // Grab current object
            MAP_SOURCE[0] = MOVTEMP_O; // replace with obect we are moving
            MAP_X = MOVTEMP_X; // RETRIEVE original location of object
            MAP_Y = MOVTEMP_Y;
            GET_TILE_FROM_MAP();
            int A = MOVTEMP_D;
            if (A == 148) { // trash compactor tile
                A = 9; // Floor tile
            }
            MAP_SOURCE[0] = A; // Replace former location
            REDRAW_WINDOW = 1; // See the result
            if (MOVTEMP_U == 255) {
                if (LIVE_MAP_ON == 1) {
                    platform->renderLiveMapTile(MAP, MOVTEMP_X, MOVTEMP_Y);
                    platform->renderLiveMapTile(MAP, MOVTEMP_UX, MOVTEMP_UY);
                }
                return;
            }
            UNIT_LOC_X[MOVTEMP_U] = MOVTEMP_UX;
            UNIT_LOC_Y[MOVTEMP_U] = MOVTEMP_UY;
            return;
        }
    }
    PRINT_INFO(MSG_BLOCKED);
    PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
}

uint8_t MOVTEMP_O = 0; // origin tile
uint8_t MOVTEMP_D = 0; // destination tile
uint8_t MOVTEMP_X = 0; // x-coordinate
uint8_t MOVTEMP_Y = 0; // y-coordinate
uint8_t MOVTEMP_U = 0; // unit number (255=none)
uint8_t MOVTEMP_UX = 0;
uint8_t MOVTEMP_UY = 0;

void CACULATE_AND_REDRAW()
{
#if (MAP_WINDOW_SIZE == 77)
    MAP_WINDOW_X = UNIT_LOC_X[0] - PLATFORM_MAP_WINDOW_TILES_WIDTH / 2; // no index needed since it's player unit
    MAP_WINDOW_Y = UNIT_LOC_Y[0] - PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2; // no index needed since it's player unit
#else
    MAP_WINDOW_X = MIN(MAX(UNIT_LOC_X[0] - PLATFORM_MAP_WINDOW_TILES_WIDTH / 2, 0), 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH); // no index needed since it's player unit
    MAP_WINDOW_Y = MIN(MAX(UNIT_LOC_Y[0] - PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2, 0), 64 - PLATFORM_MAP_WINDOW_TILES_HEIGHT); // no index needed since it's player unit
#endif
    REDRAW_WINDOW = 1;
}

// This routine checks all units from 0 to 31 and figures out if it should be dislpayed
// on screen, and then grabs that unit's tile and stores it in the MAP_PRECALC array
// so that when the window is drawn, it does not have to search for units during the
// draw, speeding up the display routine.
void MAP_PRE_CALCULATE()
{
    // CLEAR OLD BUFFER
    for (int Y = 0; Y != MAP_WINDOW_SIZE; Y++) {
        MAP_PRECALC[Y] = 0;
    }
    for (int X = 0; X != 32; X++) {
        if (X == 0 || // skip the check for unit zero, always draw it.
            (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
             UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
             UNIT_LOC_X[X] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) && // NOW CHECK VERTICAL
             UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
             UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1))) {
            // Unit found in map window, now add that unit's
            // tile to the precalc map.
#if (MAP_WINDOW_SIZE == 77)
            int Y = UNIT_LOC_X[X] - MAP_WINDOW_X + PRECALC_ROWS[UNIT_LOC_Y[X] - MAP_WINDOW_Y];
#else
            int Y = UNIT_LOC_X[X] - MAP_WINDOW_X + (UNIT_LOC_Y[X] - MAP_WINDOW_Y) * PLATFORM_MAP_WINDOW_TILES_WIDTH;
#endif
            if (UNIT_TILE[X] == 130 || UNIT_TILE[X] == 134) { // is it a bomb, is it a magnet?
                // What to do in case of bomb or magnet that should
                // go underneath the unit or robot.
                if (MAP_PRECALC[Y] != 0) {
                    continue;
                }
            }
            MAP_PRECALC[Y] = UNIT_TILE[X];
            MAP_PRECALC_DIRECTION[Y] = UNIT_DIRECTION[X];
            MAP_PRECALC_TYPE[Y] = UNIT_A[X];
        }
        // continue search
    }
}

#if (MAP_WINDOW_SIZE == 77)
uint8_t PRECALC_ROWS[] = { 0,11,22,33,44,55,66 };
#endif

// This routine is where the MAP is displayed on the screen
void INVALIDATE_PREVIOUS_MAP()
{
    for (int i = 0; i < MAP_WINDOW_SIZE; i++) {
        PREVIOUS_MAP_BACKGROUND[i] = 255;
    }
}

void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    MAP_SOURCE = MAP + ((MAP_WINDOW_Y << 7) + MAP_WINDOW_X);
#if (MAP_WINDOW_SIZE == 77)
    for (uint8_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != PLATFORM_MAP_WINDOW_TILES_HEIGHT; TEMP_Y++, MAP_SOURCE += 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH) {
        for (uint8_t TEMP_X = 0; TEMP_X != PLATFORM_MAP_WINDOW_TILES_WIDTH; TEMP_X++, MAP_SOURCE++, PRECALC_COUNT++) {
#else
    for (uint16_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != PLATFORM_MAP_WINDOW_TILES_HEIGHT; TEMP_Y++, MAP_SOURCE += 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH, PRECALC_COUNT += PLATFORM_MAP_WINDOW_TILES_WIDTH - PLATFORM_MAP_WINDOW_TILES_WIDTH) {
        for (uint16_t TEMP_X = 0; TEMP_X != PLATFORM_MAP_WINDOW_TILES_WIDTH; TEMP_X++, MAP_SOURCE++, PRECALC_COUNT++) {
#endif
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
            TILE = MAP_SOURCE[0];
            uint8_t VARIANT = 0;
            switch (TILE) {
            case 204: // WATER
            case 66:  // FLAG
            case 148: // TRASH COMPACTOR
            case 143: // SERVER
                VARIANT = ANIM_STATE & 3;
                break;
            case 196: // HVAC
            case 197:
            case 200:
            case 201:
                VARIANT = ANIM_STATE & 1;
                break;
            case 20: // CINEMA
            case 21:
            case 22:
                VARIANT = ANIM_STATE;
                break;
            default:
                break;
            }
            uint8_t FG_TILE = MAP_PRECALC[PRECALC_COUNT];
            uint8_t FG_VARIANT = 0;
            if (FG_TILE != 0) {
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
                if (FG_TILE == 96 || (FG_TILE >= 100 && FG_TILE <= 103)) { // PLAYER OR EVILBOT
                    if (DIRECTION == 0) {
                        FG_VARIANT = 8;
                    } else if (DIRECTION == 2) {
                        FG_VARIANT = 12;
                    } else if (DIRECTION == 3) {
                        FG_VARIANT = 4;
                    }
                    if (FG_TILE == 96) {
                        FG_VARIANT += WALK_FRAME + (SELECTED_WEAPON << 4);
                    }
                } else if (FG_TILE == 243) { // TRANSPORT
                    if (DEMATERIALIZE_FRAME < 7) {
                        FG_VARIANT = DEMATERIALIZE_FRAME;
                    } else {
                        FG_TILE = 0;
                    }
                } else if (FG_TILE == 115) { // DEAD ROBOT
                    switch (MAP_PRECALC_TYPE[PRECALC_COUNT]) {
                    case 17:
                    case 18:
                        FG_VARIANT = 1;
                        break;
                    case 9:
                        FG_VARIANT = 2;
                        break;
                    default:
                        break;
                    }
                }
            }
            if (TILE != PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] ||
                VARIANT != PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] ||
                FG_TILE != PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] ||
                FG_VARIANT != PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT]) {
                if (FG_TILE != 0) {
                    platform->renderTiles(TILE, FG_TILE, TEMP_X * 24, TEMP_Y * 24, VARIANT, FG_VARIANT);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] = VARIANT;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;
                } else {
                    platform->renderTile(TILE, TEMP_X * 24, TEMP_Y * 24, VARIANT);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] = VARIANT;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;

                    switch (TILE) {
                    case 20: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 1, CINEMA_MESSAGE[CINEMA_STATE], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 2, CINEMA_MESSAGE[CINEMA_STATE + 1], 1, 0);
                        break;
                    }
                    case 21: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 0, CINEMA_MESSAGE[CINEMA_STATE + 2], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 1, CINEMA_MESSAGE[CINEMA_STATE + 3], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 2, CINEMA_MESSAGE[CINEMA_STATE + 4], 1, 0);
                        break;
                    }
                    case 22: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 0, CINEMA_MESSAGE[CINEMA_STATE + 5], 1, 0);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void TOGGLE_LIVE_MAP()
{
    if (LIVE_MAP_ON != 1) {
        LIVE_MAP_ON = 1;

        platform->renderLiveMap(MAP);
    } else {
        LIVE_MAP_ON = 0;

        INVALIDATE_PREVIOUS_MAP();
    }
    REDRAW_WINDOW = 1;
}

void TOGGLE_LIVE_MAP_ROBOTS()
{
    LIVE_MAP_ROBOTS_ON = LIVE_MAP_ROBOTS_ON == 1 ? 0 : 1;
}

void DRAW_LIVE_MAP()
{
    platform->renderLiveMapUnits(MAP, UNIT_TYPE, UNIT_LOC_X, UNIT_LOC_Y, LIVE_MAP_PLAYER_BLINK < 128 ? 1 : 0, LIVE_MAP_ROBOTS_ON == 1 ? true : false);

    LIVE_MAP_PLAYER_BLINK += 10;
}

uint8_t LIVE_MAP_ON = 0;
uint8_t LIVE_MAP_ROBOTS_ON = 0;
uint8_t LIVE_MAP_PLAYER_BLINK = 0;

// This routine plots a 3x3 tile from the tile database anywhere
// on screen.  But first you must define the tile number in the
// TILE variable, as well as the starting screen address must
// be defined in $FB.
void PLOT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
    platform->renderTile(TILE, x * 24, y * 24);
}

// This routine plots a transparent tile from the tile database
// anywhere on screen.  But first you must define the tile number
// in the TILE variable, as well as the starting screen address must
// be defined in $FB.  Also, this routine is slower than the usual
// tile routine, so is only used for sprites.  The ":" character ($3A)
// is not drawn.
void PLOT_TRANSPARENT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
    uint8_t variant = 0;
    if (TILE == 96 || (TILE >= 100 && TILE <= 103)) {
        if (DIRECTION == 0) {
            variant = 8;
        } else if (DIRECTION == 2) {
            variant = 12;
        } else if (DIRECTION == 3) {
            variant = 4;
        }
        if (TILE == 96) {
            variant += WALK_FRAME + (SELECTED_WEAPON << 4);
        }
    }
    platform->renderTile(TILE, x * 24, y * 24, variant, true);
}

// This routine checks to see if UNIT is occupying any space
// that is currently visible in the window.  If so, the
// flag for redrawing the window will be set.
void CHECK_FOR_WINDOW_REDRAW()
{
    if (UNIT_LOC_X[UNIT] >= MAP_WINDOW_X && // FIRST CHECK HORIZONTAL
        UNIT_LOC_X[UNIT] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) &&
        UNIT_LOC_Y[UNIT] >= MAP_WINDOW_Y && // NOW CHECK VERTICAL
        UNIT_LOC_Y[UNIT] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1)) {
        REDRAW_WINDOW = 1;
    }
}

void DECWRITE(uint16_t destination, uint8_t color)
{
    for (int X = 2; X >= 0; X--) {
        writeToScreenMemory(destination + X, 0x30 + (DECNUM % 10), color);
        DECNUM /= 10;
    }
}

// The following routine loads the tileset from disk
void TILE_LOAD_ROUTINE()
{
    uint8_t* tileset = platform->loadTileset(TILENAME);
    DESTRUCT_PATH = tileset + 2 + 0 * 256;
    TILE_ATTRIB = tileset + 2 + 1 * 256;
    platform->generateTiles(0, TILE_ATTRIB);
}

// The following routine loads the map from disk
void MAP_LOAD_ROUTINE()
{
    platform->load(MAPNAME, UNIT_TYPE, 8960);
}

void DISPLAY_GAME_SCREEN()
{
    platform->displayImage(Platform::ImageGame);

    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 6, 0x71, 15);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 5, 0x71, 15);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 4, 0x71, 12);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 3, 0x71, 12);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 2, 0x71, 9);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 1, 0x71, 9);
}

char INTRO_OPTIONS[] = "start game"
                       "select map"
                       "difficulty"
                       "controls  ";

void DISPLAY_INTRO_SCREEN()
{
    uint8_t* row = SCREEN_MEMORY + MENU_CHART[0];
    for (int Y = 0, i = 0; Y < 3; Y++, row += SCREEN_WIDTH_IN_CHARACTERS) {
        for (int X = 0; X < 10; X++, i++) {
            row[X] = INTRO_OPTIONS[i];
        }
    }
    platform->displayImage(Platform::ImageIntro);
}

void DISPLAY_ENDGAME_SCREEN()
{
    int X;
    platform->displayImage(Platform::ImageGameOver);
    // display map name
    char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
        writeToScreenMemory(7 * SCREEN_WIDTH_IN_CHARACTERS + 22 + Y, name[Y], 4);
    }
    // display elapsed time
    DECNUM = HOURS;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 21, 4);
    DECNUM = MINUTES;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 24, 4);
    DECNUM = SECONDS;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 27, 4);
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 21, 32, 4); // SPACE
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 24, 58, 4); // COLON
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 27, 58, 4);
    // count robots remaining
    DECNUM = 0;
    for (X = 1; X != 28; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS + 22, 4);
    // Count secrets remaining
    DECNUM = 0;
    for (X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS + 22, 4);
    // display difficulty level
    char* WORD = DIFF_LEVEL_WORDS + (DIFF_LEVEL * 6);
    for (X = 0; X < 6; X++) {
        writeToScreenMemory(15 * SCREEN_WIDTH_IN_CHARACTERS + 22 + X, WORD[X], 4);
    }
}

char DIFF_LEVEL_WORDS[] = "easy  "
                          "normal"
                          "hard  "
;

void DECOMPRESS_SCREEN(uint8_t* source, uint8_t color)
{
    uint16_t destination = 0;

    while (!platform->quit) {
        if (*source != 96) { // REPEAT FLAG
            writeToScreenMemory(destination, *source, color);
        } else {
            // REPEAT CODE
            source++;
            RPT = *source;
            source++;
            for (int X = *source; X >= 0; X--) {
                writeToScreenMemory(destination++, RPT, color);
            }
            destination--;
        }
        // CHECK TO SEE IF WE REACHED $83E7 YET.
        if (destination == 0x3E7) {
            return;
        }
        source++;
        destination++;
    }
}

uint8_t RPT = 0; // repeat value

void DISPLAY_PLAYER_HEALTH()
{
    TEMP_A = UNIT_HEALTH[0] >> 1; // No index needed because it is the player, divide by two
    int Y = 0;
    while (Y != TEMP_A) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x66); // GRAY BLOCK
    }
    if (UNIT_HEALTH[0] & 0x01) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x5C); // HALF GRAY BLOCK
    }
    while (Y != 6) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x20); // SPACE
    }

    int health = 5 - MIN(TEMP_A, 5);
    platform->renderHealth(health, PLATFORM_SCREEN_WIDTH - 48, 131 + (health >> 1));
}

void CYCLE_ITEM()
{
    PLAY_SOUND(13); // CHANGE-ITEM-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_ITEM++;
    if (SELECTED_ITEM == 5) {
        SELECTED_ITEM = 0;
    }
    DISPLAY_ITEM();
}

void DISPLAY_ITEM()
{
    PRESELECT_ITEM();
    while (SELECTED_ITEM != 0) {
        if (SELECTED_ITEM == 5) { // number too high!
            SELECTED_ITEM = 0;
            return;
        } else if (SELECTED_ITEM == 1) { // bomb
            if (INV_BOMBS != 0) { // did we run out?
                DISPLAY_TIMEBOMB();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 2) { // emp
            if (INV_EMP != 0) { // did we run out?
                DISPLAY_EMP();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 3) { // medkit
            if (INV_MEDKIT != 0) { // did we run out?
                DISPLAY_MEDKIT();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 4) { // magnet
            if (INV_MAGNET != 0) { // did we run out?
                DISPLAY_MAGNET();
                return;
            }
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        } else {
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        }
    }
    // no items to show
    // add routine to draw blank space
}

// This routine checks to see if currently selected
// item is zero.  And if it is, then it checks inventories
// of other items to decide which item to automatically
// select for the user.
void PRESELECT_ITEM()
{
    if (SELECTED_ITEM != 0) { // If item already selected, return
        return;
    }
    if (INV_BOMBS != 0) {
        SELECTED_ITEM = 1; // BOMB
        return;
    }
    if (INV_EMP != 0) {
        SELECTED_ITEM = 2; // EMP
        return;
    }
    if (INV_MEDKIT != 0) {
        SELECTED_ITEM = 3; // MEDKIT
        return;
    }
    if (INV_MAGNET != 0) {
        SELECTED_ITEM = 4; // MAGNET
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_ITEM = 0; // nothing in inventory
    DISPLAY_BLANK_ITEM();
}

void DISPLAY_TIMEBOMB()
{
    platform->renderItem(5, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_BOMBS;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_EMP()
{
    platform->renderItem(3, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_EMP;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_MEDKIT()
{
    platform->renderItem(2, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_MEDKIT;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_MAGNET()
{
    platform->renderItem(4, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_MAGNET;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_BLANK_ITEM()
{
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 48, 48, 40);
}

void CYCLE_WEAPON()
{
    PLAY_SOUND(12); // CHANGE WEAPON-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_WEAPON++;
    if (SELECTED_WEAPON != 2) {
        SELECTED_WEAPON = 0;
    }
    REDRAW_WINDOW = 1;
    DISPLAY_WEAPON();
}

void DISPLAY_WEAPON()
{
    while (!platform->quit) {
        PRESELECT_WEAPON();
        if (SELECTED_WEAPON == 0) { // no weapon to show
            // add routine to draw blank space
            return;
        } else if (SELECTED_WEAPON == 1) { // PISTOL
            if (AMMO_PISTOL != 0) { // did we run out?
                DISPLAY_PISTOL();
                return;
            }
            SELECTED_WEAPON = 0;
        } else if (SELECTED_WEAPON == 2) { // PLASMA GUN
            if (AMMO_PLASMA != 0) { // did we run out?
                DISPLAY_PLASMA_GUN();
                return;
            }
            SELECTED_WEAPON = 0;
        } else {
            SELECTED_WEAPON = 0; // should never happen
        }
    }
}

// This routine checks to see if currently selected
// weapon is zero.  And if it is, then it checks inventories
// of other weapons to decide which item to automatically
// select for the user.
void PRESELECT_WEAPON()
{
    if (SELECTED_WEAPON != 0) { // If item already selected, return
        return;
    }
    if (AMMO_PISTOL != 0) {
        SELECTED_WEAPON = 1; // PISTOL
        return;
    }
    if (AMMO_PLASMA != 0) {
        SELECTED_WEAPON = 2; // PLASMAGUN
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_WEAPON = 0; // nothing in inventory
    DISPLAY_BLANK_WEAPON();
}

void DISPLAY_PLASMA_GUN()
{
    platform->renderItem(1, PLATFORM_SCREEN_WIDTH - 48, 13);
    DECNUM = AMMO_PLASMA;
    DECWRITE(5 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_PISTOL()
{
    platform->renderItem(0, PLATFORM_SCREEN_WIDTH - 48, 13);
    DECNUM = AMMO_PISTOL;
    DECWRITE(5 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
}

void DISPLAY_BLANK_WEAPON()
{
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 8, 48, 32);
}

void DISPLAY_KEYS()
{
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 106, 48, 14); // ERASE ALL 3 SPOTS
    if (KEYS & 0x01) { // %00000001 Spade key
        platform->renderKey(0, PLATFORM_SCREEN_WIDTH - 48, 106);
    }
    if (KEYS & 0x02) { // %00000010 heart key
        platform->renderKey(1, PLATFORM_SCREEN_WIDTH - 32, 106);
    }
    if (KEYS & 0x04) { // %00000100 star key
        platform->renderKey(2, PLATFORM_SCREEN_WIDTH - 16, 106);
    }
}

void GAME_OVER()
{
    platform->renderFrame();
    // stop game clock
    CLOCK_ACTIVE = 0;
    // disable music
    platform->pauseModule();
    // Did player die or win?
    if (UNIT_TYPE[0] == 0) {
        UNIT_TILE[0] = 111; // // dead player tile
        KEYTIMER = 100;
    }
    while (KEYTIMER != 0) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
    }
    for (int X = 0; X != 11; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER1[X]);
        writeToScreenMemory(((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 + 1) * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER2[X]);
        writeToScreenMemory(((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 + 2) * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER3[X]);
    }
    platform->renderFrame();
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    platform->stopModule();
    if (MUSIC_ON == 1) {
        platform->loadModule(UNIT_TYPE[0] != 0 ? Platform::ModuleWin : Platform::ModuleLose);
    }
    while (platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL == 2 ? true : false) == 0 && !platform->quit) {
        platform->renderFrame(true);
    }
    GOM4();
}

void GOM4()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    platform->stopModule();
    platform->startFadeScreen(0x000, 15);
    platform->fadeScreen(0, false);
    DISPLAY_ENDGAME_SCREEN();
    DISPLAY_WIN_LOSE();
    platform->renderFrame();
    platform->fadeScreen(15, false);
    while (platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL == 2 ? true : false) == 0 && !platform->quit) {
        platform->renderFrame(true);
    }
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    platform->stopModule();
}

uint8_t GAMEOVER1[] = { 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x6e };
uint8_t GAMEOVER2[] = { 0x5d, 0x07, 0x01, 0x0d, 0x05, 0x20, 0x0f, 0x16, 0x05, 0x12, 0x5d };
uint8_t GAMEOVER3[] = { 0x6d, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7d };

void DISPLAY_WIN_LOSE()
{
    STOP_SONG();
    if (UNIT_TYPE[0] != 0) {
        // WIN MESSAGE
        for (int X = 0; X != 8; X++) {
            writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS + 16 + X, WIN_MSG[X], 4, 1);
        }
        if (MUSIC_ON == 1) {
            platform->playModule(Platform::ModuleWin);
        }
    } else {
        // LOSE MESSAGE
        for (int X = 0; X != 9; X++) {
            writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS + 16 + X, LOS_MSG[X], 4, 1);
        }
        if (MUSIC_ON == 1) {
            platform->playModule(Platform::ModuleLose);
        }
    }
}

char WIN_MSG[] = "you win!";
char LOS_MSG[] = "you lose!";

void PRINT_INTRO_MESSAGE()
{
    PRINT_INFO(INTRO_MESSAGE);
}

// This routine will print something to the "information" window
// at the bottom left of the screen.  You must first define the 
// source of the text in $FB. The text should terminate with
// a null character.
void PRINT_INFO(const char *text)
{
    SCROLL_INFO(); // New text always causes a scroll
    PRINTX = 0;
    for (int Y = 0; text[Y] != 0; Y++) {
        if (text[Y] == -1) { // return
            PRINTX = 0;
            SCROLL_INFO();
        } else {
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + PRINTX, text[Y]);
            PRINTX++;
        }
    }
}

uint8_t PRINTX = 0; // used to store X-cursor location

// This routine scrolls the info screen by one row, clearing
// a new row at the bottom.
void SCROLL_INFO()
{
    /*
    int X;
    for (X = 0; X != 33; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x398 + X]);
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 2) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x3C0 + X]); // BOTTOM ROW
    }
    // NOW CLEAR BOTTOM ROW
    for (X = 0; X != 33; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + X, 32); // BOTTOM ROW
    }
    */
    platform->copyRect(0, PLATFORM_SCREEN_HEIGHT - 16, 0, PLATFORM_SCREEN_HEIGHT - 24, PLATFORM_SCREEN_WIDTH - 56, 16);
    // NOW CLEAR BOTTOM ROW
    platform->clearRect(0, PLATFORM_SCREEN_HEIGHT - 8, PLATFORM_SCREEN_WIDTH - 56, 8);
    platform->renderFrame(true);
}

void RESET_KEYS_AMMO()
{
    KEYS = 0;
    AMMO_PISTOL = 0;
    AMMO_PLASMA = 0;
    INV_BOMBS = 0;
    INV_EMP = 0;
    INV_MEDKIT = 0;
    INV_MAGNET = 0;
    SELECTED_WEAPON = 0;
    SELECTED_ITEM = 0;
    MAGNET_ACT = 0;
    PLASMA_ACT = 0;
    BIG_EXP_ACT = 0;
    CYCLES = 0;
    SECONDS = 0;
    MINUTES = 0;
    HOURS = 0;
}

void INTRO_SCREEN()
{
    platform->fadeScreen(0, false);
    DISPLAY_INTRO_SCREEN();
    DISPLAY_MAP_NAME();
    CHANGE_DIFFICULTY_LEVEL();
    START_INTRO_MUSIC();
    platform->show();
    platform->fadeScreen(15, false);
    MENUY = 0;
    REVERSE_MENU_OPTION(true);
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(false);
        if (A != 0xff || B != 0) {
            if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                if (MENUY != (3 - 1)) {
                    REVERSE_MENU_OPTION(false);
                    MENUY++;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
                if (MENUY != 0) {
                    REVERSE_MENU_OPTION(false);
                    MENUY--;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_SPACE] || // SPACE
                       A == KEY_CONFIG[KEY_RETURN] || (B & Platform::JoystickRed)) { // RETURN
                done = EXEC_COMMAND();
            }
             else if (A == KEY_CONFIG[KEY_MUSIC]) { // SHIFT-M
                if (MUSIC_ON == 1) {
                    MUSIC_ON = 0;
                } else {
                    MUSIC_ON = 1;
                }
                START_INTRO_MUSIC();
            }
        }
        platform->renderFrame(true);
    }
}

void START_INTRO_MUSIC()
{
    platform->playModule(MUSIC_ON == 1 ? Platform::ModuleIntro : Platform::ModuleSoundFX);
}

bool EXEC_COMMAND()
{
    if (MENUY == 0) { // START GAME
        SET_CONTROLS();
        platform->stopModule();
        platform->stopNote(); // turn off sound
        INIT_GAME();
        return true;
    } else if (MENUY == 2) { // DIFF LEVEL
        DIFF_LEVEL++;
        if (DIFF_LEVEL == 3) {
            DIFF_LEVEL = 0;
        }
        CHANGE_DIFFICULTY_LEVEL();
        PLAY_SOUND(15); // menu beep
    } else if (MENUY == 1) { // cycle map
        PLAY_SOUND(15); // menu beep
        CYCLE_MAP();
    } else if (MENUY == 3) {
        CYCLE_CONTROLS();
        PLAY_SOUND(15); // menu beep
    }
    return false;
}

void CYCLE_CONTROLS()
{
    KEYS_DEFINED = 0;
    CONTROL++;
    if (CONTROL == 3) {
        CONTROL = 0;
    }
    // display control method on screen
    for (int X = 0, Y = CONTROLSTART[CONTROL]; X != 10; X++, Y++) {
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS + 4 + X, CONTROLTEXT[Y], 14, 5);
    }
}

char CONTROLTEXT[] = "standard  "
                     "custom    "
                     "snes pad  ";
uint8_t CONTROLSTART[] = { 0, 10, 20 };

void CYCLE_MAP()
{
    SELECTED_MAP++;
    if (SELECTED_MAP == MAP_COUNT) { // Maximum number of maps
        SELECTED_MAP = 0;
    }
    DISPLAY_MAP_NAME();
}

void DISPLAY_MAP_NAME()
{
    char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
        writeToScreenMemory(7 * SCREEN_WIDTH_IN_CHARACTERS + 1 + Y, name[Y], 15, 5);
    }
    // now set the mapname for the filesystem load
    MAPNAME[6] = SELECTED_MAP + 65;
}

char* CALC_MAP_NAME()
{
    // FIND MAP NAME
    return MAP_NAMES + (SELECTED_MAP << 4); // multiply by 16 by shifting 4 times to left.
}

void REVERSE_MENU_OPTION(bool reverse)
{
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART[MENUY] + Y, SCREEN_MEMORY[MENU_CHART[MENUY] + Y], reverse ? 14 : 15, 5);
    }
}

uint8_t MENUY = 0; // CURRENT MENU SELECTION
menu_chart_t MENU_CHART[] = { 2 * SCREEN_WIDTH_IN_CHARACTERS + 4, 3 * SCREEN_WIDTH_IN_CHARACTERS + 4, 4 * SCREEN_WIDTH_IN_CHARACTERS + 4 };

void CHANGE_DIFFICULTY_LEVEL()
{
    platform->renderFace(DIFF_LEVEL, 234, 75);
}

uint8_t DIFF_LEVEL = 1; // default medium
uint8_t ROBOT_FACE[] = {
    0x3A, 0x43, 0x49, 0x55, 0x43, 0x3A, 0x49, 0x55, // EASY LEVEL
    0x40, 0x40, 0x6E, 0x70, 0x40, 0x40, 0x49, 0x55, // MEDIUM LEVEL
    0x3A, 0x4D, 0x3A, 0x3A, 0x4E, 0x3A, 0x4D, 0x4E  // HARD LEVEL
};
uint8_t FACE_LEVEL[] = { 0, 8, 16 };

// This routine is run after the map is loaded, but before the
// game starts.  If the diffulcty is set to normal, nothing 
// actually happens.  But if it is set to easy or hard, then
// some changes occur accordingly.
void SET_DIFF_LEVEL()
{
    if (DIFF_LEVEL == 0) { // easy
        SET_DIFF_EASY();
    } else if (DIFF_LEVEL == 2) { // hard
        SET_DIFF_HARD();
    }
}

void SET_DIFF_EASY()
{
    // Find all hidden items and double the quantity.
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            UNIT_TYPE[X] != 128) { // KEY
            UNIT_A[X] <<= 1; // item qty
        }
    }
}

void SET_DIFF_HARD()
{
    // Find all hoverbots and change AI
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] == 2 || // hoverbot left/right
            UNIT_TYPE[X] == 3) { // hoverbot up/down
            UNIT_TYPE[X] = 4; // hoverbot attack mode
        }
    }
}

// This chart contains the left-most staring position for each
// row of tiles on the map-editor. 7 Rows.
uint16_t MAP_CHART[PLATFORM_MAP_WINDOW_TILES_HEIGHT]
#if (MAP_WINDOW_SIZE == 77)
 = {
    0x000, 0x078, 0x0F0, 0x168, 0x1E0, 0x258, 0x2D0
}
#endif
;


void EMP_FLASH()
{
    BORDER_COLOR = 0x00f;
    BORDER = 10;
}

// This routine animates the tile #204 (water) 
// and also tile 148 (trash compactor)
void ANIMATE_WATER()
{
    if (ANIMATE != 1) {
        return;
    }
    WATER_TIMER++;
    if (WATER_TIMER != 20) {
        return;
    }
    WATER_TIMER = 0;
    ANIM_STATE++;
    CINEMA_STATE++;
    if (CINEMA_STATE == 197) {
        CINEMA_STATE = 0;
    }
    REDRAW_WINDOW = 1;
}

uint8_t WATER_TIMER = 0;
uint8_t ANIM_STATE = 0;
uint8_t CINEMA_STATE = 0;

// This is the routine that allows a person to select
// a level and highlights the selection in the information
// display. It is unique to each computer since it writes
// to the screen directly.
void ELEVATOR_SELECT()
{
    if (LIVE_MAP_ON == 0) {
        DRAW_MAP_WINDOW();
    }
    ELEVATOR_MAX_FLOOR = UNIT_D[UNIT]; // get max levels
    // Now draw available levels on screen
    for (int Y = 0, A = 0x31; Y != ELEVATOR_MAX_FLOOR; A++, Y++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 6 + Y, A);
    }
    ELEVATOR_CURRENT_FLOOR = UNIT_C[UNIT]; // what level are we on now?
    // Now highlight current level
    ELEVATOR_INVERT();
    platform->renderFrame();
    // Now get user input
    while (!platform->quit) {
        // KEYBOARD INPUT
        uint8_t A = platform->readKeyboard();
        // SNES INPUT
        uint16_t B = platform->readJoystick(CONTROL == 2 ? true : false);
        if (A != 0xff || B != 0) {
            if (A == KEY_CONFIG[KEY_LIVE_MAP] || (CONTROL == 2 && (B & Platform::JoystickPlay) && (B & Platform::JoystickLeft))) {
                TOGGLE_LIVE_MAP();
                if (LIVE_MAP_ON == 0) {
                    DRAW_MAP_WINDOW();
                }
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_LIVE_MAP_ROBOTS] || (CONTROL == 2 && (B & Platform::JoystickPlay) && (B & Platform::JoystickDown))) {
                TOGGLE_LIVE_MAP_ROBOTS();
                CLEAR_KEY_BUFFER();
            } else
            if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
                ELEVATOR_DEC();
            } else if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
                ELEVATOR_INC();
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                SCROLL_INFO();
                SCROLL_INFO();
                SCROLL_INFO();
                CLEAR_KEY_BUFFER();
                return;
            }
        }
        if (LIVE_MAP_ON == 1) {
            DRAW_LIVE_MAP();
        }
        platform->renderFrame(true);
    }
}

uint8_t ELEVATOR_MAX_FLOOR = 0;
uint8_t ELEVATOR_CURRENT_FLOOR = 0;

void ELEVATOR_INVERT()
{
    writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 5 + ELEVATOR_CURRENT_FLOOR, SCREEN_MEMORY[(SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 5 + ELEVATOR_CURRENT_FLOOR] ^ 0x80); // %10000000
}

void ELEVATOR_INC()
{
    if (ELEVATOR_CURRENT_FLOOR != ELEVATOR_MAX_FLOOR) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR++;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_DEC()
{
    if (ELEVATOR_CURRENT_FLOOR != 1) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR--;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_FIND_XY()
{
    for (int X = 32; X != 48; X++) { // start of doors
        if (UNIT_TYPE[X] == 19) { // elevator
            if (UNIT_C[X] == ELEVATOR_CURRENT_FLOOR) {
#if (MAP_WINDOW_SIZE == 77)
                UNIT_LOC_X[0] = UNIT_LOC_X[X]; // player location = new elevator location
                MAP_WINDOW_X = UNIT_LOC_X[X] - 5;
                UNIT_LOC_Y[0] = UNIT_LOC_Y[X] - 1; // player location = new elevator location
                MAP_WINDOW_Y = UNIT_LOC_Y[X] - 4;
#else
                UNIT_LOC_X[0] = UNIT_LOC_X[X]; // player location = new elevator location
                MAP_WINDOW_X = MIN(MAX(UNIT_LOC_X[X] - PLATFORM_MAP_WINDOW_TILES_WIDTH / 2, 0), 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH);
                UNIT_LOC_Y[0] = UNIT_LOC_Y[X] - 1; // player location = new elevator location
                MAP_WINDOW_Y = MIN(MAX(UNIT_LOC_Y[X] - PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2 - 1, 0), 64 - PLATFORM_MAP_WINDOW_TILES_HEIGHT);
#endif
                if (LIVE_MAP_ON == 0) {
                    DRAW_MAP_WINDOW();
                }
                PLAY_SOUND(17); // elevator sound SOUND PLAY
                break;
            }
        }
    }
}

void SET_CONTROLS()
{
    if (CONTROL == 1) { // CUSTOM KEYS
        SET_CUSTOM_KEYS();
    } else {
        // load standard values for key controls
        uint8_t* STANDARD_CONTROLS = platform->standardControls();
        for (int Y = 0; Y != sizeof(KEY_CONFIG); Y++) {
            KEY_CONFIG[Y] = STANDARD_CONTROLS[Y];
        }
    }
}

void SET_CUSTOM_KEYS()
{
    if (KEYS_DEFINED != 0) {
        return;
    }
    platform->fadeScreen(0, false);
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS, 15);
    platform->renderFrame();
    platform->fadeScreen(15, false);
    uint16_t destination = 8 * SCREEN_WIDTH_IN_CHARACTERS + 17;
    for (TEMP_A = 0; TEMP_A != 13;) {
        uint8_t A = platform->readKeyboard();
        if (A != 0xff) {
            KEY_CONFIG[TEMP_A] = A;
            DECNUM = A;
            DECWRITE(destination, 15);
            destination += SCREEN_WIDTH_IN_CHARACTERS;
            TEMP_A++;
            platform->renderFrame();
        }
    }
    KEYS_DEFINED = 1;
}

uint8_t KEYS_DEFINED = 0; // DEFAULT 0

void PET_SCREEN_SHAKE()
{
    if (BGTIMER1 != 1) {
        return;
    }
    if (SELECT_TIMEOUT != 0) { // shoehorned this into the screenshake routine
        SELECT_TIMEOUT--; // this is to prevent accidental double-taps
    } // on cycle weapons or items.
    PET_BORDER_FLASH();
    if (SCREEN_SHAKE == 0) {
        return;
    }
    if (LIVE_MAP_ON != 1) {
        if (SCREEN_SHAKE == 1) {
            platform->copyRect(8, 0, 0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32);
            platform->renderFrame(true);
            INVALIDATE_PREVIOUS_MAP();
        } else if (SCREEN_SHAKE == 3) {
            REDRAW_WINDOW = 1;
        }
        SCREEN_SHAKE++;
        if (SCREEN_SHAKE == 5) {
            SCREEN_SHAKE = 1;
        }
    }
}

// So, it doesn't really flash the PET border, instead it
// flashes the health screen.
void PET_BORDER_FLASH()
{
    if (BORDER != 0) {
        // border flash should be active
        if (FLASH_STATE != 1) { // Is it already flashing?
            platform->startFadeScreen(BORDER_COLOR, 15 - BORDER);
            FLASH_STATE = 1;
        }
    } else {
        if (FLASH_STATE != 0) {
            FLASH_STATE = 0;
            platform->stopFadeScreen();
        }
    }
}

uint8_t FLASH_STATE = 0;

// This is actually part of a background routine, but it has to be in the main
// source because the screen effects used are unique on each system.
void DEMATERIALIZE()
{
    UNIT_TILE[0] = 243; // dematerialize tile
    UNIT_TIMER_B[UNIT]++;
    DEMATERIALIZE_FRAME = UNIT_TIMER_B[UNIT] >> 2;
    if (UNIT_TIMER_B[UNIT] != 0x20) {
        UNIT_TIMER_A[UNIT] = 1;
        REDRAW_WINDOW = 1;
    } else {
        // TRANSPORT COMPLETE
        if (UNIT_B[UNIT] != 1) { // transport somewhere
            UNIT_TYPE[0] = 2; // this mean game over condition, player type
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
        } else {
            UNIT_TILE[0] = 97;
            UNIT_LOC_X[0] = UNIT_C[UNIT]; // target X coordinates
            UNIT_LOC_Y[0] = UNIT_D[UNIT]; // target Y coordinates
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
            CACULATE_AND_REDRAW();
        }
    }
}

void ANIMATE_PLAYER()
{
    UNIT_TILE[0] = 96;
    WALK_FRAME++;
    WALK_FRAME &= 3;
}

void PLAY_SOUND(int sound)
{
    platform->playSample(sound);
}

void STOP_SONG()
{
    platform->stopSample();
}

void BACKGROUND_TASKS()
{
    if (BGTIMER1 == 1) {
        if (LIVE_MAP_ON) {
            DRAW_LIVE_MAP();
        } else
        if (REDRAW_WINDOW == 1) {
            REDRAW_WINDOW = 0;
            DRAW_MAP_WINDOW();
        }
        platform->renderFrame();
    }
    // Now check to see if it is time to run background tasks
    if (BGTIMER1 != 1) {
        return;
    }
    BGTIMER1 = 0; // RESET BACKGROUND TIMER
    for (UNIT = 1; UNIT != 64; UNIT++) {
        // ALL AI routines must JMP back to here at the end.
        if (UNIT_TYPE[UNIT] != 0) { // Does unit exist?
            // Unit found to exist, now check it's timer.
            // unit code won't run until timer hits zero.
            if (UNIT_TIMER_A[UNIT] != 0) {
                UNIT_TIMER_A[UNIT]--; // Decrease timer by one.
            } else {
                // Unit exists and timer has triggered
                // The unit type determines which AI routine is run.
                if (UNIT_TYPE[UNIT] < 24) { // MAX DIFFERENT UNIT TYPES IN CHART, ABORT IF GREATER
                    AI_ROUTINE_CHART[UNIT_TYPE[UNIT]]();
                }
            }
        }
    }
}

void (*AI_ROUTINE_CHART[])(void) =
{
    DUMMY_ROUTINE,      // UNIT TYPE 00   ;non-existent unit
    DUMMY_ROUTINE,      // UNIT TYPE 01   ;player unit - can't use
    LEFT_RIGHT_DROID,   // UNIT TYPE 02
    UP_DOWN_DROID,      // UNIT TYPE 03
    HOVER_ATTACK,       // UNIT TYPE 04
    WATER_DROID,        // UNIT TYPE 05
    TIME_BOMB,      // UNIT TYPE 06
    TRANSPORTER_PAD,    // UNIT TYPE 07
    DEAD_ROBOT,     // UNIT TYPE 08
    EVILBOT,        // UNIT TYPE 09 
    AI_DOOR,        // UNIT TYPE 10
    SMALL_EXPLOSION,    // UNIT TYPE 11
    PISTOL_FIRE_UP,     // UNIT TYPE 12
    PISTOL_FIRE_DOWN,   // UNIT TYPE 13
    PISTOL_FIRE_LEFT,   // UNIT TYPE 14
    PISTOL_FIRE_RIGHT,  // UNIT TYPE 15
    TRASH_COMPACTOR,    // UNIT TYPE 16
    UP_DOWN_ROLLERBOT,  // UNIT TYPE 17
    LEFT_RIGHT_ROLLERBOT,   // UNIT TYPE 18
    ELEVATOR,       // UNIT TYPE 19
    MAGNET,         // UNIT TYPE 20
    MAGNETIZED_ROBOT,   // UNIT TYPE 21
    WATER_RAFT_LR,      // UNIT TYPE 22
    DEMATERIALIZE      // UNIT TYPE 23
};

// Dummy routine does nothing, but I need it for development.
void DUMMY_ROUTINE()
{
    return;
}

void WATER_RAFT_LR()
{
    // First check which direction raft is moving.
    if (UNIT_A[UNIT] == 1) {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]++; // player
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            CHECK_FOR_WINDOW_REDRAW();
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            CHECK_FOR_WINDOW_REDRAW();
        }
        // Now check if it has reached its destination
        if (UNIT_LOC_X[UNIT] != UNIT_C[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 0;
        }
    } else {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]--; // player
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            CHECK_FOR_WINDOW_REDRAW();
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            CHECK_FOR_WINDOW_REDRAW();
        }
        // Now check if it has reached its destination
        if (UNIT_LOC_X[UNIT] != UNIT_B[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 1;
        }
    }
}

void RAFT_DELETE()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 204; // WATER TILE
    PLOT_TILE_TO_MAP();
}

void RAFT_PLOT()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 242; // RAFT TILE
    PLOT_TILE_TO_MAP();
}

void MAGNETIZED_ROBOT()
{
    CHECK_FOR_WINDOW_REDRAW();
    MOVE_TYPE = 0x01; // %00000001
    GENERATE_RANDOM_NUMBER();
    switch (RANDOM & 0x03) { // %00000011
    case 0:
        REQUEST_WALK_UP();
        break;
    case 1:
        REQUEST_WALK_DOWN();
        break;
    case 2:
        REQUEST_WALK_LEFT();
        break;
    case 3:
        REQUEST_WALK_RIGHT();
        break;
    default:
        break;
    }
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 10;
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_TYPE[UNIT] = UNIT_D[UNIT];
    }
}

void GENERATE_RANDOM_NUMBER()
{
    if (RANDOM != 0) { // added this
        if (RANDOM & 0x80) {
            RANDOM = (RANDOM << 1) ^ 0x1D;
        } else {
            RANDOM = (RANDOM << 1);
        }
    } else {
        RANDOM = 0x1D;
    }
}

void MAGNET()
{
    // First let's take care of the timers.  This unit runs
    // every cycle so that it can detect contact with another
    // unit.  But it still needs to count down to terminate
    // So, it uses two timers for a 16-bit value.
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_A[UNIT]--;
        if (UNIT_A[UNIT] == 0) {
            // Both timers have reached zero, time to deactivate.
            UNIT_TYPE[UNIT] = 0;
            MAGNET_ACT = 0;
            return;
        }
    }
    // Now let's see if another units walks on the magnet.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    CHECK_FOR_UNIT();
    if (UNIT_FIND == 255) { // no unit found
        return;
    } else if (UNIT_FIND == 0) { // player unit
        INV_MAGNET++;
        DISPLAY_ITEM();
    } else {
        // Collision with robot detected.
        PLAY_SOUND(4); // HAYWIRE SOUND, SOUND PLAY
        UNIT_D[UNIT_FIND] = UNIT_TYPE[UNIT_FIND]; // make backup of unit type
        UNIT_TYPE[UNIT_FIND] = 21; // Crazy robot AI
        UNIT_TIMER_B[UNIT_FIND] = 60;
    }
    UNIT_TYPE[UNIT] = 0;
    MAGNET_ACT = 0;
}

void DEAD_ROBOT()
{
    UNIT_TYPE[UNIT] = 0;
}

void UP_DOWN_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    CHECK_FOR_WINDOW_REDRAW();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void LEFT_RIGHT_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    CHECK_FOR_WINDOW_REDRAW();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void ROLLERBOT_FIRE_DETECT()
{
    int X;
    TEMP_A = UNIT_LOC_X[UNIT];
    TEMP_B = UNIT_LOC_Y[UNIT];
    // See if we're lined up vertically
    if (UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) { // robot, player
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[UNIT] - UNIT_LOC_X[0] >= 6) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 14; // pistol fire left AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[0] - UNIT_LOC_X[UNIT] >= 6) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 15; // pistol fire right AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    } else if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0]) { // robot, player
        // See if we're lined up horizontally
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0] >= 4) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 12; // pistol fire UP AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[0] - UNIT_LOC_Y[UNIT] >= 4) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 13; // pistol fire DOWN AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    }
}

void ROLLERBOT_AFTER_FIRE(uint8_t unit, uint8_t tile)
{
    UNIT_TILE[unit] = tile;
    UNIT_A[unit] = 5; // travel distance.
    UNIT_B[unit] = 0; // weapon-type = pistol
    UNIT_TIMER_A[unit] = 0;
    UNIT_LOC_X[unit] = TEMP_A;
    UNIT_LOC_Y[unit] = TEMP_B;
    PLAY_SOUND(9); // PISTOL SOUND SOUND PLAY
}

void ROLLERBOT_ANIMATE()
{
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        return;
    }
    UNIT_TIMER_B[UNIT] = 3; // RESET ANIMATE TIMER
    if (UNIT_TILE[UNIT] == 164) {
        UNIT_TILE[UNIT] = 165; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TILE[UNIT] = 164; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B: 0=completes level 1=send to coordinates
// UNIT_C: X-coordinate
// UNIT_D: Y-coordinate

// The "DEMATERIALIZE" part of this AI routine has to be in the main 
// source for each individual computer, because the screen effects
// are created uniquely for each one.

void TRANSPORTER_PAD()
{
    // first determine if the player is standing here
    if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] && UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
        TRANS_PLAYER_PRESENT();
    } else {
        // player not present
        if (UNIT_A[UNIT] != 1) {
            TRANS_ACTIVE();
        } else {
            // test if all robots are dead
            for (int X = 1; X != 28; X++) {
                if (UNIT_TYPE[X] != 0) {
                    UNIT_TIMER_A[UNIT] = 30;
                    return;
                }
            }
            UNIT_A[UNIT] = 0; // make unit active
            UNIT_TIMER_A[UNIT] = 30;
        }
    }
}

void TRANS_PLAYER_PRESENT()
{
    if (UNIT_A[UNIT] != 0) { // unit active
        PRINT_INFO(MSG_TRANS1);
        PLAY_SOUND(11); // error-SOUND SOUND PLAY
        UNIT_TIMER_A[UNIT] = 100;
    } else {
        // start transport process
        UNIT_TYPE[UNIT] = 23; // Convert to different AI
        UNIT_TIMER_A[UNIT] = 5;
        UNIT_TIMER_B[UNIT] = 0;
    }
}

void TRANS_ACTIVE()
{
    if (UNIT_TIMER_B[UNIT] != 1) {
        UNIT_TIMER_B[UNIT] = 1;
        TILE = 30;
    } else {
        UNIT_TIMER_B[UNIT] = 0;
        TILE = 31;
    }
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    PLOT_TILE_TO_MAP();
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 30;
}

void TIME_BOMB()
{
    if (UNIT_A[UNIT] == 0) {
        BIG_EXP_PHASE1();
    } else if (UNIT_A[UNIT] == 1) {
        BIG_EXP_PHASE2();
    }
}

// This is the large explosion used by the time-bomb
// and plasma gun, and maybe others.  This is the first
// phase of the explosion, which stores the tiles to
// a buffer and then changes each tile to an explosion.
void BIG_EXP_PHASE1()
{
    if (BIG_EXP_ACT != 0) { // Check that no other explosion active.
        UNIT_TIMER_A[UNIT] = 10;
        return; // wait for existing explosion to finish.
    }
    BIG_EXP_ACT = 1; // Set flag so no other explosions can begin until this one ends.
    SCREEN_SHAKE = 1;
    PLAY_SOUND(0); // explosion-sound SOUND PLAY
    BEX_PART1(); // check center piece for unit
    BEXCEN(); // check center piece for unit
    if (LIVE_MAP_ON == 1) {
        GET_TILE_FROM_MAP();
        MAP_SOURCE[0] = 246;
        platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        MAP_SOURCE[0] = TILE;
    }
    BEX1_NORTH();
    BEX1_SOUTH();
    BEX1_EAST();
    BEX1_WEST();
    BEX1_NE();
    BEX1_NW();
    BEX1_SE();
    BEX1_SW();
    UNIT_TILE[UNIT] = 246; // explosion tile
    UNIT_A[UNIT] = 1; // move to next phase of explosion.
    UNIT_TIMER_A[UNIT] = 12;
    REDRAW_WINDOW = 1;
}

// There are 8 separate subroutines for the large explosion
// with each one handling a specific outward direction of motion.
// The "unit" itself changes tiles to an explosion, so we don't
// need to mess with the center tile.
void BEX1_NORTH()
{
    BEX_PART1();
    // first tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[0] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[1] = TILE;
    BEX_PART3();
}

void BEX1_SOUTH()
{
    BEX_PART1();
    // first tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[2] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[3] = TILE;
    BEX_PART3();
}

void BEX1_EAST()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[4] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[5] = TILE;
    BEX_PART3();
}

void BEX1_WEST()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[6] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[7] = TILE;
    BEX_PART3();
}

void BEX1_NE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[8] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[9] = TILE;
    BEX_PART3();
}

void BEX1_NW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[10] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[11] = TILE;
    BEX_PART3();
}

void BEX1_SE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[12] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[13] = TILE;
    BEX_PART3();
}

void BEX1_SW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[14] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[15] = TILE;
    BEX_PART3();
}

void BEX_PART1()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
}

bool BEX_PART2()
{
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x10) == 0x10; // can see through tile?
}

void BEX_PART3()
{
    MAP_SOURCE[0] = 246;
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
    }
    BEXCEN();
}

void BEXCEN()
{
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        TEMP_A = 11;
        INFLICT_DAMAGE();
    }
}

void BIG_EXP_PHASE2()
{
    // Do the center tile first.
    BEX_PART1();
    GET_TILE_FROM_MAP();
    MAP_SOURCE[0] = 246;
    TEMP_A = TILE;
    RESTORE_TILE();
    // tile #0 north 1
    BEX_PART1();
    MAP_Y--;
    TEMP_A = EXP_BUFFER[0];
    RESTORE_TILE();
    // tile #1 north 2
    MAP_Y--;
    TEMP_A = EXP_BUFFER[1];
    RESTORE_TILE();
    // tile #2 south 1
    BEX_PART1();
    MAP_Y++;
    TEMP_A = EXP_BUFFER[2];
    RESTORE_TILE();
    // tile #3 south 2
    MAP_Y++;
    TEMP_A = EXP_BUFFER[3];
    RESTORE_TILE();
    // tile #4 east 1
    BEX_PART1();
    MAP_X++;
    TEMP_A = EXP_BUFFER[4];
    RESTORE_TILE();
    // tile #5 east 2
    MAP_X++;
    TEMP_A = EXP_BUFFER[5];
    RESTORE_TILE();
    // tile #6 west 1
    BEX_PART1();
    MAP_X--;
    TEMP_A = EXP_BUFFER[6];
    RESTORE_TILE();
    // tile #7 west 2
    MAP_X--;
    TEMP_A = EXP_BUFFER[7];
    RESTORE_TILE();
    // tile #8 northeast 1
    BEX_PART1();
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[8];
    RESTORE_TILE();
    // tile #9 northeast 2
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[9];
    RESTORE_TILE();
    // tile #10 northwest 1
    BEX_PART1();
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[10];
    RESTORE_TILE();
    // tile #11 northwest 2
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[11];
    RESTORE_TILE();
    // tile #12 southeast 1
    BEX_PART1();
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[12];
    RESTORE_TILE();
    // tile #13 southeast 2
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[13];
    RESTORE_TILE();
    // tile #14 southwest 1
    BEX_PART1();
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[14];
    RESTORE_TILE();
    // tile #15 southwest 2
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[15];
    RESTORE_TILE();
    REDRAW_WINDOW = 1;
    UNIT_TYPE[UNIT] = 0; // Deactivate this AI
    BIG_EXP_ACT = 0;
    SCREEN_SHAKE = 0;
}

void RESTORE_TILE()
{
    GET_TILE_FROM_MAP();
    if (TILE != 246) {
        return;
    }
    if (TEMP_A != 131) { // Cannister tile
        if ((TILE_ATTRIB[TEMP_A] & 0x08) == 0x08) { // %00001000 can it be destroyed?
            MAP_SOURCE[0] = DESTRUCT_PATH[TEMP_A];
        } else {
            MAP_SOURCE[0] = TEMP_A;
        }
        if (LIVE_MAP_ON == 1) {
            platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        }
    } else {
        // What to do if we encounter an explosive cannister
        MAP_SOURCE[0] = 135; // Blown cannister
        if (LIVE_MAP_ON == 1) {
            platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        }
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 6; // bomb AI
                UNIT_TILE[X] = 131; // Cannister tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 10; // How long until exposion?
                UNIT_A[X] = 0;
                return;
            }
        }
        // no slots available right now, abort.
    }
}

void TRASH_COMPACTOR()
{
    if (UNIT_A[UNIT] == 0) { // OPEN
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 148) { // Usual tile for trash compactor danger zone
            MAP_SOURCE[1] = TILE;
            UNIT_TIMER_A[UNIT] = 20;
            // now check for units in the compactor
            MAP_X = UNIT_LOC_X[UNIT];
            MAP_Y = UNIT_LOC_Y[UNIT];
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return; // Nothing found, do nothing.
            }
        }
        // Object has been detected in TC, start closing.
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else if (UNIT_A[UNIT] == 1) { // MID-CLOSING STATE
        TCPIECE1 = 152;
        TCPIECE2 = 153;
        TCPIECE3 = 156;
        TCPIECE4 = 157;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 50;
        // Now check for any live units in the compactor
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) {
            MAP_X++; // check second tile
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return;
            }
        }
        // Found unit in compactor, kill it.
        PRINT_INFO(MSG_TERMINATED);
        PLAY_SOUND(0); // EXPLOSION sound SOUND PLAY
        UNIT_TYPE[UNIT_FIND] = 0;
        UNIT_HEALTH[UNIT_FIND] = 0;
        for (int X = 28; X != 32; X++) { // start of weapons
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 11; // SMALL EXPLOSION
                UNIT_TILE[X] = 248; // first tile for explosion
                UNIT_LOC_X[X] = UNIT_LOC_X[UNIT];
                UNIT_LOC_Y[X] = UNIT_LOC_Y[UNIT];
                if (UNIT_FIND == 0) { // is it the player
                    BORDER_COLOR = 0xf00;
                    BORDER = 10;
                }
                break;
            }
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else if (UNIT_A[UNIT] == 2) { // CLOSED STATE
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
    } else if (UNIT_A[UNIT] == 3) { // MID-OPENING STATE
        TCPIECE1 = 144;
        TCPIECE2 = 145;
        TCPIECE3 = 148;
        TCPIECE4 = 148;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT] = 0;
        UNIT_TIMER_A[UNIT] = 20;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else {
        // should never get here.
    }
}

void DRAW_TRASH_COMPACTOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--; // start one tile above
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = TCPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = TCPIECE2;
    MAP_SOURCE[128] = TCPIECE3;
    MAP_SOURCE[129] = TCPIECE4;
    CHECK_FOR_WINDOW_REDRAW();
}
uint8_t TCPIECE1 = 0;
uint8_t TCPIECE2 = 0;
uint8_t TCPIECE3 = 0;
uint8_t TCPIECE4 = 0;

void WATER_DROID()
{
    // first rotate the tiles
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
    } else {
        UNIT_TIMER_B[UNIT] = 3;
        UNIT_TILE[UNIT]++;
        if (UNIT_TILE[UNIT] == 143) {
            UNIT_TILE[UNIT] = 140;
        }
    }
    UNIT_A[UNIT]--;
    if (UNIT_A[UNIT] != 0) {
        CHECK_FOR_WINDOW_REDRAW();
        return;
    }
    // kill unit after countdown reaches zero. 
    UNIT_A[UNIT] = UNIT_TYPE[UNIT];
    UNIT_TYPE[UNIT] = 8; // Dead robot type
    UNIT_TIMER_A[UNIT] = 255;
    UNIT_TILE[UNIT] = 115; // dead robot tile
    CHECK_FOR_WINDOW_REDRAW();
}

void PISTOL_FIRE_UP()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_Y[UNIT]--; // move it up one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_DOWN()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_Y[UNIT]++; // move it down one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_LEFT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_X[UNIT]--; // move it left one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_RIGHT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_X[UNIT]++; // move it right one.
        PISTOL_AI_COMMON();
    }
}

void DEACTIVATE_WEAPON()
{
    UNIT_TYPE[UNIT] = 0;
    if (UNIT_B[UNIT] == 1) {
        UNIT_B[UNIT] = 0;
        PLASMA_ACT = 0;
    }
}

void PISTOL_AI_COMMON()
{
    if (UNIT_B[UNIT] == 0) { // is it pistol or plasma?
        UNIT_A[UNIT]--; // reduce range by one
        // Now check what map object it is on.
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 131) { // explosive cannister
            // hit an explosive cannister
            MAP_SOURCE[0] = 135; // Blown cannister
            UNIT_TYPE[UNIT] = 6; // bomb AI
            UNIT_TILE[UNIT] = 131; // Cannister tile
            UNIT_LOC_X[UNIT] = MAP_X;
            UNIT_LOC_Y[UNIT] = MAP_Y;
            UNIT_TIMER_A[UNIT] = 5; // How long until exposion?
            UNIT_A[UNIT] = 0;
        } else if ((TILE_ATTRIB[TILE] & 0x10) != 0x10) { // can see through tile?
            // Hit object that can't pass through, convert to explosion
            UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
            UNIT_TILE[UNIT] = 248; // first tile for explosion
            CHECK_FOR_WINDOW_REDRAW();
        } else {
            // check if it encountered a robot/human
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                CHECK_FOR_WINDOW_REDRAW();
            } else {
                // struck a robot/human
                UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
                UNIT_TILE[UNIT] = 248; // first tile for explosion
                TEMP_A = 1; // set damage for pistol
                INFLICT_DAMAGE();
                ALTER_AI();
                CHECK_FOR_WINDOW_REDRAW();
            }
        }
    } else {
        UNIT_A[UNIT]--; // reduce range by one
        // find what tile we are over
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE != 131) { // cannister tile
            if ((TILE_ATTRIB[TILE] & 0x10) == 0x10) {
                // check if it encountered a human/robot
                CHECK_FOR_UNIT();
                if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                    // no impacts detected:
                    CHECK_FOR_WINDOW_REDRAW();
                    return;
                }
            }
        }
        // impact detected. convert to explosion
        UNIT_TYPE[UNIT] = 6; // bomb AI
        UNIT_TIMER_A[UNIT] = 1; // How long until exposion?
        UNIT_A[UNIT] = 0;
        PLASMA_ACT = 0;
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// This routine checks to see if the robot being shot
// is a hoverbot, if so it will alter it's AI to attack 
// mode.
void ALTER_AI()
{
    if (UNIT_TYPE[UNIT_FIND] == 2 || UNIT_TYPE[UNIT_FIND] == 3) { // hoverbot left/right UP/DOWN
        UNIT_TYPE[UNIT_FIND] = 4; // Attack AI
    }
}

// This routine will inflict damage on whatever is defined in
// UNIT_FIND in the amount set in TEMP_A.  If the damage is more
// than the health of that unit, it will delete the unit.
void INFLICT_DAMAGE()
{
    UNIT_HEALTH[UNIT_FIND] -= TEMP_A;
    if (UNIT_HEALTH[UNIT_FIND] > 0) {
        if (UNIT_FIND == 0) { // IS IT THE PLAYER?
            DISPLAY_PLAYER_HEALTH();
            BORDER_COLOR = 0xf00;
            BORDER = 10;
        }
        return;
    }
    UNIT_HEALTH[UNIT_FIND] = 0;
    if (UNIT_FIND != 0) { // Is it the player that is dead?
        if (UNIT_TYPE[UNIT_FIND] != 8) { // Dead robot type - is it a dead robot already?
            UNIT_A[UNIT_FIND] = UNIT_TYPE[UNIT_FIND];
            UNIT_TYPE[UNIT_FIND] = 8;
            UNIT_TIMER_A[UNIT_FIND] = 255;
            UNIT_TILE[UNIT_FIND] = 115; // dead robot tile
        }
    } else {
        UNIT_TYPE[UNIT_FIND] = 0;
        DISPLAY_PLAYER_HEALTH();
        BORDER_COLOR = 0xf00;
        BORDER = 10;
    }
}

void SMALL_EXPLOSION()
{
    UNIT_TIMER_A[UNIT] = 0;
    UNIT_TILE[UNIT]++;
    if (UNIT_TILE[UNIT] != 252) {
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TYPE[UNIT] = 0;
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVER_ATTACK()
{
    UNIT_TIMER_B[UNIT] = 0;
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 7;
    CHECK_FOR_WINDOW_REDRAW();
    MOVE_TYPE = 0x02; // %00000010 HOVER
    // CHECK FOR HORIZONTAL MOVEMENT
    if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
        REQUEST_WALK_LEFT();
    } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
        REQUEST_WALK_RIGHT();
    }
    // NOW CHECK FOR VERITCAL MOVEMENT
    if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
        REQUEST_WALK_UP();
    } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
        REQUEST_WALK_DOWN();
    }
    ROBOT_ATTACK_RANGE();
    if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
        TEMP_A = 1; // amount of damage it will inflict
        UNIT_FIND = 0; // unit to inflict damage on.
        INFLICT_DAMAGE();
        CREATE_PLAYER_EXPLOSION();
        PLAY_SOUND(7); // electric shock SOUND PLAY
        UNIT_TIMER_A[UNIT] = 30; // rate of attack on player.
        // add some code here to create explosion
    }
    CHECK_FOR_WINDOW_REDRAW();
}

void CREATE_PLAYER_EXPLOSION()
{
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 11; // Small explosion AI type
            UNIT_TILE[X] = 248; // first tile for explosion
            UNIT_TIMER_A[X] = 1;
            UNIT_LOC_X[X] = UNIT_LOC_X[0];
            UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
            break;
        }
    }
}

void EVILBOT()
{
    UNIT_TIMER_A[UNIT] = 5;
    // first animate evilbot
    if (UNIT_TILE[UNIT] == 100) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 101) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 102) {
        UNIT_TILE[UNIT]++;
    } else {
        UNIT_TILE[UNIT] = 100;
    }
    // now figure out movement
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TIMER_B[UNIT] = 1; // Reset timer B
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x01; // %00000001 WALK
        // CHECK FOR HORIZONTAL MOVEMENT
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            REQUEST_WALK_LEFT();
        } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
            REQUEST_WALK_RIGHT();
        }
        // NOW CHECK FOR VERITCAL MOVEMENT
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            REQUEST_WALK_UP();
        } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
            REQUEST_WALK_DOWN();
        }
        ROBOT_ATTACK_RANGE();
        if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
            TEMP_A = 5; // amount of damage it will inflict
            UNIT_FIND = 0; // unit to inflict damage on.
            INFLICT_DAMAGE();
            CREATE_PLAYER_EXPLOSION();
            PLAY_SOUND(7); // electric shock sound SOUND PLAY
            UNIT_TIMER_A[UNIT] = 15; // rate of attack on player.
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void AI_DOOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        AIDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*AIDB[])(void) = {
    DOOR_OPEN_A,
    DOOR_OPEN_B,
    DOOR_OPEN_FULL,
    DOOR_CLOSE_A,
    DOOR_CLOSE_B,
    DOOR_CLOSE_FULL
};

void DOOR_OPEN_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 17;
        DOORPIECE2 = 9;
        DOORPIECE3 = 91;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 27;
        DOORPIECE2 = 9;
        DOORPIECE3 = 15;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 30;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 1) {
        UNIT_TIMER_B[UNIT] = 30; // RESET TIMER
        return;
    }
    // if nobody near door, lets close it.
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 84;
        DOORPIECE2 = 85;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 69;
        DOORPIECE2 = 73;
        DOORPIECE3 = 77;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 80;
        DOORPIECE2 = 81;
        DOORPIECE3 = 82;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 68;
        DOORPIECE2 = 72;
        DOORPIECE3 = 76;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT != 0) {
        // if player near door, lets open it.
        // first check if locked
        if (UNIT_C[UNIT] == 0 || // Lock status
            (UNIT_C[UNIT] == 1 && (KEYS & 0x01) == 0x01) || // SPADE KEY
            (UNIT_C[UNIT] == 2 && (KEYS & 0x02) == 0x02) || // HEART KEY
            (UNIT_C[UNIT] == 3 && (KEYS & 0x04) == 0x04)) { // STAR KEY
            // Start open door process
            PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
            if (UNIT_A[UNIT] != 1) {
                // HORIZONTAL DOOR
                DOORPIECE1 = 84;
                DOORPIECE2 = 85;
                DOORPIECE3 = 86;
                DRAW_HORIZONTAL_DOOR();
            } else {
                // VERTICAL DOOR
                DOORPIECE1 = 69;
                DOORPIECE2 = 73;
                DOORPIECE3 = 77;
                DRAW_VERTICAL_DOOR();
            }
            UNIT_B[UNIT] = 0;
            UNIT_TIMER_A[UNIT] = 5;
            CHECK_FOR_WINDOW_REDRAW();
            return;
        }
    }
    UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
}

void DRAW_VERTICAL_DOOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--;
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE2;
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE3;
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, UNIT_LOC_X[UNIT], UNIT_LOC_Y[UNIT]);
    }
}

void DRAW_HORIZONTAL_DOOR()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_X--;
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = DOORPIECE2;
    MAP_SOURCE[2] = DOORPIECE3;
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, UNIT_LOC_X[UNIT], UNIT_LOC_Y[UNIT]);
    }
}
uint8_t DOORPIECE1 = 0;
uint8_t DOORPIECE2 = 0;
uint8_t DOORPIECE3 = 0;

void ROBOT_ATTACK_RANGE()
{
    /*
    // This is the original code that does not quite work as intended
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // ROBOT UNIT, PLAYER UNIT
    if (A >= 1) { // 1 HORIZONTAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 1) { // 1 VERTICAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
    */

    int X = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]);
    int Y = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]);
    PROX_DETECT = (X == 1 && Y == 0) || (X == 0 && Y == 1) ? 1 : 0;
}

void DOOR_CHECK_PROXIMITY()
{
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 HORIZONTAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 VERTICAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
}
uint8_t PROX_DETECT = 0; // 0=NO 1=YES

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void ELEVATOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        ELDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*ELDB[])(void) = {
    ELEV_OPEN_A,
    ELEV_OPEN_B,
    ELEV_OPEN_FULL,
    ELEV_CLOSE_A,
    ELEV_CLOSE_B,
    ELEV_CLOSE_FULL
};

void ELEV_OPEN_A()
{
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_B()
{
    DOORPIECE1 = 182;
    DOORPIECE2 = 9;
    DOORPIECE3 = 172;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 50;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_FULL()
{
    // CLOSE DOOR
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // check for player or robot in the way
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // START TO CLOSE ELEVATOR DOOR
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_A()
{
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_B()
{
    DOORPIECE1 = 80;
    DOORPIECE2 = 81;
    DOORPIECE3 = 174;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
    ELEVATOR_PANEL();
}

void ELEV_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 0) {
        UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
        return;
    }
    // Start open door process
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 0;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEVATOR_PANEL()
{
    // Check to see if player is standing in the
    // elevator first.
    if ((UNIT_LOC_X[UNIT] != UNIT_LOC_X[0]) || // elevator X location, player X location
        ((UNIT_LOC_Y[UNIT] - 1) != UNIT_LOC_Y[0])) { // elevator Y location, player Y location
        return;
    }
    // PLAYER DETECTED, START ELEVATOR PANEL
    PRINT_INFO(MSG_ELEVATOR);
    PRINT_INFO(MSG_LEVELS);
    ELEVATOR_SELECT();
}

void PLOT_TILE_TO_MAP()
{
    MAP_SOURCE = MAP + (MAP_Y << 7) + MAP_X;
    MAP_SOURCE[0] = TILE;
}

// This routine will return the tile for a specific X/Y
// on the map.  You must first define MAP_X and MAP-Y.
// The result is stored in TILE.
void GET_TILE_FROM_MAP()
{
    MAP_SOURCE = MAP + ((MAP_Y << 7) + MAP_X);
    TILE = MAP_SOURCE[0];
}

// In this AI routine, the droid simply goes left until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void LEFT_RIGHT_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// In this AI routine, the droid simply goes UP until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void UP_DOWN_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVERBOT_ANIMATE(uint8_t X)
{
    if (UNIT_TIMER_B[X] != 0) {
        UNIT_TIMER_B[X]--;
    } else {
        UNIT_TIMER_B[X] = 3; // RESET ANIMATE TIMER
        if (UNIT_TILE[X] == 98) {
            UNIT_TILE[X] = 99; // HOVERBOT TILE
        } else {
            UNIT_TILE[X] = 98; // HOVERBOT TILE
        }
    }
}

// The following 4 routines are used by both the player and some
// of the enemy units.  It checks to see if you can walk in a
// specific direction.  It checks for edge of map and also that
// the tile you want to walk onto can allow that.  There is a
// separate routine for hovering robots.
void REQUEST_WALK_RIGHT()
{
    UNIT_DIRECTION[UNIT] = 3;
    if (UNIT_LOC_X[UNIT] != 122) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X++;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_LEFT()
{
    UNIT_DIRECTION[UNIT] = 2;
    if (UNIT_LOC_X[UNIT] != 5) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X--;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_DOWN()
{
    UNIT_DIRECTION[UNIT] = 1;
    if (UNIT_LOC_Y[UNIT] != 60) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y++;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_UP()
{
    UNIT_DIRECTION[UNIT] = 0;
    if (UNIT_LOC_Y[UNIT] != 3) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y--;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a unit present at 
// that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_UNIT()
{
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] != 0 && UNIT_LOC_X[X] == MAP_X && UNIT_LOC_Y[X] == MAP_Y) {
            UNIT_FIND = X; // unit found
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a hidden unit present 
// at that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_HIDDEN_UNIT()
{
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            (UNIT_LOC_X[X] == MAP_X || // first compare horizontal position
             (UNIT_LOC_X[X] <= MAP_X && (UNIT_LOC_X[X] + UNIT_C[X]) >= MAP_X)) && // add hidden unit width
            (UNIT_LOC_Y[X] == MAP_Y || // now compare vertical position
             (UNIT_LOC_Y[X] <= MAP_Y && (UNIT_LOC_Y[X] + UNIT_D[X]) >= MAP_Y))) { // add hidden unit HEIGHT
            UNIT_FIND = X;
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

uint8_t SCR_CUSTOM_KEYS[] = {
    0x55, 0x60, 0x40, 0x03, 0x73, 0x01, 0x14, 0x14, 0x01, 0x03, 0x0B, 0x20, 0x0F, 0x06, 0x20,
    0x14, 0x08, 0x05, 0x20, 0x10, 0x05, 0x14, 0x13, 0x03, 0x09, 0x09, 0x20, 0x12, 0x0F, 0x02,
    0x0F, 0x14, 0x13, 0x6B, 0x60, 0x40, 0x03, 0x49, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x10, 0x12, 0x05, 0x13, 0x13, 0x20, 0x14, 0x08,
    0x05, 0x20, 0x0B, 0x05, 0x19, 0x13, 0x20, 0x19, 0x0F, 0x15, 0x20, 0x17, 0x09, 0x13, 0x08,
    0x20, 0x14, 0x0F, 0x20, 0x15, 0x13, 0x05, 0x60, 0x20, 0x03, 0x5D, 0x5D, 0x60, 0x20, 0x04,
    0x06, 0x0F, 0x12, 0x20, 0x14, 0x08, 0x05, 0x20, 0x06, 0x0F, 0x0C, 0x0C, 0x0F, 0x17, 0x09,
    0x0E, 0x07, 0x20, 0x06, 0x15, 0x0E, 0x03, 0x14, 0x09, 0x0F, 0x0E, 0x13, 0x60, 0x20, 0x05,
    0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25,
    0x5D, 0x5D, 0x60, 0x20, 0x06, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20,
    0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x0C,
    0x05, 0x06, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x0D, 0x0F, 0x16,
    0x05, 0x20, 0x12, 0x09, 0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20,
    0x06, 0x06, 0x09, 0x12, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E, 0x3A, 0x60, 0x20, 0x16,
    0x5D, 0x5D, 0x60, 0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x0C, 0x05, 0x06, 0x14, 0x3A,
    0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x06, 0x09, 0x12, 0x05, 0x20, 0x12, 0x09,
    0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x03, 0x19, 0x03, 0x0C, 0x05,
    0x20, 0x17, 0x05, 0x01, 0x10, 0x0F, 0x0E, 0x13, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x02, 0x03, 0x19, 0x03, 0x0C, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D, 0x13, 0x3A, 0x60,
    0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x05, 0x15, 0x13, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x13, 0x05, 0x01, 0x12, 0x03, 0x08, 0x20, 0x0F,
    0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x02, 0x0D,
    0x0F, 0x16, 0x05, 0x20, 0x0F, 0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x4A, 0x60, 0x40, 0x25, 0x4B
};

char CINEMA_MESSAGE[] = 
    "coming soon: space balls 2 - the search for more money, "
    "attack of the paperclips: clippy's revenge, "
    "it came from planet earth, "
    "rocky 5000, all my circuits the movie, "
    "conan the librarian, and more! comin";

void convertToPETSCII(char* string)
{
    for (char* c = string; *c; c++) {
        if (*c >= 96) {
            *c -= 96;
        }
    }
}

void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    SCREEN_MEMORY[address] = value;
    platform->writeToScreenMemory(address, value, color, yOffset);
}

// NOTES ABOUT UNIT TYPES
// ----------------------
// 000=no unit (does not exist)
// 001=player unit
// 002=hoverbot left-to-right
// 003=hoverbot up-down
// 004=hoverbot attack mode
// 005=hoverbot chase player
// 006=
// 007=transporter
// 008=
// 009=evilbot chase player
// 010=door
// 011=small explosion
// 012=pistol fire up
// 013=pistol fire down
// 014=pistol fire left
// 015=pistol fire right
// 016=trash compactor
// 017=
// 018=
// 019=
// 020=

// NOTES ABOUT UNIT NUMBERING SCHEME
// ---------------------------------
// 0 = player unit
// 1-27 = enemy robots    (max 28 units)
// 28-31 = weapons fire
// 32-47 = doors and other units that don't have sprites (max 16 units)
// 48-63 = hidden objects to be found (max 16 units)

// NOTES ABOUT DOORS.
// -------------------
// A-0=horitzonal 1=vertical
// B-0=opening-A 1=opening-B 2=OPEN / 3=closing-A 4=closing-B 5-CLOSED
// C-0=unlocked / 1=locked spade 2=locked heart 3=locked star
// D-0=automatic / 0=manual

// HIDDEN OBJECTS
// --------------
// UNIT_TYPE:128=key UNIT_A: 0=SPADE 1=HEART 2=STAR
// UNIT_TYPE:129=time bomb
// UNIT_TYPE:130=EMP
// UNIT_TYPE:131=pistol
// UNIT_TYPE:132=charged plasma gun
// UNIT_TYPE:133=medkit
// UNIT_TYPE:134=magnet

// NOTES ABOUT TRANSPORTER
// ----------------------
// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B:    0=completes level 1=send to coordinates
// UNIT_C:    X-coordinate
// UNIT_D:    Y-coordinate

// Sound Effects
// ----------------------
// 0 explosion
// 1 small explosion
// 2 medkit
// 3 emp
// 4 haywire
// 5 evilbot
// 6 move
// 7 electric shock
// 8 plasma gun
// 9 fire pistol
// 10 item found
// 11 error
// 12 change weapons
// 13 change items
// 14 door
// 15 menu beep
// 16 walk
// 17 sfx (short beep)
// 18 sfx
