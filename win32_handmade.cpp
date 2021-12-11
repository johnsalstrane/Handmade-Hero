#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

internal LRESULT CALLBACK Win32MainWindowCallback(HWND   Window,
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
    } break;
    case WM_CLOSE:
    {
    } break;
    case WM_ACTIVATEAPP:
    {
    } break;
    case WM_PAINT:
    {
    } break;
    default:
    {
    } break;
    }

    return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_CLASSDC | CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroClass";

	return(0);
}