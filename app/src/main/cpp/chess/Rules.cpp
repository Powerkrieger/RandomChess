#include "Rules.h"

#include "Pieces.h"

namespace chess {

// ---------------------------------------------------------------------------------------------
// Chess
// ---------------------------------------------------------------------------------------------

void ChessRules::setup(Board &board) {
    board.setupStandard();
}

void ChessRules::afterMove(Board &board, const Move &move, Piece &mover, const Piece *) {
    // A pawn reaching the far rank promotes; always to a queen for now.
    const int lastRank = mover.color() == Color::White ? kBoardSize - 1 : 0;
    if (mover.type() == PieceType::Pawn && move.to.rank == lastRank) {
        board.place(std::make_unique<Queen>(mover.color(), move.to));
    }
}

std::optional<Color> ChessRules::winner(const Board &) const {
    // Checkmate detection is not implemented yet.
    return std::nullopt;
}

// ---------------------------------------------------------------------------------------------
// Shogun
// ---------------------------------------------------------------------------------------------

ShogunRules::ShogunRules() : rng_(std::random_device{}()) {}

void ShogunRules::setup(Board &board) {
    board.clear();

    for (Color color: {Color::White, Color::Black}) {
        const int homeRank = color == Color::White ? 0 : kBoardSize - 1;

        for (int file = 0; file < kBoardSize; ++file) {
            std::unique_ptr<Piece> piece;
            if (file == 4) {
                piece = std::make_unique<Shogun>(color, Square{file, homeRank});
            } else {
                piece = std::make_unique<Soldier>(color, Square{file, homeRank});
            }
            roll(*piece);
            board.place(std::move(piece));
        }
    }
}

void ShogunRules::afterMove(Board &, const Move &, Piece &mover, const Piece *) {
    roll(mover);
}

std::optional<Color> ShogunRules::winner(const Board &board) const {
    return outcome(board);
}

std::optional<Color> ShogunRules::outcome(const Board &board) {
    int pieces[2] = {0, 0};
    bool hasKing[2] = {false, false};

    board.forEachPiece([&](const Piece &piece) {
        const int side = static_cast<int>(piece.color());
        ++pieces[side];
        if (piece.type() == PieceType::King) {
            hasKing[side] = true;
        }
    });

    for (Color color: {Color::White, Color::Black}) {
        const int side = static_cast<int>(color);
        if (!hasKing[side] || pieces[side] < kMinPiecesToPlay) {
            return opposite(color);
        }
    }
    return std::nullopt;
}

std::pair<int, int> ShogunRules::powerRange(const Piece &piece) {
    if (piece.type() == PieceType::King) {
        return {Shogun::kMinPower, Shogun::kMaxPower};
    }
    return {Soldier::kMinPower, Soldier::kMaxPower};
}

void ShogunRules::roll(Piece &piece) {
    const auto [minPower, maxPower] = powerRange(piece);
    std::uniform_int_distribution<int> dist(minPower, maxPower);
    piece.setPower(dist(rng_));
}

} // namespace chess
