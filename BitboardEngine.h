#ifndef BITBOARD_ENGINE_H
#define BITBOARD_ENGINE_H

#include <cstdint>
#include <vector>
#include <algorithm>

using namespace std;

class Bitboard {
private:
    struct Move {
        int column;
        int score;

        Move(int c, int s) : column(c), score(s) {}
    };

    uint64_t bb[2] = {0, 0}; 
    int height[7] = {0, 0, 0, 0, 0, 0, 0};
    int moves;

    const int INF = 1000000000;
    const int preferredOrder[7] = {3, 4, 2, 5, 1, 6, 0};

    int evaluate() {
        if (isWin(1)) {
            return INF - moves;
        }
        if (isWin(0)) {
            return -INF + moves;
        }
        if (isDraw()) {
            return 0;
        }

        int score = 0;
        uint64_t centerCol = 0x7E00000;
        
        score += __builtin_popcountll(bb[1] & centerCol) * 3;
        score -= __builtin_popcountll(bb[0] & centerCol) * 3;

        return score;
    }

    int minimax(int depth, int alpha, int beta, bool isMax) {
        if (isWin(1)) {
            return INF - moves;
        }
        if (isWin(0)) {
            return -INF + moves;
        }
        if (isDraw()) {
            return 0;
        }
        if (depth == 0) {
            return evaluate();
        }

        if (isMax) {
            int maxEval = -INF;
            for (int col : preferredOrder) {
                if (isValidMove(col)) {
                    makeMove(col, 1);
                    int eval = minimax(depth - 1, alpha, beta, false);
                    undoMove(col, 1);
                    
                    maxEval = max(maxEval, eval);
                    alpha = max(alpha, eval);
                    if (beta <= alpha) {
                        break;
                    }
                }
            }
            return maxEval;
        }
        else {
            int minEval = INF;
            for (int col : preferredOrder) {
                if (isValidMove(col)) {
                    makeMove(col, 0);
                    int eval = minimax(depth - 1, alpha, beta, true);
                    undoMove(col, 0);
                    
                    minEval = min(minEval, eval);
                    beta = min(beta, eval);

                    if (beta <= alpha) {
                        break;
                    }
                }
            }
            return minEval;
        }
    }

public:
    Bitboard() {
        reset();
    }

    void reset() {
        bb[0] = bb[1] = moves = 0;
        for (int i = 0; i < 7; ++i) {
            height[i] = i * 7;
        }
    }

    bool isValidMove(int col) {
        return height[col] - col * 7 < 6;
    }

    void makeMove(int col, int player) {
        uint64_t move = 1ULL << height[col]++;
        bb[player] |= move;
        ++moves;
    }

    void undoMove(int col, int player) {
        --height[col];
        uint64_t move = 1ULL << height[col];
        bb[player] &= ~move;
        --moves;
    }

    bool isWin(int player) {
        uint64_t m;
        uint64_t b = bb[player];

        // (-)
        m = b & (b >> 7);
        if (m & (m >> 14)) {
            return true;
        }
        
        // (|)
        m = b & (b >> 1);
        if (m & (m >> 2)) {
            return true;
        }
        
        // (\)
        m = b & (b >> 6);
        if (m & (m >> 12)) {
            return true;
        }
        
        // (/)
        m = b & (b >> 8);
        if (m & (m >> 16)) {
            return true;
        }
        
        return false;
    }

    bool isDraw() {
        return moves == 42;
    } 

    vector<Move> getAllMoves(int depth) {
        vector<Move> moves;

        for (int col : preferredOrder) {
            if (isValidMove(col)) {
                makeMove(col, 1);
                int score = minimax(depth - 1, -INF, INF, false); 
                undoMove(col, 1);
                
                moves.push_back(Move(col, score));
            }
        }

        sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
            return a.score > b.score;
        });

        return moves;
    }

    vector<int> getBestMoves(int depth) {
        vector<Move> moves = getAllMoves(depth);
        vector<int> bestMoves;

        int bestScore = moves[0].score;

        for (Move move : moves) {
            if (move.score != bestScore) {
                break;
            }
            bestMoves.push_back(move.column);
        }

        return bestMoves;
    }

    vector<int> getGoodMoves(int depth, int delta) {
        vector<Move> moves = getAllMoves(depth);

        int bestScore = moves[0].score;

        vector<int> bestMoves;
        vector<int> goodMoves;

        for (Move move : moves) {
            if (move.score == bestScore) {
                bestMoves.push_back(move.column);
            }
            else if (bestScore - delta <= move.score && move.score < bestScore) {
                goodMoves.push_back(move.column);
            }
        }

        if (!goodMoves.empty()) {
            return goodMoves;
        }

        return bestMoves;
    }

    vector<int> getBadMoves(int depth, int delta) {
        vector<Move> moves = getAllMoves(depth);

        int bestScore = moves[0].score;

        vector<int> bestMoves;
        vector<int> goodMoves;
        vector<int> badMoves;

        for (Move move : moves) {
            if (move.score == bestScore) {
                bestMoves.push_back(move.column);
            }
            else if (bestScore - delta <= move.score && move.score < bestScore) {
                goodMoves.push_back(move.column);
            }
            else {
                badMoves.push_back(move.column);
            }
        }

        if (!badMoves.empty()) {
            return badMoves;
        }

        if (!goodMoves.empty()) {
            return goodMoves;
        }

        return bestMoves;
    }

    vector<int> getRandomMoves() {
        vector<int> moves;

        for (int col = 0; col < 7; ++col) {
            if (isValidMove(col)) {
                moves.push_back(col);
            }
        }

        return moves;
    }
};

#endif 