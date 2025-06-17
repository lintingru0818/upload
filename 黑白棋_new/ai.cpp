#include "ai.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits.h>  // for INT_MIN

int evaluateBoard(const ReversiGame& game) {
    const int weight[BOARD_SIZE][BOARD_SIZE] = {
        {100, -20, 10, 5, 5, 10, -20, 100},
        {-20, -50, -2, -2, -2, -2, -50, -20},
        {10,  -2,  -1, -1, -1, -1,  -2,  10},
        {5,   -2,  -1, -1, -1, -1,  -2,   5},
        {5,   -2,  -1, -1, -1, -1,  -2,   5},
        {10,  -2,  -1, -1, -1, -1,  -2,  10},
        {-20, -50, -2, -2, -2, -2, -50, -20},
        {100, -20, 10, 5, 5, 10, -20, 100}
    };

    int score = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (game.board[r][c] == WHITE)
                score += weight[r][c];
            else if (game.board[r][c] == BLACK)
                score -= weight[r][c];
        }
    }
    return score;
}

int minimaxAB(ReversiGame game, int depth, int alpha, int beta, bool isMaximizing) {
    if (depth == 0 || game.isGameOver())
        return evaluateBoard(game);

    std::vector<std::pair<int, int>> moves;
    Cell player = isMaximizing ? WHITE : BLACK;

    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (game.isValidMove(r, c, player))
                moves.emplace_back(r, c);

    if (moves.empty()) {
        return minimaxAB(game, depth - 1, alpha, beta, !isMaximizing);
    }

    if (isMaximizing) {
        int maxEval = INT_MIN;
        for (auto [r, c] : moves) {
            ReversiGame next = game;
            next.placePiece(r, c, player);
            next.currentPlayer = BLACK;
            int eval = minimaxAB(next, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break;  // ❌ 剪枝
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (auto [r, c] : moves) {
            ReversiGame next = game;
            next.placePiece(r, c, player);
            next.currentPlayer = WHITE;
            int eval = minimaxAB(next, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break;  // ❌ 剪枝
        }
        return minEval;
    }
}


void aiMove(ReversiGame& game, HWND hwnd) {
    std::vector<std::pair<int, int>> moves;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (game.isValidMove(r, c, WHITE))
                moves.emplace_back(r, c);

    if (moves.empty()) return;

    int bestScore = INT_MIN;
    std::pair<int, int> bestMove = moves[0];

    for (auto [r, c] : moves) {
        ReversiGame next = game;
        next.placePiece(r, c, WHITE);
        next.currentPlayer = BLACK;

        int score = minimaxAB(next, 5, INT_MIN, INT_MAX, false);  // 使用 alpha-beta
        if (score > bestScore) {
            bestScore = score;
            bestMove = {r, c};
        }
    }

    game.placePiece(bestMove.first, bestMove.second, WHITE);
    game.anim = { bestMove.first, bestMove.second, 0, true };
    SetTimer(hwnd, 1, 30, NULL);
    game.currentPlayer = BLACK;
    InvalidateRect(hwnd, nullptr, TRUE);
}
