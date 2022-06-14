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

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input* Controller = GetController(Input, ControllerIndex);
        if (Controller->IsAnalog)
        {
            GameState->ToneHz = 256 + (int)(128.0f * (Controller->StickAverageY));
            GameState->BlueOffset += (int)4.0f * (int)(Controller->StickAverageX);
        }
        else
        {
            // use digital movement tuning
            if (Controller->MoveLeft.EndedDown)
            {
                GameState->BlueOffset -= 1;
            }
            if (Controller->MoveRight.EndedDown)
            {
                GameState->BlueOffset += 1;
            }
            if (Controller->MoveUp.EndedDown)
            {
                GameState->ToneHz -= 1;
            }
            if (Controller->MoveUp.EndedDown)
            {
                GameState->ToneHz += 1;
            }
        }
        /*
        if (Controller->ActionDown.EndedDown)
        {
            GameState->GreenOffset += 1;
        }
        if (Controller->ActionUp.EndedDown)
        {
            GameState->GreenOffset -= 1;
        }
        if (Controller->ActionRight.EndedDown)
        {
            GameState->ToneHz += 1;
        }
        if (Controller->ActionLeft.EndedDown)
        {
            if ((GameState->ToneHz - 1) != 0)
            {
                GameState->ToneHz -= 1;
            }
        }
        */
    }

    RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
    GameOutputSound(SoundBuffer, GameState->ToneHz);
}

#define HANDMADE_CPP
#endif