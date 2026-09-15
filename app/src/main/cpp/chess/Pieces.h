#ifndef RANDOMCHESS_CHESS_PIECES_H
#define RANDOMCHESS_CHESS_PIECES_H

#include "Piece.h"

namespace chess {

/*!
 * The six standard pieces. Each one is just a movement pattern on top of Piece; adding a new
 * (e.g. random/fairy) piece means adding another small class like these.
 */

class King : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<King>(*this); }
    PieceType type() const override { return PieceType::King; }
    std::vector<Move> moves(const Board &board) const override;
};

class Queen : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Queen>(*this); }
    PieceType type() const override { return PieceType::Queen; }
    std::vector<Move> moves(const Board &board) const override;
};

class Rook : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Rook>(*this); }
    PieceType type() const override { return PieceType::Rook; }
    std::vector<Move> moves(const Board &board) const override;
};

class Bishop : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Bishop>(*this); }
    PieceType type() const override { return PieceType::Bishop; }
    std::vector<Move> moves(const Board &board) const override;
};

class Knight : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Knight>(*this); }
    PieceType type() const override { return PieceType::Knight; }
    std::vector<Move> moves(const Board &board) const override;
};

class Pawn : public Piece {
public:
    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Pawn>(*this); }
    PieceType type() const override { return PieceType::Pawn; }
    std::vector<Move> moves(const Board &board) const override;
};

/*!
 * Shogun pieces. Both move by their current power() using the walk() rule; they differ only in
 * the range the power is rolled from and in how they're drawn.
 */

class Soldier : public Piece {
public:
    static constexpr int kMinPower = 1;
    static constexpr int kMaxPower = 4;

    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Soldier>(*this); }
    PieceType type() const override { return PieceType::Pawn; }
    std::vector<Move> moves(const Board &board) const override;
};

class Shogun : public Piece {
public:
    static constexpr int kMinPower = 1;
    static constexpr int kMaxPower = 2;

    using Piece::Piece;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Shogun>(*this); }
    PieceType type() const override { return PieceType::King; }
    std::vector<Move> moves(const Board &board) const override;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_PIECES_H
