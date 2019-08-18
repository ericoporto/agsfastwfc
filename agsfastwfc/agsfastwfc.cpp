// dllmain.cpp : Defines the entry point for the DLL application.

#pragma region Defines_and_Includes

#include "core/platform.h"

#define MIN_EDITOR_VERSION 1
#define MIN_ENGINE_VERSION 3

#if AGS_PLATFORM_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include "plugin/agsplugin.h"

#include "./fastwfc/include/utils/array2D.hpp"
#include "./fastwfc/include/color.hpp"
#include "./fastwfc/include/overlapping_wfc.hpp"
#include "./fastwfc/include/wfc.hpp"

// #include "./fastwfc/lib/wave.cpp"
// #include "./fastwfc/lib/propagator.cpp"
// #include "./fastwfc/lib/wfc.cpp"

#if defined(BUILTIN_PLUGINS)
namespace agsfastwfc {
#endif

typedef unsigned char uint8;

#define DEFAULT_RGB_R_SHIFT_32  16
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  0
#define DEFAULT_RGB_A_SHIFT_32  24

#if !AGS_PLATFORM_OS_WINDOWS
#define min(x,y) (((x) < (y)) ? (x) : (y))
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define abs(a)             ((a)<0 ? -(a) : (a))

#pragma endregion

#if AGS_PLATFORM_OS_WINDOWS
// The standard Windows DLL entry point

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD ul_reason_for_call,
                       LPVOID lpReserved) {

        switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
                break;
        }
        return TRUE;
}
#endif

//define engine
IAGSEngine *engine;

#pragma region Color_Functions

int getr32(int c)
{
        return ((c >> DEFAULT_RGB_R_SHIFT_32) & 0xFF);
}


int getg32 (int c)
{
        return ((c >> DEFAULT_RGB_G_SHIFT_32) & 0xFF);
}


int getb32 (int c)
{
        return ((c >> DEFAULT_RGB_B_SHIFT_32) & 0xFF);
}


int geta32 (int c)
{
        return ((c >> DEFAULT_RGB_A_SHIFT_32) & 0xFF);
}


int makeacol32 (int r, int g, int b, int a)
{
        return ((r << DEFAULT_RGB_R_SHIFT_32) |
                (g << DEFAULT_RGB_G_SHIFT_32) |
                (b << DEFAULT_RGB_B_SHIFT_32) |
                (a << DEFAULT_RGB_A_SHIFT_32));
}

#pragma endregion

#pragma region Pixel32_Definition

struct Pixel32 {
public:
        Pixel32();
        ~Pixel32() = default;
        int GetColorAsInt();
        int Red;
        int Green;
        int Blue;
        int Alpha;
};

Pixel32::Pixel32() {
        Red = 0;
        Blue = 0;
        Green = 0;
        Alpha = 0;
}

int Pixel32::GetColorAsInt() {
        return makeacol32(Red,Green,Blue,Alpha);
}

#pragma endregion

/// <summary>
/// Gets the alpha value at coords x,y
/// </summary>
int GetAlpha(int sprite, int x, int y){

        BITMAP *engineSprite = engine->GetSpriteGraphic(sprite);

        unsigned char **charbuffer = engine->GetRawBitmapSurface (engineSprite);
        unsigned int **longbuffer = (unsigned int**)charbuffer;

        int alpha = geta32(longbuffer[y][x]);

        engine->ReleaseBitmapSurface (engineSprite);

        return alpha;
}

/// <summary>
/// Sets the alpha value at coords x,y
/// </summary>
int PutAlpha(int sprite, int x, int y, int alpha){

        BITMAP *engineSprite = engine->GetSpriteGraphic(sprite);

        unsigned char **charbuffer = engine->GetRawBitmapSurface (engineSprite);
        unsigned int **longbuffer = (unsigned int**)charbuffer;


        int r = getr32(longbuffer[y][x]);
        int g = getg32(longbuffer[y][x]);
        int b = getb32(longbuffer[y][x]);
        longbuffer[y][x] = makeacol32(r,g,b,alpha);

        engine->ReleaseBitmapSurface (engineSprite);

        return alpha;
}


