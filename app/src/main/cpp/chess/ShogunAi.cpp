#include "ShogunAi.h"

#include <algorithm>
#include <limits>
#include <vector>

#include "Rules.h"

namespace chess {

namespace {

constexpr float kWin = 100000.f;
// Small random jitter so the AI doesn't always pick the first of equally good moves.
constexpr float kJitter = 0.5f;

std::vector<Move> allMoves(const Board &board, Color side) {
    std::vector<Move> moves;
    board.forEachPiece([&](const Piece &piece) {
        if (piece.color() == side) {
            auto own = piece.moves(board);
            moves.insert(moves.end(), own.begin(), own.end());
        }
    });
    return moves;
}

} // namespace

ShogunAi::ShogunAi(int depth) : depth_(depth), rng_(std::random_device{}()) {}

std::optional<Move> ShogunAi::chooseMove(const Board &original, Color side) {
    Board board = original;
    std::vector<Move> moves = allMoves(board, side);
    if (moves.empty()) {
        return std::nullopt;
    }

    // Captures first: makes alpha-beta cut earlier.
    std::stable_partition(moves.begin(), moves.end(), [](const Move &m) { return m.isCapture; });

    std::uniform_real_distribution<float> jitter(0.f, kJitter);
    std::optional<Move> best;
    float bestScore = -std::numeric_limits<float>::infinity();

    for (const auto &move: moves) {
        const float score = expectAfterMove(board, move, side, side, depth_ - 1,
                                            -std::numeric_limits<float>::infinity(),
                                            std::numeric_limits<float>::infinity())
                            + jitter(rng_);
        if (score > bestScore) {
            bestScore = score;
            best = move;
        }
    }
    return best;
}

float ShogunAi::search(Board &board, Color toMove, Color side, int depth, float alpha,
                       float beta) {
    if (auto winner = ShogunRules::outcome(board)) {
        // Prefer quick wins and slow losses.
        return (*winner == side ? kWin : -kWin) + (depth * (*winner == side ? 1.f : -1.f));
    }
    if (depth <= 0) {
        return evaluate(board, side, toMove);
    }

    std::vector<Move> moves = allMoves(board, toMove);
    if (moves.empty()) {
        return evaluate(board, side, toMove);
    }
    std::stable_partition(moves.begin(), moves.end(), [](const Move &m) { return m.isCapture; });

    const bool maximizing = toMove == side;
    float best = maximizing ? -std::numeric_limits<float>::infinity()
                            : std::numeric_limits<float>::infinity();

    for (const auto &move: moves) {
        const float score = expectAfterMove(board, move, toMove, side, depth - 1, alpha, beta);
        if (maximizing) {
            best = std::max(best, score);
            alpha = std::max(alpha, best);
        } else {
            best = std::min(best, score);
            beta = std::min(beta, best);
        }
        if (beta <= alpha) {
            break;
        }
    }
    return best;
}

float ShogunAi::expectAfterMove(Board &board, const Move &move, Color mover, Color side,
                                int depth, float alpha, float beta) {
    Board::Undo undo = board.makeMove(move);
    Piece *moved = board.at(move.to);
    const auto [minPower, maxPower] = ShogunRules::powerRange(*moved);

    // If the game is decided or we've hit the horizon the new number doesn't matter, so skip
    // the chance node; this keeps leaf evaluation cheap.
    float total = 0.f;
    if (depth <= 0 || ShogunRules::outcome(board)) {
        total = search(board, opposite(mover), side, depth, alpha, beta);
    } else {
        for (int power = minPower; power <= maxPower; ++power) {
            moved->setPower(power);
            total += search(board, opposite(mover), side, depth, alpha, beta);
        }
        total /= static_cast<float>(maxPower - minPower + 1);
    }

    board.unmove(std::move(undo));
    return total;
}

float ShogunAi::evaluate(const Board &board, Color side, Color toMove) const {
    float score = 0.f;
    int pieces[2] = {0, 0};

    board.forEachPiece([&](const Piece &piece) {
        const float sign = piece.color() == side ? 1.f : -1.f;
        ++pieces[static_cast<int>(piece.color())];
        score += sign * weights_.piece;

        // Threats are worth more for the side that gets to act on them first.
        const float urgency = piece.color() == toMove ? weights_.toMoveFactor : 1.f;

        for (const auto &move: piece.moves(board)) {
            score += sign * weights_.mobility;
            if (!move.isCapture) {
                continue;
            }
            const Piece *target = board.at(move.to);
            const bool isKing = target != nullptr && target->type() == PieceType::King;
            score += sign * urgency * (isKing ? weights_.kingThreat : weights_.threat);
        }
    });

    // With exactly the minimum number of pieces left, every loss ends the game.
    for (Color color: {Color::White, Color::Black}) {
        if (pieces[static_cast<int>(color)] <= ShogunRules::kMinPiecesToPlay) {
            score += (color == side ? -1.f : 1.f) * weights_.lastPieces;
        }
    }
    return score;
}

} // namespace chess
