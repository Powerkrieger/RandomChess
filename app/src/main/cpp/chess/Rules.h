#ifndef RANDOMCHESS_CHESS_RULES_H
#define RANDOMCHESS_CHESS_RULES_H

#include <optional>
#include <random>
#include <utility>

#include "Board.h"
#include "Types.h"

namespace chess {

/*!
 * Everything that differs between game variants: the starting position, what happens right
 * after a move, and when the game is over. Game drives the turn/selection flow and asks the
 * rules at these three points.
 */
class Rules {
public:
    virtual ~Rules() = default;

    /*! Places the starting position on an empty board. */
    virtual void setup(Board &board) = 0;

    /*!
     * Called after @a mover has landed on move.to and @a captured (may be null) has been taken
     * off the board.
     */
    virtual void afterMove(Board &board, const Move &move, Piece &mover,
                           const Piece *captured) = 0;

    /*! The winning side if the game is over, nullopt while play continues. */
    virtual std::optional<Color> winner(const Board &board) const = 0;
};

/*! Standard chess (without check, castling and en passant so far). */
class ChessRules : public Rules {
public:
    void setup(Board &board) override;
    void afterMove(Board &board, const Move &move, Piece &mover, const Piece *captured) override;
    std::optional<Color> winner(const Board &board) const override;
};

/*!
 * Shogun (Ravensburger, 1979): a king and seven soldiers per side. Every piece carries a number
 * that says exactly how far it moves (see Piece::walk); the number is re-rolled whenever the
 * piece moves. You win by capturing the enemy king or reducing the enemy to fewer than three
 * pieces.
 */
class ShogunRules : public Rules {
public:
    static constexpr int kSoldiersPerSide = 7;
    static constexpr int kMinPiecesToPlay = 3;

    ShogunRules();

    /*! Deterministic games, e.g. for tests. */
    explicit ShogunRules(unsigned seed) : rng_(seed) {}

    void setup(Board &board) override;
    void afterMove(Board &board, const Move &move, Piece &mover, const Piece *captured) override;
    std::optional<Color> winner(const Board &board) const override;

    /*! The winner() logic as a pure function, so the AI can use it on search positions. */
    static std::optional<Color> outcome(const Board &board);

    /*! Inclusive power range a piece re-rolls from. */
    static std::pair<int, int> powerRange(const Piece &piece);

private:
    /*! Gives @a piece a fresh random power within its own range. */
    void roll(Piece &piece);

    std::mt19937 rng_;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_RULES_H
