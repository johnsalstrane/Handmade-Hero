#include <windows.h>
#include <cstdint>

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
#define Pi32 3.14159265359f

global_variable bool GlobalRunning;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;


internal void
RenderWeirdGradient(int BlueOffset, int GreenOffset)
{
    uint8* Row = (uint8*)BitmapMemory;
    int Pitch = BitmapWidth * BytesPerPixel;
    for (int Y = 0; Y < BitmapHeight; Y++)
    {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < BitmapWidth; X++)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;      // negative means we draw top to bottom
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect.right - ClientRect.left;
    int WindowHeight = ClientRect.bottom - ClientRect.top;
    StretchDIBits(DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, BitmapWidth, BitmapHeight,
        BitmapMemory, &BitmapInfo,
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
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;
        Win32ResizeDIBSection(Width, Height);
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
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);
        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
        Win32UpdateWindow(DeviceContext, ClientRect, X, Y, Width, Height);
        EndPaint(Window, &Paint);
    } break;
    default:
    {
        Result = DefWindowProcA(Window, Message, WParam, LParam);
    } break;
    }

    return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_CLASSDC | CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroClass";

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
                RenderWeirdGradient(BlueOffset, GreenOffset);
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                ++BlueOffset;
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