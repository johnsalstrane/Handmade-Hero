#if !defined(HANDMADE_CPP)
#include "handmade.h"

internal void
RenderWeirdGradient(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset)
{
    uint8* Row = (uint8*)Buffer->BitmapMemory;
    for (int Y = 0; Y < Buffer->BitmapHeight; Y++)
    {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < Buffer->BitmapWidth; X++)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}

internal void
GameUpdateAndRender(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset)
{
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}

#define HANDMADE_CPP
#endif