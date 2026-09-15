#ifndef RANDOMCHESS_CHESS_SHOGUNAI_H
#define RANDOMCHESS_CHESS_SHOGUNAI_H

#include <optional>
#include <random>

#include "Board.h"
#include "Types.h"

namespace chess {

/*!
 * A computer opponent for the Shogun rules.
 *
 * It searches with expectiminimax: after every move the mover's number is re-rolled, so each
 * move is followed by a chance node that averages over the possible new numbers. Positions are
 * scored by material, mobility and win/loss. Depth counts plies (one player's move each), so
 * depth 2 means "my move, your reply".
 */
class ShogunAi {
public:
    /*! Evaluation weights; public so they can be tuned and compared in tests. */
    struct Weights {
        float piece = 100.f;        // per own piece (minus per enemy piece)
        float mobility = 2.f;       // per legal move
        float threat = 12.f;        // per capture available against an ordinary piece
        float kingThreat = 250.f;   // per capture available against the king
        float toMoveFactor = 3.f;   // threats of the side to move count this much more
        float lastPieces = 150.f;   // penalty for being at the minimum piece count
    };

    explicit ShogunAi(int depth = 2);

    void setWeights(const Weights &weights) { weights_ = weights; }

    const Weights &weights() const { return weights_; }

    /*! Best move for @a side on @a board, or nullopt if it has no legal move. */
    std::optional<Move> chooseMove(const Board &board, Color side);

    int depth() const { return depth_; }

    void setDepth(int depth) { depth_ = depth; }

private:
    /*! Score of @a board from @a side's point of view with @a toMove to play. */
    float search(Board &board, Color toMove, Color side, int depth, float alpha, float beta);

    /*!
     * Expected score after @a move has been made: the mover's number is unknown, so this
     * averages search() over every possible value.
     */
    float expectAfterMove(Board &board, const Move &move, Color mover, Color side, int depth,
                          float alpha, float beta);

    /*! Static score of @a board for @a side, with @a toMove to play next. */
    float evaluate(const Board &board, Color side, Color toMove) const;

    int depth_;
    Weights weights_;
    std::mt19937 rng_;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_SHOGUNAI_H
