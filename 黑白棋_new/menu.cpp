#include "menu.h"
#include <algorithm>
#define ID_PVP_BUTTON 100
#define ID_AI_BUTTON  101
HWND hPvpButton = nullptr;
HWND hAiButton = nullptr;

void drawMainMenu(HDC hdc, const RECT& rect) {
    // 填入背景
    HBRUSH bg = CreateSolidBrush(RGB(150, 200, 150));
    FillRect(hdc, &rect, bg);
    DeleteObject(bg);

    // 圓點樣式
    //HBRUSH decoBrush = CreateSolidBrush(RGB(190, 220, 190));  // 比背景稍微亮
    //HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, decoBrush);
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int dotRadius = 10;
    int spacingX = 50;
    int spacingY = 50;

    for (int y = 0; y < rect.bottom; y += spacingY) {
        for (int x = 0; x < rect.right; x += spacingX) {
            int offset = ((y / spacingY) % 2) * (spacingX / 2);  // 奇數行往右半格
            int cx = x + offset;
            int cy = y;
            // 根據 y 位置調整亮度（從上到下由亮變暗）
            int brightness = 200 + (cy * 40 / rect.bottom);  // 200~240
            brightness = std::min(brightness, 240);
            COLORREF color = RGB(brightness, 240, brightness);  // 偏綠色

            HBRUSH decoBrush = CreateSolidBrush(color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, decoBrush);
            Ellipse(hdc,
                cx - dotRadius,
                cy - dotRadius,
                cx + dotRadius,
                cy + dotRadius);
            SelectObject(hdc, oldBrush);
            DeleteObject(decoBrush);
        }
    }


    SelectObject(hdc, oldPen);
    // 主選單標題
    const TCHAR* title = TEXT("Reversi Game");

    HFONT font = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);

    SIZE textSize;
    GetTextExtentPoint32(hdc, title, lstrlen(title), &textSize);

    int titleX = (rect.right - textSize.cx) / 2;
    int titleY = rect.bottom / 6;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(30, 30, 30));
    TextOut(hdc, titleX, titleY, title, lstrlen(title));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void createMainMenuButtons(HWND hwnd, HINSTANCE hInstance) {
    hPvpButton = CreateWindow(TEXT("BUTTON"), TEXT("Player vs Player"),
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 200, 200, 40,
        hwnd, (HMENU)ID_PVP_BUTTON, hInstance, nullptr);

    hAiButton = CreateWindow(TEXT("BUTTON"), TEXT("Player vs AI"),
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 260, 200, 40,
        hwnd, (HMENU)ID_AI_BUTTON, hInstance, nullptr);
}

void toggleMenuButtons(HWND hwnd, bool show) {
    ShowWindow(GetDlgItem(hwnd, ID_PVP_BUTTON), show ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_AI_BUTTON), show ? SW_SHOW : SW_HIDE);
}

void resizeMainMenuButtons(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    int btnWidth = 200;
    int btnHeight = 40;
    int gap = 20;

    int x = (rect.right - btnWidth) / 2;
    int yStart = rect.bottom / 3;  // 讓按鈕組垂直靠上一點

    if (hPvpButton)
        MoveWindow(hPvpButton, x, yStart, btnWidth, btnHeight, TRUE);
    if (hAiButton)
        MoveWindow(hAiButton, x, yStart + btnHeight + gap, btnWidth, btnHeight, TRUE);
}

