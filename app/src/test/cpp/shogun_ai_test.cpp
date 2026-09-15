#include "check.h"

#include "chess/Game.h"
#include "chess/Pieces.h"
#include "chess/ShogunAi.h"

using namespace chess;

TEST(takes_the_king_when_possible) {
    Board b;
    b.place(std::make_unique<Shogun>(Color::White, Square{4, 0}));
    for (int f: {0, 1, 2}) b.place(std::make_unique<Soldier>(Color::White, Square{f, 0}));
    b.place(std::make_unique<Shogun>(Color::Black, Square{4, 7}));
    for (int f: {5, 6, 7}) b.place(std::make_unique<Soldier>(Color::Black, Square{f, 7}));
    auto *attacker = new Soldier(Color::Black, Square{4, 3});
    b.place(std::unique_ptr<Piece>(attacker));
    b.forEachPiece([](const Piece &p) { const_cast<Piece &>(p).setPower(1); });
    attacker->setPower(3);

    for (int depth: {1, 2, 3}) {
        ShogunAi ai(depth);
        auto m = ai.chooseMove(b, Color::Black);
        CHECK(m && m->to == Square{4, 0} && m->isCapture);
    }
}

TEST(does_not_hang_the_king) {
    // Black king on e8 (power 1). White soldier on e4 with power 3 could reach e7? No: e4->e7 is
    // 3 straight, so a black piece stepping *away* from e7 would let white capture e8 next.
    // Simpler: black to move, white threatens e8 via e5 (power 3); black must capture or block.
    Board b;
    b.place(std::make_unique<Shogun>(Color::White, Square{4, 0}));
    for (int f: {0, 1, 2}) b.place(std::make_unique<Soldier>(Color::White, Square{f, 0}));
    b.place(std::make_unique<Shogun>(Color::Black, Square{4, 7}));
    for (int f: {0, 1}) b.place(std::make_unique<Soldier>(Color::Black, Square{f, 7}));
    auto *threat = new Soldier(Color::White, Square{4, 4});
    b.place(std::unique_ptr<Piece>(threat));
    auto *defender = new Soldier(Color::Black, Square{4, 6});   // e7 blocks the path
    b.place(std::unique_ptr<Piece>(defender));
    b.forEachPiece([](const Piece &p) { const_cast<Piece &>(p).setPower(1); });
    threat->setPower(3);
    defender->setPower(2);

    ShogunAi ai(2);
    auto m = ai.chooseMove(b, Color::Black);
    CHECK(m.has_value());
    // Moving the defender off the e-file (e.g. to d6/f6 via the corner) would expose the king.
    CHECK(!(m->from == Square{4, 6} && m->to.file != 4));
}

TEST(self_play_finishes) {
    Game g(std::make_unique<ShogunRules>(3));
    ShogunAi white(2), black(2);
    int plies = 0;
    while (!g.isOver() && plies < 400) {
        auto mv = (g.turn() == Color::White ? white : black).chooseMove(g.board(), g.turn());
        if (!mv) break;
        CHECK(g.play(*mv));
        ++plies;
    }
    CHECK(g.isOver());
}

TEST(board_copy_and_unmove_restore_state) {
    Game g(std::make_unique<ShogunRules>(9));
    Board b = g.board();
    const Piece *p = b.at({0, 0});
    Move m{{0, 0}, {0, p->power()}, false};
    const int power = p->power();

    Board::Undo undo = b.makeMove(m);
    CHECK(b.at({0, 0}) == nullptr);
    b.at(m.to)->setPower(99);
    b.unmove(std::move(undo));
    CHECK(b.at({0, 0}) != nullptr && b.at({0, 0})->power() == power);
    CHECK(b.at(m.to) == nullptr);
    CHECK(!b.at({0, 0})->hasMoved());
    // the original game board was never touched
    CHECK(g.board().at({0, 0})->power() == power);
}

int main() { return finish(); }
