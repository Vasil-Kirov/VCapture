#pragma once
#include <Windows.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

#include "std.h"
#include <stdio.h>

using namespace Gdiplus;

typedef uint8_t 	uint8;
typedef uint16_t 	uint16;
typedef uint32_t 	uint32;
typedef uint64_t 	uint64;
typedef int8_t 	int8;
typedef int16_t 	int16;
typedef int32_t 	int32;
typedef int64_t 	int64;


void CaptureScreen(HWND Window, void *in_pixels, int ScreenWidth, int ScreenHeight)
{

	char *pixels = (char *)in_pixels;
    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
        ScreenWidth, ScreenHeight);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0, 0, ScreenWidth, ScreenHeight,
        hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);


    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    if (GetDIBits(hCaptureDC, hCaptureBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS) == 0)
    {
        Error("Coudln't get bmi structure");
    }
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;  // no compression -> easier to use
    // corvstdRect the bottom-up ordering of lines (abs is in cstdblib and stdlib.h)
    bmi.bmiHeader.biHeight = abs(bmi.bmiHeader.biHeight);


    GetDIBits(hCaptureDC, hCaptureBitmap, 0, ScreenHeight, pixels, &bmi, DIB_RGB_COLORS);
   

    SetWindowPos(Window, HWND_TOPMOST, 0, 0, ScreenWidth, ScreenHeight, SWP_SHOWWINDOW);
    ReleaseDC(hDesktopWnd, hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);

}
void CopyFileToClipboard(HWND Window, LPCWSTR path)
{
	if(!OpenClipboard(Window)) Error("Failed to access clipboard\n");
	EmptyClipboard();
	int size = sizeof(DROPFILES)+((lstrlenW(path)+2)*sizeof(WCHAR));
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	DROPFILES *df = (DROPFILES*) GlobalLock(hGlobal);
	ZeroMemory(df, size);
	df->pFiles = sizeof(DROPFILES);
	df->fWide = TRUE;
	LPWSTR ptr = (LPWSTR) (df + 1);
	lstrcpyW(ptr, path);
	GlobalUnlock(hGlobal);
	SetClipboardData(CF_HDROP, hGlobal);
	CloseClipboard();
}

void CopyImageToClipboard(HWND Window, Bitmap *gdibmp) 
{
    if (gdibmp->GetLastStatus() != Gdiplus::Ok)
        return;

    HBITMAP hbitmap;
    auto status = gdibmp->GetHBITMAP(NULL, &hbitmap);
    if (status != Gdiplus::Ok)
        return;
    BITMAP bm;
    GetObject(hbitmap, sizeof bm, &bm);

    BITMAPINFOHEADER bi =
    { sizeof(bi), bm.bmWidth, bm.bmHeight, 1, bm.bmBitsPixel, BI_RGB };

    int vec_size = bm.bmWidthBytes * bm.bmHeight;
    char* vec = (char *)VirtualAlloc(NULL, vec_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    auto hdc = GetDC(NULL);
    GetDIBits(hdc, hbitmap, 0, bi.biHeight, vec, (BITMAPINFO*)&bi, 0);
    ReleaseDC(NULL, hdc);

    auto hmem = GlobalAlloc(GMEM_MOVEABLE, sizeof bi + vec_size);
    auto buffer = (BYTE*)GlobalLock(hmem);
    memcpy(buffer, &bi, sizeof(bi));
    memcpy(buffer + sizeof(bi), vec, vec_size);
    GlobalUnlock(hmem);

    if (OpenClipboard(Window))
    {
        EmptyClipboard();
        SetClipboardData(CF_DIB, hmem);
        CloseClipboard();
    }
    VirtualFree(vec, NULL, MEM_RELEASE);
    DeleteObject(hbitmap);
}

