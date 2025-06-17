#pragma once
#include <windows.h>
#include "graphics.h"


const int BOARD_SIZE = 8;

enum Cell { EMPTY, BLACK, WHITE };
enum GameMode {
    MENU,
    PVP,
    VS_AI  
};

struct AnimatedPiece {
    int row, col;
    int frame; // 目前幀數
    bool active;
};

class ReversiGame {
public:
    ReversiGame();
    void handleClick(int x, int y, HWND hwnd);
    Cell board[BOARD_SIZE][BOARD_SIZE];
    Cell currentPlayer;

    int count(Cell color) const;

    bool isValidMove(int row, int col, Cell player) const;
    bool canFlip(int r, int c, int dr, int dc, Cell player) const;
    void placePiece(int row, int col, Cell player);
    void flipDirection(int r, int c, int dr, int dc, Cell player);
    bool hasValidMove(Cell player) const;
    bool isGameOver() const;
    void reset();
    GameMode mode = MENU;
    AnimatedPiece anim;
    bool waitingForAI;
    void checkNextTurn(HWND hwnd);
};
