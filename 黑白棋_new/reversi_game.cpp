#include "reversi_game.h"
#include "ai.h"
#include <algorithm>

ReversiGame::ReversiGame() {
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            board[r][c] = EMPTY;

    board[3][3] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;
    board[4][4] = WHITE;

    currentPlayer = BLACK;
}


void ReversiGame::handleClick(int x, int y, HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    int boardWidth = rect.right - rect.left - 100;
    int boardHeight = rect.bottom - 100;
    int CELL_SIZE = std::min(boardWidth, boardHeight) / BOARD_SIZE;
    int offsetX = (rect.right - CELL_SIZE * BOARD_SIZE) / 2;
    int offsetY = 90;

    int col = (x - offsetX) / CELL_SIZE;
    int row = (y - offsetY) / CELL_SIZE;

    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        if (isValidMove(row, col, currentPlayer)) {
            placePiece(row, col, currentPlayer);
            anim = { row, col, 0, true }; // ✅ 動畫
            SetTimer(hwnd, 1, 20, NULL);
            currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
        }
    }

    // 🧠 檢查是否跳過某方
    while (!hasValidMove(currentPlayer)) {
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;

        // 若雙方都不能下就跳出
        if (!hasValidMove(currentPlayer)) break;
    }

    // 若是 AI 輪到，設定為等待動畫後觸發
    if (mode == VS_AI && currentPlayer == WHITE) {
        waitingForAI = true;
        if (!anim.active) {
            aiMove(*this, hwnd);
        }
    }

    // 檢查是否遊戲結束
    if (isGameOver()) {
        int black = count(BLACK);
        int white = count(WHITE);
        TCHAR result[100];

        if (black > white)
            wsprintf(result, TEXT("Game Over!\nBlack wins!\nBlack: %d   White: %d"), black, white);
        else if (white > black)
            wsprintf(result, TEXT("Game Over!\nWhite wins!\nBlack: %d   White: %d"), black, white);
        else
            wsprintf(result, TEXT("Game Over!\nIt's a tie!\nBlack: %d   White: %d"), black, white);

        MessageBox(hwnd, result, TEXT("Game Over"), MB_OK | MB_ICONINFORMATION);
    }

    InvalidateRect(hwnd, nullptr, TRUE);
}



bool ReversiGame::isValidMove(int row, int col, Cell player) const {
    if (board[row][col] != EMPTY) return false;
    for (int dr = -1; dr <= 1; ++dr)
        for (int dc = -1; dc <= 1; ++dc)
            if (dr != 0 || dc != 0)
                if (canFlip(row, col, dr, dc, player))
                    return true;
    return false;
}

bool ReversiGame::canFlip(int r, int c, int dr, int dc, Cell player) const {
    int i = r + dr, j = c + dc;
    bool foundOpponent = false;
    Cell opponent = (player == BLACK) ? WHITE : BLACK;

    while (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE) {
        if (board[i][j] == opponent) {
            foundOpponent = true;
        } else if (board[i][j] == player) {
            return foundOpponent;
        } else {
            break;
        }
        i += dr;
        j += dc;
    }
    return false;
}

void ReversiGame::placePiece(int row, int col, Cell player) {
    board[row][col] = player;
    for (int dr = -1; dr <= 1; ++dr)
        for (int dc = -1; dc <= 1; ++dc)
            if (dr != 0 || dc != 0)
                if (canFlip(row, col, dr, dc, player))
                    flipDirection(row, col, dr, dc, player);
    // ✨ 開始動畫
    anim = { row, col, 0, true };
}

void ReversiGame::flipDirection(int r, int c, int dr, int dc, Cell player) {
    int i = r + dr, j = c + dc;
    Cell opponent = (player == BLACK) ? WHITE : BLACK;

    while (board[i][j] == opponent) {
        board[i][j] = player;
        i += dr;
        j += dc;
    }
}

int ReversiGame::count(Cell color) const {
    int cnt = 0;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (board[r][c] == color)
                ++cnt;
    return cnt;
}

bool ReversiGame::hasValidMove(Cell player) const {
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (isValidMove(r, c, player))
                return true;
    return false;
}

bool ReversiGame::isGameOver() const {
    return !hasValidMove(BLACK) && !hasValidMove(WHITE);
}
void ReversiGame::reset() {
    anim.active = false;
    waitingForAI = false;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            board[r][c] = EMPTY;

    board[3][3] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;
    board[4][4] = WHITE;

    currentPlayer = BLACK;
}

void ReversiGame::checkNextTurn(HWND hwnd) {
    // 若遊戲已結束，結算
    if (isGameOver()) {
        int black = count(BLACK);
        int white = count(WHITE);
        TCHAR result[100];
        if (black > white)
            wsprintf(result, TEXT("Game Over!\nBlack wins!\nBlack: %d   White: %d"), black, white);
        else if (white > black)
            wsprintf(result, TEXT("Game Over!\nWhite wins!\nBlack: %d   White: %d"), black, white);
        else
            wsprintf(result, TEXT("Game Over!\nIt's a tie!\nBlack: %d   White: %d"), black, white);
        MessageBox(hwnd, result, TEXT("Game Over"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    // 如果換手後沒有合法步驟，繼續切換直到有合法步驟或雙方皆無
    for (int i = 0; i < 2; ++i) {
        if (hasValidMove(currentPlayer)) {
            if (mode == VS_AI && currentPlayer == WHITE) {
                waitingForAI = true;  // AI 等動畫結束後執行
                if (!anim.active) aiMove(*this, hwnd);  // 若沒動畫則直接執行
            }
            break;
        }
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    }
}
