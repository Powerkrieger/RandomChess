#include "Board.h"

#include "Pieces.h"

namespace chess {

Board &Board::operator=(const Board &other) {
    if (this != &other) {
        for (size_t i = 0; i < squares_.size(); ++i) {
            squares_[i] = other.squares_[i] ? other.squares_[i]->clone() : nullptr;
        }
    }
    return *this;
}

void Board::clear() {
    for (auto &square: squares_) {
        square.reset();
    }
}

void Board::setupStandard() {
    clear();

    for (Color color: {Color::White, Color::Black}) {
        const int homeRank = color == Color::White ? 0 : kBoardSize - 1;
        const int pawnRank = color == Color::White ? 1 : kBoardSize - 2;

        place(std::make_unique<Rook>(color, Square{0, homeRank}));
        place(std::make_unique<Knight>(color, Square{1, homeRank}));
        place(std::make_unique<Bishop>(color, Square{2, homeRank}));
        place(std::make_unique<Queen>(color, Square{3, homeRank}));
        place(std::make_unique<King>(color, Square{4, homeRank}));
        place(std::make_unique<Bishop>(color, Square{5, homeRank}));
        place(std::make_unique<Knight>(color, Square{6, homeRank}));
        place(std::make_unique<Rook>(color, Square{7, homeRank}));

        for (int file = 0; file < kBoardSize; ++file) {
            place(std::make_unique<Pawn>(color, Square{file, pawnRank}));
        }
    }
}

void Board::place(std::unique_ptr<Piece> piece) {
    squares_[index(piece->square())] = std::move(piece);
}

std::unique_ptr<Piece> Board::remove(Square square) {
    return std::move(squares_[index(square)]);
}

std::unique_ptr<Piece> Board::move(Square from, Square to) {
    std::unique_ptr<Piece> mover = remove(from);
    std::unique_ptr<Piece> captured = remove(to);
    if (mover) {
        mover->moveTo(to);
        place(std::move(mover));
    }
    return captured;
}

} // namespace chess

namespace chess {

Board::Undo Board::makeMove(const Move &move) {
    const Piece *mover = at(move.from);
    Undo undo{move, mover->hasMoved(), mover->power(), nullptr};
    undo.captured = this->move(move.from, move.to);
    return undo;
}

void Board::unmove(Undo undo) {
    std::unique_ptr<Piece> mover = remove(undo.move.to);
    mover->restore(undo.move.from, undo.hadMoved, undo.power);
    place(std::move(mover));
    if (undo.captured) {
        place(std::move(undo.captured));
    }
}

} // namespace chess
