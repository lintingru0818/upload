#define NOMINMAX
#include <windows.h>
#include "reversi_game.h"
#include "graphics.h"
#include "menu.h"
#include "ai.h"


#define ID_RESET_BUTTON 1
#define ID_PVP_BUTTON 100
#define ID_AI_BUTTON  101
#define ID_MENU_BUTTON  200


ReversiGame game;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            int winWidth = LOWORD(lParam);
            // 可選：int winHeight = HIWORD(lParam);
        
            // 調整按鈕位置（右上角對齊）
            HWND hReset = GetDlgItem(hwnd, ID_RESET_BUTTON);
            HWND hBack  = GetDlgItem(hwnd, ID_MENU_BUTTON);
        
            if (hReset)
                MoveWindow(hReset, winWidth - 130, 10, 110, 30, TRUE);
            if (hBack)
                MoveWindow(hBack, winWidth - 130, 50, 110, 30, TRUE);
            // 加上主選單按鈕自適應
            if (game.mode == MENU) {
                resizeMainMenuButtons(hwnd);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            game.handleClick(x, y, hwnd);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
        
            // ⭐ 雙緩衝核心（在記憶體中繪製，再貼到畫面）
            RECT rect;
            GetClientRect(hwnd, &rect);
        
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
        
            // ✅ 繪製到記憶體 HDC
            drawScreen(memDC, hwnd, game);
        
            // ✅ 一次性貼上螢幕
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
        
            // 🧹 清理
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
        
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_RESET_BUTTON) {
                game.reset();  // 重設棋盤
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            switch (LOWORD(wParam)) {
                case ID_PVP_BUTTON:
                    game.reset();
                    game.mode = PVP;
                    toggleMenuButtons(hwnd, false);
                    ShowWindow(GetDlgItem(hwnd, ID_RESET_BUTTON), SW_SHOW);
                    ShowWindow(GetDlgItem(hwnd, ID_MENU_BUTTON), SW_SHOW);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
            
                case ID_AI_BUTTON:
                    game.reset();
                    game.mode = VS_AI;
                    toggleMenuButtons(hwnd, false);
                    ShowWindow(GetDlgItem(hwnd, ID_RESET_BUTTON), SW_SHOW);
                    ShowWindow(GetDlgItem(hwnd, ID_MENU_BUTTON), SW_SHOW);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                
                case ID_RESET_BUTTON:
                    game.reset();
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;

                case ID_MENU_BUTTON:
                    game.reset();
                    game.mode = MENU;
                    toggleMenuButtons(hwnd, true);  // 顯示主選單按鈕
                    ShowWindow(GetDlgItem(hwnd, ID_RESET_BUTTON), SW_HIDE);
                    ShowWindow(GetDlgItem(hwnd, ID_MENU_BUTTON), SW_HIDE);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                
            }
            break;
        case WM_TIMER:
            if (game.anim.active) {
                game.anim.frame++;
                if (game.anim.frame > 10) {
                    game.anim.active = false;
                    KillTimer(hwnd, 1);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    // ✅ 動畫結束 → 觸發 AI 落子
                    if (game.waitingForAI) {
                        game.waitingForAI = false;
                        aiMove(game, hwnd);
                        //return 0;  // ❗⚠️ 確保結束這一輪 WM_TIMER，防止重入
                    }
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            break;
        case WM_CTLCOLORBTN: {
                HDC hdcButton = (HDC)wParam;
                SetBkMode(hdcButton, TRANSPARENT);  // 文字背景透明
                return (INT_PTR)GetStockObject(NULL_BRUSH);  // 不要填滿背景
            }
            
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "ReversiWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Reversi Game", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT, 600, 650,
                               nullptr, nullptr, hInstance, nullptr);
    
    createMainMenuButtons(hwnd, hInstance);

    HWND hResetButton = CreateWindow(TEXT("BUTTON"), TEXT("restart"),
                                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                460, 10, 110, 30,
                                hwnd, (HMENU)ID_RESET_BUTTON, hInstance, nullptr);
    ShowWindow(hResetButton, SW_HIDE);  // 預設隱藏

    HWND hMenuButton = CreateWindow(TEXT("BUTTON"), TEXT("Back to Menu"),
                                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                460, 50, 110, 30,
                                hwnd, (HMENU)ID_MENU_BUTTON, hInstance, nullptr);
    ShowWindow(hMenuButton, SW_HIDE);  // 預設不顯示

                            
                       
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
