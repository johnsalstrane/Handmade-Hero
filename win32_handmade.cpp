/*
Platform Layer To-Do list
Saved game locations
getting a handle to our own executable file
asset loading path
threading
raw input (support for multiple keyboards)
sleep/time begin period (for laptops to save battery)
ClipCursor()  (for multimonitor support)
fullscreen support
WM_SETCURSOR (control cursor visibility)
QueryCancelAutoplay
WM_ACTIVATEAPP (for when we are not the active app)
Blit speed improvements (BitBlit)
Hardware acceleration (OpenGL or Direct3D or both)
GetKEyboardLayout (for French keyboards, international WASD support)
*/


#include <windows.h>
#include <cstdint>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include <malloc.h>

#define internal static
#define local_persist static
#define global_variable static
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float real32;
typedef double real64;
typedef int32 bool32;
#define Pi32 3.14159265359f

#include "handmade.cpp"
#include "win32_handmade.h"

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
        Assert(!"Keyboard input came in through a non-dispatch message!!");
        

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

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    void* Region1;
    DWORD Region1Size;
    void* Region2;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;

        int16* DestSample = (int16*)Region1;
        int16* SourceSample = SourceBuffer->Samples;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DestSample = (int16*)Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ClearBuffer(win32_sound_output* SoundOutput)
{
    void* Region1;
    DWORD Region1Size;
    void* Region2;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        uint8* DestSample = (uint8*)Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        DestSample = (uint8*)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ProcessKeyboardMessage(game_button_state* NewState, bool32 IsDown)
{
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, game_button_state* OldState, DWORD ButtonBit, game_button_state* NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal debug_read_file_result 
DEBUGPlatformReadEntireFile(char* Filename)
{
    debug_read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && BytesRead == FileSize32)
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
        }
        CloseHandle(FileHandle);
    }
    return(Result);
}
internal void 
DEBUGPlatformFreeFileMemory(void* Memory)
{
    if (Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}
internal bool32 
DEBUGPlatformWriteEntireFile(char* Filename, uint32 MemorySize, void* Memory)
{
    bool32 Result = false;
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            Result = (BytesWritten == MemorySize);
        }
        CloseHandle(FileHandle);
    }
    return(Result);
}

internal void
Win32ProcessPendingMessages(game_controller_input* KeyboardController)
{
    MSG Message;
    
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch (Message.message)
        {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool WasDown = ((Message.lParam & (1 << 30)) != 0);
            bool IsDown = ((Message.lParam & (1 << 31)) == 0);
            uint32 VKCode = (uint32)Message.wParam;
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
                    Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);

                }
                else if (VKCode == 'E')
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);

                }
                else if (VKCode == VK_UP)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
                }
                else if (VKCode == VK_LEFT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Left, IsDown);
                }
                else if (VKCode == VK_DOWN)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Down, IsDown);
                }
                else if (VKCode == VK_RIGHT)
                {
                    Win32ProcessKeyboardMessage(&KeyboardController->Right, IsDown);
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
                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if ((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }
                if (VKCode == WM_QUIT)
                {
                    GlobalRunning = false;
                }
            }
        } break;

        default:
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        } break;
        }
    }
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

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

            win32_sound_output SoundOutput = {};
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;

            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            bool32 SoundIsPlaying = false;
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            int16* Samples = (int16*)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif

            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes(4);

            uint64 TotalSize = GameMemory.TransientStorageSize + GameMemory.PermanentStorageSize;
            GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorage = ((uint8*)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);
            if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {

                game_input Input[2] = {};
                game_input* NewInput = &Input[0];
                game_input* OldInput = &Input[1];

                GlobalRunning = true;
                LARGE_INTEGER LastCounter;
                QueryPerformanceCounter(&LastCounter);
                uint64 LastCycleCount = __rdtsc();
                while (GlobalRunning)
                {
                    game_controller_input* KeyboardController = &NewInput->Controllers[0];
                    game_controller_input ZeroController = {};
                    *KeyboardController = ZeroController;
                    
                    Win32ProcessPendingMessages(KeyboardController);

                    DWORD MaxControllerCount = XUSER_MAX_COUNT;
                    if (MaxControllerCount > ArrayCount(NewInput->Controllers))
                    {
                        MaxControllerCount = ArrayCount(NewInput->Controllers);
                    }
                    for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
                    {
                        game_controller_input* OldController = &OldInput->Controllers[ControllerIndex];
                        game_controller_input* NewController = &NewInput->Controllers[ControllerIndex];
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

                            NewController->IsAnalog = true;
                            NewController->StartX = OldController->EndX;
                            NewController->StartY = OldController->EndY;

                            real32 X;
                            if (Pad->sThumbLX < 0)
                            {
                                X = (real32)Pad->sThumbLX / 32768.0f;
                            }
                            else
                            {
                                X = (real32)Pad->sThumbLX / 32767.0f;
                            }
                            NewController->MinX = NewController->MaxX = NewController->EndX = X;
                            real32 Y;
                            if (Pad->sThumbLY < 0)
                            {
                                Y = (real32)Pad->sThumbLY / 32768.0f;
                            }
                            else
                            {
                                Y = (real32)Pad->sThumbLY / 32767.0f;
                            }
                            NewController->MinY = NewController->MaxY = NewController->EndY = Y;


                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Down, XINPUT_GAMEPAD_A, &NewController->Down);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Right, XINPUT_GAMEPAD_A, &NewController->Right);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Left, XINPUT_GAMEPAD_A, &NewController->Left);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Up, XINPUT_GAMEPAD_A, &NewController->Up);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder, XINPUT_GAMEPAD_A, &NewController->LeftShoulder);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_A, &NewController->RightShoulder);
                        }
                        else
                        {
                            // This controller is not available
                        }
                    }

                    DWORD PlayCursor = 0;
                    DWORD WriteCursor = 0;
                    DWORD ByteToLock = 0;
                    DWORD BytesToWrite = 0;
                    DWORD TargetCursor = 0;
                    bool32 SoundIsValid = false;

                    if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                    {
                        TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);
                        ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

                        if (ByteToLock > TargetCursor)
                        {
                            BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                            BytesToWrite += TargetCursor;
                        }
                        else
                        {
                            BytesToWrite = TargetCursor - ByteToLock;
                        }
                        SoundIsValid = true;
                    }

                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                    SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                    SoundBuffer.Samples = Samples;

                    game_offscreen_buffer Buffer = {};
                    Buffer.BitmapMemory = GlobalBackbuffer.BitmapMemory;
                    Buffer.BitmapWidth = GlobalBackbuffer.BitmapWidth;
                    Buffer.BitmapHeight = GlobalBackbuffer.BitmapHeight;
                    Buffer.Pitch = GlobalBackbuffer.Pitch;
                    GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

                    if (SoundIsValid)
                    {
                        Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);

                    }

                    RECT ClientRect;
                    GetClientRect(Window, &ClientRect);
                    int WindowWidth = ClientRect.right - ClientRect.left;
                    int WindowHeight = ClientRect.bottom - ClientRect.top;
                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);

                    LARGE_INTEGER EndCounter;
                    QueryPerformanceCounter(&EndCounter);
                    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    int32 MSPerFrame = (int32)((1000 * CounterElapsed) / PerfCountFrequency);
                    int32 FPS = (uint32)(PerfCountFrequency / CounterElapsed);
                    uint64 EndCycleCount = __rdtsc();
                    uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                    int32 MCPF = (int32)(CyclesElapsed / (1000 * 1000));
                    char StringBuffer[256];
                    wsprintf(StringBuffer, "Milliseconds/frame: %dms    %dFPS    %dmc/f\n", MSPerFrame, FPS, MCPF);
                    OutputDebugStringA(StringBuffer);
                    LastCounter = EndCounter;

                    game_input* Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
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