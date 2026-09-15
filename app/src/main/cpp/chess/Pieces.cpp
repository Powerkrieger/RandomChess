#include "Pieces.h"

#include "Board.h"

namespace chess {

namespace {
constexpr std::initializer_list<Offset> kOrthogonal = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
constexpr std::initializer_list<Offset> kDiagonal = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
constexpr std::initializer_list<Offset> kAllDirections = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
constexpr std::initializer_list<Offset> kKnightJumps = {
        {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
} // namespace

std::vector<Move> King::moves(const Board &board) const {
    std::vector<Move> out;
    step(board, kAllDirections, out);
    // Castling is intentionally not handled yet.
    return out;
}

std::vector<Move> Queen::moves(const Board &board) const {
    std::vector<Move> out;
    slide(board, kAllDirections, out);
    return out;
}

std::vector<Move> Rook::moves(const Board &board) const {
    std::vector<Move> out;
    slide(board, kOrthogonal, out);
    return out;
}

std::vector<Move> Bishop::moves(const Board &board) const {
    std::vector<Move> out;
    slide(board, kDiagonal, out);
    return out;
}

std::vector<Move> Knight::moves(const Board &board) const {
    std::vector<Move> out;
    step(board, kKnightJumps, out);
    return out;
}

std::vector<Move> Pawn::moves(const Board &board) const {
    std::vector<Move> out;

    // Pawns are the only piece whose pattern depends on colour and that moves differently from
    // how it captures, so it doesn't use slide()/step().
    const int forward = color() == Color::White ? 1 : -1;
    const Square from = square();

    const Square oneAhead = from + Offset{0, forward};
    if (oneAhead.isValid() && board.isEmpty(oneAhead)) {
        out.push_back(Move{from, oneAhead, false});

        const Square twoAhead = from + Offset{0, 2 * forward};
        if (!hasMoved() && twoAhead.isValid() && board.isEmpty(twoAhead)) {
            out.push_back(Move{from, twoAhead, false});
        }
    }

    for (int side: {-1, 1}) {
        const Square target = from + Offset{side, forward};
        if (!target.isValid()) {
            continue;
        }
        const Piece *occupant = board.at(target);
        if (occupant != nullptr && isEnemyOf(*occupant)) {
            out.push_back(Move{from, target, true});
        }
    }

    // En passant is intentionally not handled yet.
    return out;
}

std::vector<Move> Soldier::moves(const Board &board) const {
    std::vector<Move> out;
    walk(board, power(), out);
    return out;
}

std::vector<Move> Shogun::moves(const Board &board) const {
    std::vector<Move> out;
    walk(board, power(), out);
    return out;
}

} // namespace chess
