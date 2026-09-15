#ifndef RANDOMCHESS_CHESS_PIECE_H
#define RANDOMCHESS_CHESS_PIECE_H

#include <initializer_list>
#include <memory>
#include <vector>

#include "Types.h"

namespace chess {

class Board;

/*!
 * Base class for every piece. It owns the state shared by all pieces (colour, square, whether it
 * has moved yet) and the two fundamental movement primitives: sliding along a direction until
 * something blocks, and stepping to a fixed set of offsets. Both primitives already implement the
 * capture ("slaying") rule: an enemy piece can be landed on, a friendly piece cannot.
 *
 * A concrete piece only has to provide its type and describe its pattern in moves() using those
 * primitives (or something custom, as the pawn does).
 */
class Piece {
public:
    Piece(Color color, Square square)
            : color_(color), square_(square), hasMoved_(false), power_(0) {}

    virtual ~Piece() = default;

    virtual PieceType type() const = 0;

    /*! Deep copy; lets a Board be copied for search/simulation. */
    virtual std::unique_ptr<Piece> clone() const = 0;

    /*!
     * All squares this piece could move to on the given board, following its pattern and the
     * blocking/capture rules. These are pseudo-legal: they do not yet take check into account.
     */
    virtual std::vector<Move> moves(const Board &board) const = 0;

    Color color() const { return color_; }

    Square square() const { return square_; }

    bool hasMoved() const { return hasMoved_; }

    bool isEnemyOf(const Piece &other) const { return color_ != other.color_; }

    /*!
     * The piece's current move number for variants that use one (Shogun). 0 means "not used";
     * classic chess pieces ignore it entirely.
     */
    int power() const { return power_; }

    void setPower(int power) { power_ = power; }

    /*! Called by the board when the piece is relocated. */
    void moveTo(Square square) {
        square_ = square;
        hasMoved_ = true;
    }

    /*! Puts the piece back where it was, e.g. when a search undoes a move. */
    void restore(Square square, bool hasMoved, int power) {
        square_ = square;
        hasMoved_ = hasMoved;
        power_ = power;
    }

protected:
    /*!
     * Walks along each direction one square at a time, adding every empty square, and stopping at
     * the first occupied one (which is added as a capture if it's an enemy). Rook, bishop, queen.
     */
    void slide(const Board &board, std::initializer_list<Offset> directions,
               std::vector<Move> &out) const;

    /*!
     * Adds each offset as a single jump if the target is on the board and not a friendly piece.
     * Knight, king.
     */
    void step(const Board &board, std::initializer_list<Offset> offsets,
              std::vector<Move> &out) const;

    /*!
     * Adds every square reachable by a path of exactly @a length orthogonal steps that is either
     * straight or bends once by 90 degrees. Every square on the path except the last must be
     * empty, so nothing can be jumped over and nothing behind an obstruction can be captured.
     * This is the Shogun rule; the same destination reached by two paths is only added once.
     */
    void walk(const Board &board, int length, std::vector<Move> &out) const;

    /*!
     * Tries to add a move to @a to. Off-board and friendly squares are rejected, enemy squares are
     * added as captures, empty squares as plain moves.
     *
     * @return true if @a to was empty, i.e. a slide could continue past it
     */
    bool tryAdd(const Board &board, Square to, std::vector<Move> &out) const;

private:
    Color color_;
    Square square_;
    bool hasMoved_;
    int power_;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_PIECE_H