/// <summary>
///  Translates index from a 2D array to a 1D array
/// </summary>
int xytolocale(int x, int y, int width){
        return (y * width + x);
}

int Clamp(int val, int min, int max){
        if (val < min) return min;
        else if (val > max) return max;
        else return val;
}

void write_image_sprite_buffer(unsigned int** destlongbuffer, const Array2D<Color>& m) noexcept {
        for (unsigned i = 0; i < (unsigned)m.height; i++) {
                for (unsigned j = 0; j < (unsigned)m.width; j++) {
                        //int locale = xytolocale(j, i, m.width);
                        Color pixel = m.get(i, j);

                        destlongbuffer[i][j] = makeacol32(pixel.r, pixel.g, pixel.b, 255); //ags alpha 0 -> plugin alpha 255
                }
        }
}

using namespace std;
int AgsFastWFC_Overlapping(int destination, int sprite, int seed, bool periodic_input, bool periodic_output, int N, int ground){

        int srcWidth, srcHeight, destWidth, destHeight;

        BITMAP* src = engine->GetSpriteGraphic(sprite);
        BITMAP* dest = engine->GetSpriteGraphic(destination);

        engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
        engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

        unsigned char **srccharbuffer = engine->GetRawBitmapSurface (src);
        unsigned int **srclongbuffer = (unsigned int**)srccharbuffer;

        unsigned char **destcharbuffer = engine->GetRawBitmapSurface (dest);
        unsigned int **destlongbuffer = (unsigned int**)destcharbuffer;

        OverlappingWFCOptions options = {
                periodic_input, periodic_output, destHeight, destWidth, 8 /*no symmetry*/, ground, N
        };

        Array2D<Color> m = Array2D<Color>(srcHeight, srcWidth);
        for(unsigned i = 0; i < (unsigned)srcHeight; i++) {
                for(unsigned j = 0; j < (unsigned)srcWidth; j++) {
                        int srcr, srcg, srcb;
                        srcr =  getr32(srclongbuffer[i][j]);
                        srcg =  getg32(srclongbuffer[i][j]);
                        srcb =  getb32(srclongbuffer[i][j]);
                        m.data[i * srcWidth + j] = { (unsigned char) srcr, (unsigned char) srcg, (unsigned char) srcb};
                }
        }

        OverlappingWFC<Color> wfc( m, options, seed);
        std::optional<Array2D<Color> > success = wfc.run();
        bool isSuccess = false;
        if (success.has_value()) {
                //write_image_png("results/" + name + to_string(i) + ".png", *success);

                cout << " success!" << endl;

                write_image_sprite_buffer(destlongbuffer, *success);

                cout << " finished!" << endl;
                isSuccess = true;
        } else {
                cout << "failed!" << endl;
        }

        engine->ReleaseBitmapSurface(src);
        engine->ReleaseBitmapSurface(dest);
        engine->NotifySpriteUpdated(destination);
        return isSuccess;
}



#if AGS_PLATFORM_OS_WINDOWS

//==============================================================================

// ***** Design time *****

IAGSEditor *editor; // Editor interface

const char *ourScriptHeader =
        "enum TypeSymmetry { \r\n"
        "  eSymmetryX=0, \r\n"
        "  eSymmetryT=1, \r\n"
        "  eSymmetryI=2, \r\n"
        "  eSymmetryL=3, \r\n"
        "  eSymmetryBackslash=4, \r\n"
        "  eSymmetryP=5, \r\n"
        "  eSymmetryNone=8, \r\n"
        "}; \r\n"
        "  \r\n"
        "struct AgsFastWFC { \r\n"
        "  import static bool Overlapping(int destination, int sprite, int seed, bool periodic_input, bool periodic_output, int N=3, int ground=0); \r\n"
        "}; \r\n";


//------------------------------------------------------------------------------

LPCSTR AGS_GetPluginName()
{
        return ("agsfastwfc");
}

//------------------------------------------------------------------------------

