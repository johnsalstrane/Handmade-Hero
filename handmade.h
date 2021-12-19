#if !defined(HANDMADE_H)
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


// Services that the platform layer provides to the game: Whad do you want to render,
// what do you want to play out of the sound card, what do you want to do for files?


// Services that the game provides to the platform layer

internal void
GameUpdateAndRender(game_offscreen_buffer* Buffer, game_sound_output_buffer *SoundBuffer, int BlueOffset, int GreenOffset, int ToneHz);

#define HANDMADE_H
#endif