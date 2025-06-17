#include "graphics.h"
#include "reversi_game.h"
#include "menu.h"
#include <algorithm>

void drawPieces(HDC hdc, const ReversiGame& game, int offsetX, int offsetY, int CELL_SIZE) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (game.board[r][c] == EMPTY) continue;
            
            // 檢查是否是動畫中的棋子
            bool isAnimating = game.anim.active && game.anim.row == r && game.anim.col == c;
            float scale = isAnimating ? game.anim.frame / 10.0f : 1.0f;
            /*
            int x1 = offsetX + c * CELL_SIZE + 6;
            int y1 = offsetY + r * CELL_SIZE + 6;
            int x2 = x1 + CELL_SIZE - 12;
            int y2 = y1 + CELL_SIZE - 12;
*/
            int baseX = offsetX + c * CELL_SIZE;
            int baseY = offsetY + r * CELL_SIZE;
            int size = static_cast<int>((CELL_SIZE - 12) * scale);
            int x1 = baseX + (CELL_SIZE - size) / 2;
            int y1 = baseY + (CELL_SIZE - size) / 2;
            int x2 = x1 + size;
            int y2 = y1 + size;
             // 🟠 邊框筆（白棋 → 淺灰、黑棋 → 深灰）
             COLORREF borderColor = (game.board[r][c] == BLACK) ? RGB(50, 50, 50) : RGB(180, 180, 180);
             HPEN pen = CreatePen(PS_SOLID, 2, borderColor);
             HGDIOBJ oldPen = SelectObject(hdc, pen);
 
             // 🟡 主體色（稍微偏一點，不是純黑白）
             COLORREF fillColor = (game.board[r][c] == BLACK) ? RGB(30, 30, 30) : RGB(245, 245, 245);
             HBRUSH brush = CreateSolidBrush(fillColor);
             HGDIOBJ oldBrush = SelectObject(hdc, brush);

            Ellipse(hdc, x1, y1, x2, y2);
            
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(pen);
            DeleteObject(brush);

        }
    }
}

