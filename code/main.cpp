// Windows headers
#include <Windows.h>
#include <shlobj_core.h> 
#include <pathcch.h>
#pragma comment(lib, "pathcch.lib")
#include <gdiplus.h>
// End of windows headers
#include <stdint.h>
#include "std.h"
#include "Capture.h"

using namespace Gdiplus;

#define internal static
#define global_var static

typedef uint8_t 	uint8;
typedef uint16_t 	uint16;
typedef uint32_t 	uint32;
typedef uint64_t 	uint64;
typedef int8_t 	int8;
typedef int16_t 	int16;
typedef int32_t 	int32;
typedef int64_t 	int64;



global_var bool isCropping = false;
global_var vstdRect Crop;
global_var BITMAPINFO BitmapInfo;
global_var void *BackBuffer;
global_var int BitmapWidth;
global_var int BitmapHeight;


internal void
Win32ResizeDIBSection(int w, int h)
{
	#if 1
		w = GetSystemMetrics(SM_CXSCREEN);
		h = GetSystemMetrics(SM_CYSCREEN);
		
		
	#endif
#if 0
	if(BackBuffer)
	{
		VirtualFree(BackBuffer, 0, MEM_RELEASE);
	}
#endif
	BitmapWidth  = w;
	BitmapHeight = h;
	
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32; // TODO: Might need to change to 24
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	int BytesPerPixel = 4;
	int BackBufferSize = (w * h) * BytesPerPixel;
	if(!BackBuffer) BackBuffer = VirtualAlloc(NULL, BackBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC WindowDC, RECT WindowRect)
{
	char *pixels = (char *)BackBuffer;
	int WindowWidth  = WindowRect.right - WindowRect.left;
	int WindowHeight = WindowRect.bottom - WindowRect.top;
	
	StretchDIBits( 	WindowDC,
					0, 0, BitmapWidth, BitmapHeight,
					0, 0, WindowWidth, WindowHeight,
					pixels,
					&BitmapInfo,
					DIB_RGB_COLORS,
					SRCCOPY
	);
}


LRESULT CALLBACK WindowProc(
	_In_ HWND   Window,
	_In_ UINT   Message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)

{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC PaintDC = BeginPaint(Window, &Paint);
			
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			
			Win32UpdateWindow(PaintDC, ClientRect);
			
			EndPaint(Window, &Paint);
		}
		case WM_MOUSEMOVE: 
		{
			if(Crop.x1 != -1)
			{
				HDC WindowDC = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);

				Win32UpdateWindow(WindowDC, ClientRect);

				RECT rect;
				POINT p;
				GetCursorPos(&p);

				if (p.x > Crop.x1)
				{
					rect.left = Crop.x1;
					rect.right = p.x;
				}
				else
				{
					rect.left = p.x;
					rect.right = Crop.x1;
				}
				if (p.y > Crop.y1)
				{
					rect.top = Crop.y1;
					rect.bottom = p.y;
				}
				else
				{
					rect.top = p.y;
					rect.bottom = Crop.y1;
				}

				FrameRect(WindowDC, &rect, CreateSolidBrush(0x000000FF)); // Red

			}
		} break;
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			
			Win32ResizeDIBSection(Width, Height);
		}
		case WM_SETCURSOR:
		{	
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		} break;
		case WM_CLOSE:
		{
			PostQuitMessage(0);
		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		default:
		{
			Result = DefWindowProcW(Window, Message, wParam, lParam);
		} break;
	}
	return Result;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR  CmdLine, int ShowCmd)
{
	uint64 token;
	GdiplusStartupInput gdi_inp = {};
	gdi_inp.GdiplusVersion = 1;
	gdi_inp.DebugEventCallback = NULL;
	gdi_inp.SuppressBackgroundThread = FALSE;
	gdi_inp.SuppressExternalCodecs = FALSE;


	GdiplusStartup(&token, &gdi_inp, NULL);

	int ScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
	int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);


	
	WNDCLASSW wnd = {};
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc = WindowProc;
	wnd.hInstance = Instance;
	wnd.hCursor = LoadCursor(wnd.hInstance, IDC_ARROW);
	wnd.lpszClassName = L"CaptureScreenshotUtilityWindowClass";
	if(RegisterClassW(&wnd))
	{
		HWND Window = CreateWindowExW(0, wnd.lpszClassName, L"Capture", WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, 0, 0, Instance, 0);
		if(Window)
		{
			
			ShowWindow(Window, SW_HIDE);
			RAWINPUTDEVICE Devices[2];					// 1 = Keyboard 2 = Mouse

			Devices[0].usUsagePage = 0x0001;			// Keyboard / Mouse
			Devices[0].usUsage = 0x0006;				// Keyboard
			Devices[0].dwFlags = RIDEV_INPUTSINK;		// Get input when not in focus
			Devices[0].hwndTarget = Window;

			Devices[1].usUsagePage = 0x0001;			// Keyboard / Mouse
			Devices[1].usUsage = 0x0002;				// Mouse
			Devices[1].dwFlags = RIDEV_INPUTSINK;		// Get input when not in focus
			Devices[1].hwndTarget = Window;

			if (RegisterRawInputDevices(Devices, 2, sizeof(Devices[0])) == FALSE) Error("RegisterRawInputDevice failed!\n");
			bool ctrlIsDown = false;
			bool shiftIsDown = false;

			Crop.x1 = -1;
			bool ShouldShowWindow = false;
			bool leftIsDown = false;

			MSG Message;
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

			while(GetMessage(&Message, 0, 0, 0) > 0)
			{
				TranslateMessage(&Message);
				DispatchMessageW(&Message);
				if(Message.message == WM_INPUT)
				{
					UINT DataSize;
					if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, NULL, &DataSize, sizeof(RAWINPUTHEADER)) == -1)
					{
						Error("Failed getting raw input amaount\n");
					}
					RAWINPUT *Raw;
					Raw = (PRAWINPUT)VirtualAlloc(NULL, DataSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					if (!Raw)
					{
						Error("Failed to allocate memory!\n");
					}
					if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, Raw, &DataSize, sizeof(RAWINPUTHEADER)) == -1)  // <-- Fails here
					{
						Error("Failed getting raw input\n");
					}


					if(Raw->header.dwType == RIM_TYPEKEYBOARD)
					{
						unsigned short flags = Raw->data.keyboard.Flags;
						unsigned short vkey = Raw->data.keyboard.VKey;
						bool isDown = ((flags & RI_KEY_BREAK) == 0);
						switch(vkey)
						{
							case VK_CONTROL:
							{
								ctrlIsDown = isDown;
								break;
							}
							case VK_SHIFT:
							{
								shiftIsDown = isDown;
								break;
							}
//							VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
							case 0x53: // S
							{
								if(ctrlIsDown && shiftIsDown && isDown)
								{
									ShouldShowWindow = true;
									ctrlIsDown = false;
									shiftIsDown = false;

									CaptureScreen(Window, BackBuffer, ScreenWidth, ScreenHeight);
										
									HDC WindowDC = GetDC(Window);
									RECT ClientRect;
									GetClientRect(Window, &ClientRect);
									Win32UpdateWindow(WindowDC, ClientRect);
									ReleaseDC(Window, WindowDC);
									Crop.x1 = -1;
									isCropping = true;

								//	DrawPixelsToScreen(Window, BackBuffer, ScreenWidth, ScreenHeight);
								}
							}
						}
					}
					else if(Raw->header.dwType == RIM_TYPEMOUSE)
					{
						RedrawWindow(Window, NULL, NULL, RDW_INTERNALPAINT);
						if(isCropping && Raw->data.mouse.usButtonFlags != 0)
						{
							unsigned short flags = Raw->data.mouse.usButtonFlags;
							switch (flags)
							{
								case RI_MOUSE_LEFT_BUTTON_DOWN:
								{
									leftIsDown = true;
									POINT point;
									GetCursorPos(&point);
									Crop.x1 = point.x;
									Crop.y1 = point.y;
									break;
								}
								case RI_MOUSE_LEFT_BUTTON_UP:
								{
									if (leftIsDown)
									{
										ShouldShowWindow = false;
										leftIsDown = false;
										POINT point;
										GetCursorPos(&point);
										Crop.x2 = point.x;
										Crop.y2 = point.y;
										int bottom;
										int right;
										int top;
										int left;
										if (Crop.x2 > Crop.x1)
										{
											left = Crop.x1;
											right = Crop.x2;
										}
										else
										{
											left = Crop.x2;
											right = Crop.x1;
										}
										if (Crop.y2 > Crop.y1)
										{
											top = Crop.y1;
											bottom = Crop.y2;
										}
										else
										{
											top = Crop.y2;
											bottom = Crop.y1;
										}


										HDC WindowDC = GetDC(NULL);
										HDC hdc = CreateCompatibleDC(WindowDC);
										HBITMAP hbmp = CreateCompatibleBitmap(WindowDC,
											ScreenWidth, ScreenHeight);
										SelectObject(hdc, hbmp);

										PrintWindow(Window, hdc, PW_CLIENTONLY);

										HDC CustomDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
										int new_w = right - left;
										int new_h = bottom - top;
										HDC CustomComp = CreateCompatibleDC(CustomDC);
										HBITMAP CropBitmap = CreateCompatibleBitmap(CustomDC, new_w, new_h);
										SelectObject(CustomComp, CropBitmap);

										BitBlt(CustomComp, 0, 0, new_w, new_h, hdc, left, top, SRCCOPY);
											
										Bitmap* img = new Bitmap(CropBitmap, NULL);


										CopyImageToClipboard(Window, img);
											
										SetWindowPos(Window, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
										delete img;
										DeleteDC(CustomDC);
										DeleteObject(hbmp);
										DeleteDC(hdc);
										DeleteObject(CropBitmap);
										ReleaseDC(Window, WindowDC);				
										Crop.x1 = -1;
										Crop.x2 = 0;
										Crop.y1 = 0;
										Crop.y2 = 0;
										isCropping = false;
									}
									break;
								}
							}
						}
					}
				VirtualFree(Raw, 0, MEM_RELEASE);
				}
			}
			
		}
	
	}
    return 0;
}