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
            uint8 Blue = (uint8)(X + BlueOffset);
            uint8 Green = (uint8)(Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}

internal void
GameOutputSound(game_sound_output_buffer* SoundBuffer, int ToneHz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 9000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16* SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
}

internal void
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer* Buffer, game_sound_output_buffer *SoundBuffer)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state* GameState = (game_state*)Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        char* Filename = __FILE__;
        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if (File.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }
        GameState->ToneHz = 256;
        //GameState->GreenOffset = 0; //unnecessary, since VirtualAlloc sets memory to 0
        //GameState->BlueOffset = 0;
        Memory->IsInitialized = true;
    }

    game_controller_input* Input0 = &Input->Controllers[0];
    if (Input0->IsAnalog)
    {
        GameState->ToneHz = 256 + (int)(128.0f * (Input0->EndX));
        GameState->BlueOffset += (int)4.0f * (int)(Input0->EndY);
    }
    else
    {
        // use digital movement tuning
    }

    if (Input0->Down.EndedDown)
    {
        GameState->GreenOffset += 1;
    }
    if (Input0->Up.EndedDown)
    {
        GameState->GreenOffset -= 1;
    }
    if (Input0->Right.EndedDown)
    {
        GameState->ToneHz += 1;
    }
    if (Input0->Left.EndedDown)
    {
        if ((GameState->ToneHz - 1) != 0)
        {
            GameState->ToneHz -= 1;
        }
    }

    RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
    GameOutputSound(SoundBuffer, GameState->ToneHz);
}

#define HANDMADE_CPP
#endif