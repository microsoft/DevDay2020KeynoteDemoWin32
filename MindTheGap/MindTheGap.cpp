// MindTheGap.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "MindTheGap.h"
#include "ScreenInfo.h"
#include <string>

#define MAX_LOADSTRING 100

const int MARGIN = 5;
const int TEXT_HEIGHT = 32;

ScreenInfo screenInfo{};
HWND hwnd;
HWND textWnd;
HFONT font;
HPEN pen;

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

    font = CreateFontA(30, 0, 0, 0, 100, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
    pen = CreatePen(PS_SOLID, 2, RGB(128, 128, 128));

    textWnd = CreateWindowW(L"STATIC", L"info", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER, 0, 0, 0, 0, hWnd, 0, hInst, nullptr);
    SetWindowTextW(textWnd, L"");
    SendMessage(textWnd, WM_SETFONT, (WPARAM)font, TRUE);

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

        // Figure out which screen has the most available space, and then move the
        // simple status text to that screen
        auto bestScreen = Shrink(screenInfo.GetRect(screenInfo.GetTallestOrWidestIndex()), MARGIN);
        MoveWindow(textWnd, bestScreen.left, bestScreen.top, Width(bestScreen), TEXT_HEIGHT, TRUE);

        // Update our status text for new info
        static wchar_t buffer[500];
        swprintf_s(buffer, L"Rects: %d%s, client area: %d x %d", screenInfo.GetRectCount(), 
            screenInfo.IsEmulating() ? L" (emu'ed)" : L"",
            Width(screenInfo.GetWindowBounds()), Height(screenInfo.GetWindowBounds()));
        SetWindowText(textWnd, buffer);

        // Force re-draw
        InvalidateRect(hWnd, nullptr, true);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HBRUSH brush{};

        auto oldFont = SelectObject(hdc, font);
        auto oldPen = SelectObject(hdc, pen);
        for (unsigned int i = 0; i < screenInfo.GetRectCount(); ++i)
        {
            auto thisRect(screenInfo.GetRect(i));

            if (i % 2)
            {
                brush = CreateSolidBrush(RGB(255, 255, 0));
            }
            else
            {
                brush = CreateSolidBrush(RGB(0, 255, 255));
            }

            static wchar_t buffer[500];
            swprintf_s(buffer, L"Rect #%d, size: %d x %d\r\n(%d, %d) - (%d, %d)",
                i, Width(thisRect), Height(thisRect),
                thisRect.left, thisRect.top, thisRect.right - 1, thisRect.bottom - 1);


            // Shrink the rect for a bit of a margin
            auto shrunkRect = Shrink(thisRect, MARGIN);

            // Add space for the heading text unless we're on the bottom-screen of a dual-
            // screen device
            if (!(i == 1 && screenInfo.GetSplitKind() == SplitKind::Horizontal))
            {
                shrunkRect.top += TEXT_HEIGHT + MARGIN;
            }

            auto oldBrush = SelectObject(hdc, brush);
            Rectangle(hdc, shrunkRect.left, shrunkRect.top, shrunkRect.right, shrunkRect.bottom);

            // Shrink again for another margin
            shrunkRect = Shrink(shrunkRect, MARGIN);
            DrawTextW(hdc, buffer, (int)wcslen(buffer), &shrunkRect, DT_CENTER);

            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }
        SelectObject(hdc, oldFont);
        SelectObject(hdc, oldPen);
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

        case ID_HELP_ABOUT:
            MessageBoxA(hWnd, (std::string("Build timestamp: ") + __TIMESTAMP__).c_str(), "About App", MB_OK);
            break;

        case ID_TOOLS_TOGGLEMODES:
        {
            bool currentlyEmulating{ screenInfo.IsEmulating() };
            bool actuallyMultipleScreens{ ScreenInfo::AreMultipleScreensPresent() };

            if (currentlyEmulating)
            {
                if (actuallyMultipleScreens)
                {
                    screenInfo.EmulateScreens(0, SplitKind::None);
                    screenInfo.Update(hWnd);
                }
                else
                {
                    if (screenInfo.GetSplitKind() == SplitKind::Vertical)
                    {
                        screenInfo.EmulateScreens(2, SplitKind::Horizontal);
                    }
                    else if (screenInfo.GetRectCount() == 2)
                    {
                        // Note there's actually a difference between 1 emulated screen
                        // and no emulation even on a single-screen Desktop device. If you
                        // are emulating 1 screen, the app will always draw the entire
                        // client area of the window, but if you are not emulating 1 screen
                        // then the app will not draw into any client area that is off-screen.
                        screenInfo.EmulateScreens(1, SplitKind::None);
                    }
                    else
                    {
                        screenInfo.EmulateScreens(0, SplitKind::None);
                        screenInfo.Update(hWnd);
                    }
                }
            }
            else
            {
                if (actuallyMultipleScreens)
                {
                    screenInfo.EmulateScreens(1, SplitKind::None);
                }
                else
                {
                    screenInfo.EmulateScreens(2, SplitKind::Vertical);
                }
            }

            SendMessageA(hWnd, WM_SIZE, 0, 0);

            break;
        }

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
