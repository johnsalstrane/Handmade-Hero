#include <windows.h>
#include <cstdint>
#include <Xinput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef float real32;
typedef double real64;
typedef int32 bool32;
#define Pi32 3.14159265359f

struct win32_offscreen_buffer
{
    BITMAPINFO BitmapInfo;
    void* BitmapMemory;
    int BitmapWidth;
    int BitmapHeight;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
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
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->BitmapMemory)
    {
        VirtualFree(Buffer->BitmapMemory, 0, MEM_RELEASE);
    }

    Buffer->BitmapWidth = Width;
    Buffer->BitmapHeight = Height;
    int BytesPerPixel = 4;


    Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
    Buffer->BitmapInfo.bmiHeader.biWidth = Buffer->BitmapWidth;
    Buffer->BitmapInfo.bmiHeader.biHeight = -Buffer->BitmapHeight;      // negative means we draw top to bottom
    Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
    Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
    Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->BitmapWidth * Buffer->BitmapHeight) * BytesPerPixel;
    Buffer->BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    Buffer->Pitch = Buffer->BitmapWidth * BytesPerPixel;

}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    StretchDIBits(DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer->BitmapWidth, Buffer->BitmapHeight,
        Buffer->BitmapMemory, &Buffer->BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT 
CALLBACK Win32MainWindowCallback(HWND   Window,
                                 UINT   Message,
                                 WPARAM WParam,
                                 LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_SIZE:
    {

    } break;
    case WM_DESTROY:
    {
        GlobalRunning = false;
    } break;
    case WM_CLOSE:
    {
        GlobalRunning = false;
    } break;
    case WM_ACTIVATEAPP:
    {
    } break;
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
        EndPaint(Window, &Paint);
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        bool WasDown = ((LParam & (1 << 30)) != 0);
        bool IsDown = ((LParam & (1 << 31)) == 0);
        uint32 VKCode = WParam;
        if (WasDown != IsDown)
        {
            if (VKCode == 'W')
            {
                OutputDebugStringA("W\n");
            }
            else if (VKCode == 'A')
            {
            }
            else if (VKCode == 'S')
            {
            }
            else if (VKCode == 'D')
            {
            }
            else if (VKCode == 'Q')
            {
            }
            else if (VKCode == 'E')
            {
            }
            else if (VKCode == VK_UP)
            {
            }
            else if (VKCode == VK_LEFT)
            {
            }
            else if (VKCode == VK_DOWN)
            {
            }
            else if (VKCode == VK_RIGHT)
            {
            }
            else if (VKCode == VK_ESCAPE)
            {
                GlobalRunning = false;
            }
            else if (VKCode == VK_SPACE)
            {
                OutputDebugStringA("SPACE: ");
                if (IsDown)
                {
                    OutputDebugStringA("Is Down");
                }
                if (WasDown)
                {
                    OutputDebugStringA("Was Down");
                }
                OutputDebugStringA("\n");
            }
            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if ((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        }

    } break;
    default:
    {
        Result = DefWindowProcA(Window, Message, WParam, LParam);
    } break;
    }

    return(Result);
}

internal void
Win32InitDSound(HWND Window, int SamplesPerSecond, int BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary)
    {
        direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            
            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                BufferDescription.dwSize = sizeof(BufferDescription);
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {

                    if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {

                    }
                }
            }
            else
            {
                // Log this, SetCooperativeLevel failed
            }
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
            {

            }
        }
    }
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    Win32LoadXInput();

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_CLASSDC | CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroClass";

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
            WindowClass.lpszClassName,
            "Handmade Hero Window",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0,
            Instance, 0);

        if (Window)
        {
            HDC DeviceContext = GetDC(Window);

            uint32 RunningSampleIndex = 0;
            int ToneHz = 256;
            int16 ToneVolume = 3000;
            int SamplesPerSecond = 48000;
            int SquareWavePeriod = SamplesPerSecond / ToneHz;
            int HalfSquareWavePeriod = SquareWavePeriod / 2;
            int BytesPerSample = sizeof(int16) * 2;
            int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;
            Win32InitDSound(Window, SamplesPerSecond, SecondaryBufferSize);
            bool32 SoundIsPlaying = false;

            int BlueOffset = 0;
            int GreenOffset = 0;
            
            GlobalRunning = true;
            while (GlobalRunning)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // This controller is plugged in
                        XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;
                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                    }
                    else
                    {
                        // This controller is not available
                    }
                }

                RenderWeirdGradient(&GlobalBackbuffer, BlueOffset, GreenOffset);

                void* Region1;
                DWORD Region1Size;
                void* Region2;
                DWORD Region2Size;

                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    DWORD WritePointer; // TODO(John): Remove?
                    DWORD BytesToWrite;
                    DWORD ByteToLock = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;

                    if (ByteToLock == PlayCursor)
                    {
                        BytesToWrite = SecondaryBufferSize;
                    }
                    else if (ByteToLock > PlayCursor)
                    {
                        BytesToWrite = (SecondaryBufferSize - ByteToLock);
                        BytesToWrite += PlayCursor;
                    }
                    else
                    {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }

                    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
                    {
                        DWORD Region1SampleCount = Region1Size / BytesPerSample;
                        DWORD Region2SampleCount = Region2Size / BytesPerSample;

                        int16* SampleOut = (int16*)Region1;
                        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
                        {
                            
                            int16 SampleValue = ((RunningSampleIndex/HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                            ++RunningSampleIndex;
                        }

                        SampleOut = (int16*)Region2;
                        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
                        {
                            
                            int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                            ++RunningSampleIndex;
                        }
                    }
                }
                GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
                ++BlueOffset;

                if (!SoundIsPlaying)
                {
                    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
                    SoundIsPlaying = true;
                }
            }
        }
        else
        {
            // Log an error, CreateWindowEX failed
        }
    }
    else
    {
        // Log an error, RegisterClass failed
    }

	return(0);
}