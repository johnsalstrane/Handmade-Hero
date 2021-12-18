#if !defined(HANDMADE_H)
struct game_offscreen_buffer
{
    void* BitmapMemory;
    int BitmapWidth;
    int BitmapHeight;
    int Pitch;
};


// Services that the platform layer provides to the game


// Services that the game provides to the platform layer

internal void
GameUpdateAndRender(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif