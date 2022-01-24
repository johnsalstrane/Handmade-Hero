#if !defined(HANDMADE_H)

/*
    HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

    HANDMADE_SLOW:
    1 - No slow code allowed
    0 - Slow code welcome
*/

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int*)0=0;}
#else
Assert(Expression)
#endif

struct game_offscreen_buffer
{
    void* BitmapMemory;
    int BitmapWidth;
    int BitmapHeight;
    int Pitch;
};

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16* Samples;
};

struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    real32 StartX;
    real32 MaxX;
    real32 MinX;
    real32 EndX;

    real32 StartY;
    real32 MaxY;
    real32 MinY;
    real32 EndY;

    bool32 IsAnalog;

    union
    {
        game_button_state Buttons[6];
        struct
        {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input
{
    game_controller_input Controllers[4];
};

struct game_state
{
    int ToneHz;
    int GreenOffset;
    int BlueOffset;
};

struct game_memory
{
    uint64 PermanentStorageSize;
    void* PermanentStorage;
    bool32 IsInitialized;
    uint64 TransientStorageSize;
    void* TransientStorage;
};

// Services that the platform layer provides to the game: Whad do you want to render,
// what do you want to play out of the sound card, what do you want to do for files?
#if HANDMADE_INTERNAL
struct debug_read_file_result
{
    uint32 ContentsSize;
    void* Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char* Filename);
internal void DEBUGPlatformFreeFileMemory(void* Memory);
internal bool32 DEBUGPlatformWriteEntireFile(char* Filename, uint32 MemorySize, void* Memory);
#endif

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    Assert(Value < 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

// Services that the game provides to the platform layer

internal void
GameUpdateAndRender(game_memory *Memory, game_input* Input, game_offscreen_buffer* Buffer, game_sound_output_buffer *SoundBuffer);

#define HANDMADE_H
#endif