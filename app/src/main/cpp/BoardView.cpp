#include "BoardView.h"

#include <cmath>

using chess::kBoardSize;
using chess::Square;

namespace {

constexpr Vector4 kLightSquare{0.94f, 0.85f, 0.71f, 1.f};
constexpr Vector4 kDarkSquare{0.71f, 0.53f, 0.39f, 1.f};
constexpr Vector4 kSelected{0.95f, 0.85f, 0.20f, 0.60f};
constexpr Vector4 kLastMove{0.60f, 0.80f, 0.20f, 0.45f};
constexpr Vector4 kMoveTarget{0.10f, 0.10f, 0.10f, 0.35f};
constexpr Vector4 kCaptureTarget{0.90f, 0.20f, 0.20f, 0.50f};

} // namespace

void BoardView::draw(Canvas &canvas, const chess::Game &game, const Rect &area, bool flipped) {
    area_ = area;
    squareSize_ = area.w / kBoardSize;
    flipped_ = flipped;

    auto squareRect = [this](Square s, float scale = 1.f) {
        return Rect{centerX(s), centerY(s), squareSize_ * scale, squareSize_ * scale};
    };

    // 1. the checkerboard
    for (int rank = 0; rank < kBoardSize; ++rank) {
        for (int file = 0; file < kBoardSize; ++file) {
            const Square square{file, rank};
            const bool isLight = (file + rank) % 2 == 1;
            canvas.quad(squareRect(square), isLight ? kLightSquare : kDarkSquare);
        }
    }

    // 2. highlights, drawn over the squares but under the pieces
    if (auto lastMove = game.lastMove()) {
        canvas.quad(squareRect(lastMove->from), kLastMove);
        canvas.quad(squareRect(lastMove->to), kLastMove);
    }

    if (auto selected = game.selected()) {
        canvas.quad(squareRect(*selected), kSelected);

        for (const auto &move: game.legalMoves()) {
            if (move.isCapture) {
                canvas.quad(squareRect(move.to), kCaptureTarget);
            } else {
                canvas.quad(squareRect(move.to, 0.3f), kMoveTarget);
            }
        }
    }

    // 3. the pieces
    game.board().forEachPiece([&](const chess::Piece &piece) {
        canvas.piece(centerX(piece.square()), centerY(piece.square()), squareSize_, piece);
    });
}

std::optional<Square> BoardView::squareAt(float x, float y) const {
    int file = static_cast<int>(std::floor((x - area_.left()) / squareSize_));
    int rank = static_cast<int>(std::floor((y - area_.bottom()) / squareSize_));
    if (flipped_) {
        file = kBoardSize - 1 - file;
        rank = kBoardSize - 1 - rank;
    }

    const Square square{file, rank};
    if (!square.isValid()) {
        return std::nullopt;
    }
    return square;
}

float BoardView::centerX(Square square) const {
    const int file = flipped_ ? kBoardSize - 1 - square.file : square.file;
    return area_.left() + (file + 0.5f) * squareSize_;
}

float BoardView::centerY(Square square) const {
    const int rank = flipped_ ? kBoardSize - 1 - square.rank : square.rank;
    return area_.bottom() + (rank + 0.5f) * squareSize_;
}
