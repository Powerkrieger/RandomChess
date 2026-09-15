#ifndef RANDOMCHESS_CHESS_BOARD_H
#define RANDOMCHESS_CHESS_BOARD_H

#include <array>
#include <memory>

#include "Piece.h"
#include "Types.h"

namespace chess {

/*!
 * An 8x8 grid that owns its pieces. The board knows nothing about turns or rules; it only stores
 * pieces and relocates them. Rule enforcement lives in Game.
 */
class Board {
public:
    Board() = default;

    Board(const Board &other) { *this = other; }

    Board &operator=(const Board &other);

    Board(Board &&) = default;

    Board &operator=(Board &&) = default;

    /*! Removes every piece. */
    void clear();

    /*! Clears the board and places the standard starting position. */
    void setupStandard();

    const Piece *at(Square square) const { return squares_[index(square)].get(); }

    Piece *at(Square square) { return squares_[index(square)].get(); }

    bool isEmpty(Square square) const { return at(square) == nullptr; }

    /*! Places a piece on its own square, replacing whatever was there. */
    void place(std::unique_ptr<Piece> piece);

    /*! Takes the piece off the board and hands ownership to the caller. */
    std::unique_ptr<Piece> remove(Square square);

    /*!
     * Relocates the piece at @a from to @a to.
     * @return the piece that was captured on @a to, or nullptr if the square was empty
     */
    std::unique_ptr<Piece> move(Square from, Square to);

    /*! State needed to take a move back again. */
    struct Undo {
        Move move;
        bool hadMoved;
        int power;
        std::unique_ptr<Piece> captured;
    };

    /*! Like move(), but returns everything unmove() needs. Used by search. */
    Undo makeMove(const Move &move);

    /*! Reverts a makeMove(); must be called in reverse order of the makes. */
    void unmove(Undo undo);

    /*! Invokes @a fn(const Piece&) for every piece on the board. */
    template<typename Fn>
    void forEachPiece(Fn &&fn) const {
        for (const auto &piece: squares_) {
            if (piece) {
                fn(*piece);
            }
        }
    }

private:
    static int index(Square square) { return square.rank * kBoardSize + square.file; }

    std::array<std::unique_ptr<Piece>, kBoardSize * kBoardSize> squares_;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_BOARD_H
