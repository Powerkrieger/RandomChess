#include "check.h"

#include <algorithm>

#include "chess/Game.h"
#include "chess/Pieces.h"

using namespace chess;

static bool has(const std::vector<Move> &m, Square s, bool capture = false) {
    return std::any_of(m.begin(), m.end(),
                       [&](const Move &x) { return x.to == s && x.isCapture == capture; });
}

TEST(walk_on_empty_board) {
    Board b;
    auto *s = new Soldier(Color::White, {4, 4});
    b.place(std::unique_ptr<Piece>(s));

    s->setPower(1);
    CHECK(s->moves(b).size() == 4);                          // orthogonal only

    s->setPower(2);
    auto m = s->moves(b);
    CHECK(m.size() == 8);                                    // 4 straight + 4 via a corner
    CHECK(has(m, {4, 6}) && has(m, {5, 5}) && !has(m, {6, 6}));

    s->setPower(3);
    m = s->moves(b);
    CHECK(m.size() == 12);                                   // 2+1 and 1+2 reach the same squares
    CHECK(has(m, {4, 7}) && has(m, {5, 6}) && has(m, {6, 5}));
    CHECK(!has(m, {6, 6}) && !has(m, {5, 5}));

    s->setPower(4);
    m = s->moves(b);
    CHECK(has(m, {4, 0}) && has(m, {6, 6}) && has(m, {5, 7}) && has(m, {7, 5}));
    CHECK(!has(m, {5, 5}) && !has(m, {4, 5}));
}

TEST(walk_is_blocked_by_pieces) {
    Board b;
    auto *s = new Soldier(Color::White, {4, 4});
    b.place(std::unique_ptr<Piece>(s));
    b.place(std::make_unique<Soldier>(Color::White, Square{4, 5}));

    s->setPower(2);
    auto m = s->moves(b);
    CHECK(!has(m, {4, 6}));                                  // no jumping
    CHECK(has(m, {3, 5}) && has(m, {5, 5}));                 // other corners still work

    b.remove({4, 5});
    b.place(std::make_unique<Soldier>(Color::Black, Square{4, 5}));
    s->setPower(1);
    CHECK(has(s->moves(b), {4, 5}, true));                   // adjacent enemy: capture
    s->setPower(2);
    m = s->moves(b);
    CHECK(!has(m, {4, 5}) && !has(m, {4, 6}));               // ...but not through it
}

TEST(capture_needs_a_clear_corner) {
    Board b;
    auto *s = new Soldier(Color::White, {0, 0});
    b.place(std::unique_ptr<Piece>(s));
    b.place(std::make_unique<Soldier>(Color::Black, Square{1, 1}));
    s->setPower(2);
    CHECK(has(s->moves(b), {1, 1}, true));
    b.place(std::make_unique<Soldier>(Color::White, Square{0, 1}));
    CHECK(has(s->moves(b), {1, 1}, true));                   // via b1
    b.place(std::make_unique<Soldier>(Color::White, Square{1, 0}));
    CHECK(!has(s->moves(b), {1, 1}, true));                  // both corners blocked
}

TEST(setup_and_rolls) {
    Game g(std::make_unique<ShogunRules>(42));
    int white = 0, black = 0, kings = 0;
    g.board().forEachPiece([&](const Piece &p) {
        (p.color() == Color::White ? white : black)++;
        if (p.type() == PieceType::King) kings++;
        const int max = p.type() == PieceType::King ? 2 : 4;
        CHECK(p.power() >= 1 && p.power() <= max);
    });
    CHECK(white == 8 && black == 8 && kings == 2);
    CHECK(!g.isOver());
}

TEST(power_rerolls_after_move_and_is_recorded) {
    Game g(std::make_unique<ShogunRules>(1));
    const Piece *p = g.board().at({0, 0});
    const int before = p->power();
    Square to = {0, before};
    CHECK(g.play({{0, 0}, to, false}));
    const auto &rec = g.history().back();
    CHECK(rec.powerBefore == before);
    CHECK(rec.powerAfter == g.board().at(to)->power());
    CHECK(rec.notation() == "a1-" + to.name() + ">" + std::to_string(rec.powerAfter));
}

TEST(win_conditions) {
    ShogunRules rules(1);
    Board b;
    rules.setup(b);
    CHECK(!rules.winner(b));
    b.remove({4, 7});
    CHECK(rules.winner(b) == Color::White);                  // black king gone

    rules.setup(b);
    for (int f: {0, 1, 2, 3, 5, 6}) b.remove({f, 0});
    CHECK(rules.winner(b) == Color::Black);                  // white has 2 pieces
}

TEST(taps_ignored_on_automated_turn_and_restart_after_end) {
    Game g(std::make_unique<ShogunRules>(3));
    g.setAutomatedSide(Color::White);
    CHECK(g.tap({0, 0}) == Game::TapResult::Ignored);
    g.setAutomatedSide(std::nullopt);

    Board &b = const_cast<Board &>(g.board());
    b.remove({4, 7});
    // a move by white triggers the end check
    const Piece *p = g.board().at({0, 0});
    CHECK(g.play({{0, 0}, {0, p->power()}, false}));
    CHECK(g.isOver() && g.winner() == Color::White);
    CHECK(g.tap({0, 0}) == Game::TapResult::Restarted);
    CHECK(!g.isOver() && g.history().empty());
}

int main() { return finish(); }