int AGS_EditorStartup(IAGSEditor *lpEditor)
{
        // User has checked the plugin to use it in their game

        // If it's an earlier version than what we need, abort.
        if (lpEditor->version < MIN_EDITOR_VERSION)
                return (-1);

        editor = lpEditor;
        editor->RegisterScriptHeader(ourScriptHeader);

        // Return 0 to indicate success
        return (0);
}

//------------------------------------------------------------------------------

void AGS_EditorShutdown()
{
        // User has un-checked the plugin from their game
        editor->UnregisterScriptHeader(ourScriptHeader);
}

//------------------------------------------------------------------------------

void AGS_EditorProperties(HWND parent)                        //*** optional ***
{
        // User has chosen to view the Properties of the plugin
        // We could load up an options dialog or something here instead
/*	MessageBox(parent,
             L"agsfastwfc v1.0 By Calin Leafshade",
             L"About",
         MB_OK | MB_ICONINFORMATION);
 */
}

//------------------------------------------------------------------------------

int AGS_EditorSaveGame(char *buffer, int bufsize)             //*** optional ***
{
        // Called by the editor when the current game is saved to disk.
        // Plugin configuration can be stored in [buffer] (max [bufsize] bytes)
        // Return the amount of bytes written in the buffer
        return (0);
}

//------------------------------------------------------------------------------

void AGS_EditorLoadGame(char *buffer, int bufsize)            //*** optional ***
{
        // Called by the editor when a game is loaded from disk
        // Previous written data can be read from [buffer] (size [bufsize]).
        // Make a copy of the data, the buffer is freed after this function call.
}

//==============================================================================

#endif

// ***** Run time *****

// Engine interface

//------------------------------------------------------------------------------

#define REGISTER(x) engine->RegisterScriptFunction(#x, (void *) (x));
#define STRINGIFY(s) STRINGIFY_X(s)
#define STRINGIFY_X(s) #s

void AGS_EngineStartup(IAGSEngine *lpEngine)
{
        engine = lpEngine;

        // Make sure it's got the version with the features we need
        if (engine->version < MIN_ENGINE_VERSION)
                engine->AbortGame("Plugin needs engine version " STRINGIFY(MIN_ENGINE_VERSION) " or newer.");

        //register functions

        engine->RegisterScriptFunction("AgsFastWFC::Overlapping^7", (void*)AgsFastWFC_Overlapping);

}

//------------------------------------------------------------------------------

void AGS_EngineShutdown()
{
        // Called by the game engine just before it exits.
        // This gives you a chance to free any memory and do any cleanup
        // that you need to do before the engine shuts down.
}

//------------------------------------------------------------------------------

int AGS_EngineOnEvent(int event, int data)                    //*** optional ***
{
        switch (event)
        {
/*
    case AGSE_KEYPRESS:
    case AGSE_MOUSECLICK:
    case AGSE_POSTSCREENDRAW:
    case AGSE_PRESCREENDRAW:
    case AGSE_SAVEGAME:
    case AGSE_RESTOREGAME:
    case AGSE_PREGUIDRAW:
    case AGSE_LEAVEROOM:
    case AGSE_ENTERROOM:
    case AGSE_TRANSITIONIN:
    case AGSE_TRANSITIONOUT:
    case AGSE_FINALSCREENDRAW:
    case AGSE_TRANSLATETEXT:
    case AGSE_SCRIPTDEBUG:
    case AGSE_SPRITELOAD:
    case AGSE_PRERENDER:
    case AGSE_PRESAVEGAME:
    case AGSE_POSTRESTOREGAME:
 */
        default:
                break;
        }

        // Return 1 to stop event from processing further (when needed)
        return (0);
}

//------------------------------------------------------------------------------

int AGS_EngineDebugHook(const char *scriptName,
                        int lineNum, int reserved)            //*** optional ***
{
        // Can be used to debug scripts, see documentation
        return 0;
}

//------------------------------------------------------------------------------

void AGS_EngineInitGfx(const char *driverID, void *data)      //*** optional ***
{
        // This allows you to make changes to how the graphics driver starts up.
        // See documentation
}

//..............................................................................


#if defined(BUILTIN_PLUGINS)
}
#endif
