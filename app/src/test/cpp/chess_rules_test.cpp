#include "check.h"

#include "chess/Game.h"
#include "chess/Pieces.h"

using namespace chess;

TEST(standard_setup_has_32_pieces) {
    Game g(std::make_unique<ChessRules>());
    int count = 0;
    g.board().forEachPiece([&](const Piece &) { ++count; });
    CHECK(count == 32);
    CHECK(g.board().at({4, 0})->type() == PieceType::King);
    CHECK(g.board().at({3, 7})->type() == PieceType::Queen);
}

TEST(selection_state_machine) {
    Game g(std::make_unique<ChessRules>());
    CHECK(g.tap({4, 4}) == Game::TapResult::Ignored);     // empty square
    CHECK(g.tap({4, 6}) == Game::TapResult::Ignored);     // black pawn, white to move
    CHECK(g.tap({4, 1}) == Game::TapResult::Selected);    // e2
    CHECK(g.legalMoves().size() == 2);                    // e3, e4
    CHECK(g.tap({4, 1}) == Game::TapResult::Deselected);
    CHECK(g.tap({6, 0}) == Game::TapResult::Selected);    // g1 knight
    CHECK(g.legalMoves().size() == 2);
    CHECK(g.tap({4, 1}) == Game::TapResult::Selected);    // switch selection
    CHECK(g.tap({4, 3}) == Game::TapResult::Moved);       // e4
    CHECK(g.turn() == Color::Black);
    CHECK(g.history().size() == 1);
    CHECK(g.history()[0].notation() == "e2-e4");
}

TEST(capture_removes_piece_and_is_recorded) {
    Game g(std::make_unique<ChessRules>());
    g.play({{4, 1}, {4, 3}, false});
    g.play({{3, 6}, {3, 4}, false});
    CHECK(g.tap({4, 3}) == Game::TapResult::Selected);
    const Move *capture = g.legalMoveTo({3, 4});
    CHECK(capture != nullptr && capture->isCapture);
    CHECK(g.tap({3, 4}) == Game::TapResult::Moved);
    CHECK(g.board().at({3, 4})->color() == Color::White);
    CHECK(g.board().at({4, 3}) == nullptr);
    CHECK(g.history().back().notation() == "e4xd5");
    CHECK(g.history().back().captured == PieceType::Pawn);
}

TEST(sliders_stop_at_blockers) {
    Board b;
    b.place(std::make_unique<Rook>(Color::White, Square{0, 0}));
    b.place(std::make_unique<Pawn>(Color::White, Square{0, 2}));
    b.place(std::make_unique<Pawn>(Color::Black, Square{3, 0}));
    auto moves = b.at({0, 0})->moves(b);
    CHECK(moves.size() == 4);  // a2, b1, c1, d1(x)
    bool capturesD1 = false;
    for (const auto &m: moves) {
        if (m.to == Square{3, 0}) capturesD1 = m.isCapture;
        CHECK(m.to != Square{0, 2});
        CHECK(m.to != Square{0, 3});
    }
    CHECK(capturesD1);
}

TEST(pawn_promotes_to_queen) {
    Game g(std::make_unique<ChessRules>());
    Board &b = const_cast<Board &>(g.board());
    b.clear();
    b.place(std::make_unique<King>(Color::White, Square{4, 0}));
    b.place(std::make_unique<King>(Color::Black, Square{4, 7}));
    b.place(std::make_unique<Pawn>(Color::White, Square{0, 6}));
    CHECK(g.play({{0, 6}, {0, 7}, false}));
    CHECK(g.board().at({0, 7})->type() == PieceType::Queen);
}

int main() { return finish(); }
