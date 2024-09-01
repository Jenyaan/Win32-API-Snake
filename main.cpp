#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "logic.h"
#include "resource.h"

LRESULT CALLBACK Wnd_Setting(HWND, UINT, WPARAM, LPARAM);
void setting_menu(HWND hwnd);

int WINAPI _tWinMain(HINSTANCE hinst, HINSTANCE, LPTSTR, int) {
    if (DialogBox(hinst, MAKEINTRESOURCE(IDD_START_DIALOG), NULL, Wnd_Setting) == ID_BUTTON_PLAY) {
        return create_window(hinst, SN_APP);
    }
    return 0;
}

LRESULT CALLBACK Wnd_Setting(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        setting_menu(hwnd);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUTTON_HISTORY:
            ShellExecute(NULL, "open", "game_history.txt", NULL, NULL, SW_SHOWNORMAL);
            break;
        case ID_BUTTON_PLAY:
        {
            int selectedColor = SendDlgItemMessage(hwnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
            switch (selectedColor) {
            case 1:
                snakeColor = RGB(0, 255, 0); // Green
                break;
            case 0:
                snakeColor = RGB(0, 0, 255); // Blue
                break;
            case 2:
                snakeColor = RGB(255, 0, 0); // Red
                break;
            }
            int selectedMap = SendDlgItemMessage(hwnd, IDC_COMBO2, CB_GETCURSEL, 0, 0);
            switch (selectedMap) {
            case 0:
                snakeMap = 0;
                break;
            case 1:
                snakeMap = 1;
                break;
            case 2:
                snakeMap = 2;
                break;
            }
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        }
    }
    return FALSE;
}

//обработчик
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HBITMAP hbm = NULL;
    static HBRUSH wall, back = NULL;
    static HPEN    pen = NULL;
    static HBITMAP food = NULL;
    static HDC mdc = NULL;
    static int dir = VK_UP;
    HDC hdc;

    switch (msg) {
    case WM_CREATE:
        hdc = GetDC(hwnd);
        mdc = CreateCompatibleDC(hdc);
        hbm = CreateCompatibleBitmap(hdc, SN_WIDTH, SN_HEIGHT);
        SelectObject(mdc, hbm);
        ReleaseDC(hwnd, hdc);
        food = (HBITMAP)LoadImage(NULL, "img/apple1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        back = CreateSolidBrush(snakeColor);
        wall = CreateSolidBrush(RGB(0x00, 0x00, 0xFF)); 
        pen = CreatePen(PS_SOLID, 1, RGB(0, 0xAA, 0));
        start(dir);
        MessageBox(hwnd, SN_HELP, SN_APP, MB_OK | MB_ICONQUESTION);
        SetTimer(hwnd, SN_TIMER, 70, NULL);
        break;
    case WM_ERASEBKGND:
        PatBlt(mdc, 0, 0, SN_WIDTH, SN_HEIGHT, WHITENESS);
        draw(mdc, dir, back, pen, food, wall);
        if (g_paused) {
            draw_pause(hwnd);
        }
        else {
            BitBlt((HDC)wParam, 0, 0, SN_WIDTH, SN_HEIGHT, mdc, 0, 0, SRCCOPY);
        }
        return 1;
    case WM_KEYDOWN:
        switch (LOWORD(wParam)) {
        case VK_LEFT:
            if (dir != VK_RIGHT && !g_paused) dir = VK_LEFT;
            break;
        case VK_RIGHT:
            if (dir != VK_LEFT && !g_paused) dir = VK_RIGHT;
            break;
        case VK_UP:
            if (dir != VK_DOWN && !g_paused) dir = VK_UP;
            break;
        case VK_DOWN:
            if (dir != VK_UP && !g_paused) dir = VK_DOWN;
            break;
        case VK_RETURN:
            if (g_over)
                start(dir);
            break;
        case VK_ESCAPE:
            if (g_paused) {
                SetTimer(hwnd, SN_TIMER, 70, NULL); 
            }
            else {
                KillTimer(hwnd, SN_TIMER); 
            }
            g_paused = !g_paused;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_GAME_HISTORY:
            ShellExecute(NULL, "open", "game_history.txt", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        break;
    case WM_TIMER:
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    case WM_DESTROY:
        DeleteDC(mdc);
        DeleteObject(hbm);
        DeleteObject(back);
        DeleteObject(pen);
        DeleteObject(food);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int create_window(HINSTANCE hinst, LPCTSTR cap) {
    WNDCLASSEX cls = { 0 };
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = (WNDPROC)wnd_proc;
    cls.hInstance = hinst;
    cls.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON1)); 
    cls.hCursor = LoadCursor(NULL, IDC_ARROW);
    cls.lpszClassName = SN_CLASS;
    if (!RegisterClassEx(&cls))
        return 1;

    DWORD sty = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
    RECT  rc = { 0, 0, SN_WIDTH, SN_HEIGHT };
    AdjustWindowRectEx(&rc, sty, FALSE, 0);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    HWND hwnd = CreateWindowEx(0, SN_CLASS, cap, sty, (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - height) / 2, width, height, NULL, NULL, hinst, NULL);
    if (hwnd == NULL) {
        UnregisterClass(SN_CLASS, hinst);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterClass(SN_CLASS, hinst);
    return 0;
}

void setting_menu(HWND hwnd) {
    SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Green");
    SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Blue");
    SendDlgItemMessage(hwnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Red");

    SendDlgItemMessage(hwnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Easy");
    SendDlgItemMessage(hwnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Medium");
    SendDlgItemMessage(hwnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Hard");


    HWND hCombo2 = GetDlgItem(hwnd, IDC_COMBO1);
    HWND hCombo3 = GetDlgItem(hwnd, IDC_COMBO2);

    SetWindowPos(hCombo2, NULL, 0, 0, 100, 200, SWP_NOMOVE | SWP_NOZORDER);
    SetWindowPos(hCombo3, NULL, 0, 0, 100, 200, SWP_NOMOVE | SWP_NOZORDER);

    SendDlgItemMessage(hwnd, IDC_COMBO1, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(hwnd, IDC_COMBO2, CB_SETCURSEL, 0, 0);
}