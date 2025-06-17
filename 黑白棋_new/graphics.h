#pragma once
#include <windows.h>

class ReversiGame;

void drawScreen(HDC hdc, HWND hwnd, const ReversiGame& game);
void drawHintCircle(HDC hdc, int x, int y, int CELL_SIZE);
