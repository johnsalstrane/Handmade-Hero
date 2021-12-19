#if !defined(HANDMADE_H)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

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

// Services that the platform layer provides to the game: Whad do you want to render,
// what do you want to play out of the sound card, what do you want to do for files?


// Services that the game provides to the platform layer

internal void
GameUpdateAndRender(game_input* Input, game_offscreen_buffer* Buffer, game_sound_output_buffer *SoundBuffer);

#define HANDMADE_H
#endif