#pragma once
#include <windows.h>
#include "reversi_game.h"

void drawMainMenu(HDC hdc, const RECT& rect);
void createMainMenuButtons(HWND hwnd, HINSTANCE hInstance);
void toggleMenuButtons(HWND hwnd, bool show);
void resizeMainMenuButtons(HWND hwnd);

