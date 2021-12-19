#if !defined(WIN32_HANDMADE_H)

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_offscreen_buffer
{
    BITMAPINFO BitmapInfo;
    void* BitmapMemory;
    int BitmapWidth;
    int BitmapHeight;
    int Pitch;
};

struct win32_sound_output
{
    uint32 RunningSampleIndex;
    int SamplesPerSecond;
    int BytesPerSample;
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

#define WIN32_HANDMADE_H
#endif