void drawScreen(HDC hdc, HWND hwnd, const ReversiGame& game) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    if (game.mode == MENU) {
        drawMainMenu(hdc, rect);
        return;
    }

    // 動態計算 cell 大小與偏移
    int boardWidth = rect.right - rect.left - 100;
    int boardHeight = rect.bottom - 100;
    int CELL_SIZE = std::min(boardWidth, boardHeight) / BOARD_SIZE;
    int offsetX = (rect.right - CELL_SIZE * BOARD_SIZE) / 2;
    int offsetY = 90;

    // 背景
    HBRUSH bg = CreateSolidBrush(RGB(150, 200, 150));
    FillRect(hdc, &rect, bg);
    DeleteObject(bg);

    // 棋盤
    HBRUSH boardBrush = CreateSolidBrush(RGB(245, 245, 220));  // 淡米色
    HPEN noPen = (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ boardPen = SelectObject(hdc, noPen);
    HGDIOBJ boBrush = SelectObject(hdc, boardBrush);
    RoundRect(hdc,
        offsetX,
        offsetY,
        offsetX + CELL_SIZE * BOARD_SIZE,
        offsetY + CELL_SIZE * BOARD_SIZE,
        16, 16);
    SelectObject(hdc, boBrush);
    SelectObject(hdc, boardPen);
    DeleteObject(boardBrush);

    // 顯示合法提示圈
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (game.board[r][c] == EMPTY && game.isValidMove(r, c, game.currentPlayer)) {
                int x = offsetX + c * CELL_SIZE;
                int y = offsetY + r * CELL_SIZE;
                drawHintCircle(hdc, x, y, CELL_SIZE);
            }
        }
    }

    //畫棋子
    drawPieces(hdc, game, offsetX, offsetY, CELL_SIZE);

    // 格線
    HPEN thinPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN thickPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));  // 粗線（邊框）

    // 選擇細線筆畫內部格線
    SelectObject(hdc, thinPen);
    for (int i = 1; i < BOARD_SIZE; ++i) {
        // 垂直線
        MoveToEx(hdc, offsetX + i * CELL_SIZE, offsetY, nullptr);
        LineTo(hdc, offsetX + i * CELL_SIZE, offsetY + BOARD_SIZE * CELL_SIZE);
        // 水平線
        MoveToEx(hdc, offsetX, offsetY + i * CELL_SIZE, nullptr);
        LineTo(hdc, offsetX + BOARD_SIZE * CELL_SIZE, offsetY + i * CELL_SIZE);
    }
    // 畫邊框四條邊，用粗線
    SelectObject(hdc, thickPen);
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN oldPen = (HPEN)SelectObject(hdc, thickPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);

    RoundRect(hdc,
        offsetX,
        offsetY,
        offsetX + CELL_SIZE * BOARD_SIZE,
        offsetY + CELL_SIZE * BOARD_SIZE,
        16, 16);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    // 清理筆
    DeleteObject(thinPen);
    DeleteObject(thickPen);
    
    // 左上角顯示當前玩家
    TCHAR currentText[32];
    if (game.currentPlayer == BLACK)
        lstrcpy(currentText, TEXT("Current: Black"));
    else
        lstrcpy(currentText, TEXT("Current: White"));

    // 文字大小測量
    SIZE currentSize;
    GetTextExtentPoint32(hdc, currentText, lstrlen(currentText), &currentSize);

    // 顯示位置：左上角偏右一點
    int currentX = 20;
    int currentY = 20;

    // 文字輸出
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    TextOut(hdc, currentX, currentY, currentText, lstrlen(currentText));


    // 分數顯示
    // 🟢 分數底塊樣式（改成動態寬度）
    int margin = 10;
    int pieceSize = 30;
    int boxHeight = 60;
    int radius = 20;
    int spacing = 20;

    // 分數資料
    int black = game.count(BLACK);
    int white = game.count(WHITE);
    TCHAR blackText[8], whiteText[8];
    wsprintf(blackText, TEXT("%d"), black);
    wsprintf(whiteText, TEXT("%d"), white);

    // 字體
    HFONT font = CreateFont(38, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    // 測量文字寬度
    SIZE blackSize, whiteSize;
    GetTextExtentPoint32(hdc, blackText, lstrlen(blackText), &blackSize);
    GetTextExtentPoint32(hdc, whiteText, lstrlen(whiteText), &whiteSize);

    // 計算區塊寬度
    int blackBoxWidth = blackSize.cx + pieceSize + margin * 3;
    int whiteBoxWidth = whiteSize.cx + pieceSize + margin * 3;
    int totalWidth = blackBoxWidth + whiteBoxWidth + spacing;
    int startX = (rect.right - totalWidth) / 2;
    int baseY = 20;

    // 顏色
    COLORREF activeColor = RGB(80, 120, 80);
    COLORREF inactiveColor = RGB(100, 160, 100);

    HBRUSH blackBoxBrush = CreateSolidBrush(game.currentPlayer == BLACK ? activeColor : inactiveColor);
    HBRUSH whiteBoxBrush = CreateSolidBrush(game.currentPlayer == WHITE ? activeColor : inactiveColor);

    // 無邊框
    SelectObject(hdc, GetStockObject(NULL_PEN));

    // 畫黑棋底塊
    SelectObject(hdc, blackBoxBrush);
    RoundRect(hdc, startX, baseY, startX + blackBoxWidth, baseY + boxHeight, radius, radius);

    // 畫白棋底塊
    int whiteStartX = startX + blackBoxWidth + spacing;
    SelectObject(hdc, whiteBoxBrush);
    RoundRect(hdc, whiteStartX, baseY, whiteStartX + whiteBoxWidth, baseY + boxHeight, radius, radius);

    // 黑棋文字與圖示
    TextOut(hdc, startX + margin, baseY + (boxHeight - blackSize.cy) / 2, blackText, lstrlen(blackText));
    int blackPieceX = startX + blackBoxWidth - pieceSize - margin;
    int pieceY = baseY + (boxHeight - pieceSize) / 2;
    HBRUSH blackBrush = CreateSolidBrush(RGB(30, 30, 30));
    SelectObject(hdc, blackBrush);
    Ellipse(hdc, blackPieceX, pieceY, blackPieceX + pieceSize, pieceY + pieceSize);

    // 白棋文字與圖示
    int whitePieceX = whiteStartX + margin;
    HBRUSH whiteBrush = CreateSolidBrush(RGB(245, 245, 245));
    SelectObject(hdc, whiteBrush);
    Ellipse(hdc, whitePieceX, pieceY, whitePieceX + pieceSize, pieceY + pieceSize);
    TextOut(hdc, whitePieceX + pieceSize + margin, baseY + (boxHeight - whiteSize.cy) / 2, whiteText, lstrlen(whiteText));

    // 清理
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    DeleteObject(blackBrush);
    DeleteObject(whiteBrush);
    DeleteObject(blackBoxBrush);
    DeleteObject(whiteBoxBrush);

}

void drawHintCircle(HDC hdc, int x, int y, int CELL_SIZE) {
    int dotSize = CELL_SIZE / 6;  // 小圓點大小，比例縮放
    int cx = x + CELL_SIZE / 2;
    int cy = y + CELL_SIZE / 2;

    HBRUSH brush = CreateSolidBrush(RGB(200, 220, 255));  // 淺藍色
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

    Ellipse(hdc,
        cx - dotSize / 2,
        cy - dotSize / 2,
        cx + dotSize / 2,
        cy + dotSize / 2);

    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}
