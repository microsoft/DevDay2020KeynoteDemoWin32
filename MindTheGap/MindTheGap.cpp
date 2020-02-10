// MindTheGap.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "MindTheGap.h"
#include "ScreenInfo.h"
#include <string>

#define MAX_LOADSTRING 100

const int MARGIN = 5;
const int DOUBLE_MARGIN = MARGIN * 2;
const int TEXT_HEIGHT = 40;

ScreenInfo screenInfo{};
HWND hwnd;
HWND textWnd;
HFONT font;

BOOL CALLBACK PaintRect(HDC hdc, const ScreenInfo* info, unsigned int rectIndex);

#pragma region Default project template stuff

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MINDTHEGAP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINDTHEGAP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINDTHEGAP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINDTHEGAP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 720, 500, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    textWnd = CreateWindowW(L"STATIC", L"info", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER, 0, 0, 0, 0, hWnd, 0, hInst, nullptr);
    SetWindowTextW(textWnd, L"");
    font = CreateFontA(36, 0, 0, 0, 100, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
    if (font != nullptr)
    {
        SendMessage(textWnd, WM_SETFONT, (WPARAM)font, TRUE);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

#pragma endregion

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    case WM_MOVE:
    {
        // Any time we move or re-size, we refresh our view of the two screens and see which 
        // one is the "best" one for displaying some content, and then repaint

        // Update the configuration of the screens
        screenInfo.Update(hWnd);

        // Force re-draw
        InvalidateRect(hWnd, nullptr, true);
        break;
    }
    case WM_PAINT:
    {
        // Figure out which screen has the most available space, and then move the
        // simple status text to that screen
        auto bestScreen = screenInfo.GetRect(screenInfo.GetTallestOrWidestIndex());
        MoveWindow(textWnd, bestScreen.left + MARGIN, bestScreen.top + MARGIN, Width(&bestScreen) - DOUBLE_MARGIN, TEXT_HEIGHT, TRUE);

        // Update our status text for new info
        wchar_t buffer[200];
        swprintf_s(buffer, L"Rects: %d, client area: %d x %d", 
            screenInfo.GetRectCount(), 
            Width(&screenInfo.WindowBounds), 
            Height(&screenInfo.WindowBounds));

        SetWindowText(textWnd, buffer);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        for (unsigned int i = 0; i < screenInfo.GetRectCount(); ++i)
        {
            // Paint each rectangle one at a time.
            PaintRect(hdc, &screenInfo, i);
        }
        //EnumDisplayMonitors(hdc, nullptr, PaintEachMonitor, (LPARAM)&screenInfo);
        EndPaint(hWnd, &ps);
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_FILE_REFRESH:
            SendMessageA(hWnd, WM_SIZE, 0, 0);
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case ID_HELP_ABOUT:
            MessageBoxA(hWnd, (std::string("Build timestamp: ") + __TIMESTAMP__).c_str(), "About App", MB_OK);
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK PaintRect(HDC hdc, const ScreenInfo* info, unsigned int rectIndex)
{
    auto rect = info->GetRect(rectIndex);
    auto originalRect = rect;

    // Main info is at the top of the first (left / top) screen only. When in
    // double-landscape, the bottom screen doesn't need the extra margin.
    if (!(info->SplitKind == SplitKind::Horizontal && rectIndex > 0))
    {
        rect.top += TEXT_HEIGHT + MARGIN;
    }

    HBRUSH brush{};

    if (rectIndex == 0)
    {
        brush = CreateSolidBrush(RGB(255, 255, 0));
    }
    else
    {
        brush = CreateSolidBrush(RGB(0, 255, 255));
    }

    auto oldBrush = SelectObject(hdc, brush);
    auto oldFont = SelectObject(hdc, font);
    auto shrunkRect = Shrink(&rect, MARGIN);
    Rectangle(hdc, shrunkRect.left, shrunkRect.top, shrunkRect.right, shrunkRect.bottom);

    wchar_t buffer[200];
    swprintf_s(buffer, L"Rect %d, size: %d x %d\r\n(%d, %d) - (%d, %d)", 
        rectIndex, Width(&originalRect), Height(&originalRect),
        originalRect.left, originalRect.top, originalRect.right - 1, originalRect.bottom - 1);

    shrunkRect = Shrink(&shrunkRect, MARGIN);
    DrawTextW(hdc, buffer, (int) wcslen(buffer), &shrunkRect, DT_CENTER);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldFont);
    DeleteObject(brush);

    return TRUE;
}
