#include "Game.h"

namespace chess {

Game::Game() : Game(std::make_unique<ShogunRules>()) {}

Game::Game(std::unique_ptr<Rules> rules) : rules_(std::move(rules)) {
    reset();
}

std::string Game::Record::notation() const {
    std::string text = move.from.name() + (move.isCapture ? "x" : "-") + move.to.name();
    if (powerAfter > 0) {
        text += ">" + std::to_string(powerAfter);
    }
    return text;
}

void Game::reset() {
    rules_->setup(board_);
    turn_ = Color::White;
    lastMove_.reset();
    history_.clear();
    winner_.reset();
    deselect();
}

Game::TapResult Game::tap(Square square) {
    if (isOver()) {
        reset();
        return TapResult::Restarted;
    }

    if (!square.isValid() || isAutomatedTurn()) {
        return TapResult::Ignored;
    }

    if (selected_) {
        if (tryMove(square)) {
            return TapResult::Moved;
        }
        if (*selected_ == square) {
            deselect();
            return TapResult::Deselected;
        }
        if (select(square)) {
            return TapResult::Selected;
        }
        deselect();
        return TapResult::Deselected;
    }

    return select(square) ? TapResult::Selected : TapResult::Ignored;
}

bool Game::select(Square square) {
    const Piece *piece = board_.at(square);
    if (piece == nullptr || piece->color() != turn_) {
        return false;
    }

    selected_ = square;
    legalMoves_ = piece->moves(board_);
    return true;
}

void Game::deselect() {
    selected_.reset();
    legalMoves_.clear();
}

bool Game::tryMove(Square to) {
    const Move *move = legalMoveTo(to);
    if (move == nullptr) {
        return false;
    }

    const Move performed = *move;
    Record record{performed, turn_, board_.at(performed.from)->type(),
                  board_.at(performed.from)->power(), 0, std::nullopt};

    std::unique_ptr<Piece> captured = board_.move(performed.from, performed.to);
    rules_->afterMove(board_, performed, *board_.at(performed.to), captured.get());

    record.powerAfter = board_.at(performed.to)->power();
    if (captured) {
        record.captured = captured->type();
    }
    history_.push_back(record);
    lastMove_ = performed;
    winner_ = rules_->winner(board_);
    endTurn();
    return true;
}

bool Game::play(const Move &move) {
    if (isOver() || !select(move.from)) {
        return false;
    }
    if (!tryMove(move.to)) {
        deselect();
        return false;
    }
    return true;
}

const Move *Game::legalMoveTo(Square to) const {
    for (const auto &move: legalMoves_) {
        if (move.to == to) {
            return &move;
        }
    }
    return nullptr;
}

void Game::endTurn() {
    deselect();
    turn_ = opposite(turn_);
}

} // namespace chess
