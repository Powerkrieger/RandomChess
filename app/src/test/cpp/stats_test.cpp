#include "check.h"

#include <cstdio>

#include "Stats.h"

static GameRecord rec(const char *white, const char *black, const char *diff, const char *winner) {
    return GameRecord{"2026-09-15 14:02", white, black, diff, winner, 23, "41432332/13112414"};
}

TEST(round_trip_through_line_format) {
    GameRecord r = rec("You", "Computer", "Hard", "White");
    GameRecord back;
    CHECK(Stats::fromLine(Stats::toLine(r), back));
    CHECK(back.date == r.date && back.white == r.white && back.black == r.black);
    CHECK(back.difficulty == r.difficulty && back.winner == r.winner);
    CHECK(back.moves == 23 && back.startSetup == r.startSetup);
    CHECK(!Stats::fromLine("garbage", back));
    CHECK(!Stats::fromLine("a\tb\tc\td\te\tnotanumber\tf", back));
}

TEST(summary_counts_from_the_humans_point_of_view) {
    Stats s;
    s.add(rec("You", "Computer", "Easy", "White"));      // win as white
    s.add(rec("You", "Computer", "Normal", "Black"));    // loss as white
    s.add(rec("Computer", "You", "Hard", "Black"));      // win as black
    s.add(rec("Computer", "You", "Hard", "White"));      // loss as black
    s.add(rec("Player 1", "Player 2", "-", "Black"));    // two players
    auto sum = s.summary();
    CHECK(sum.vsComputer.wins == 2 && sum.vsComputer.losses == 2);
    CHECK(sum.asWhite.wins == 1 && sum.asWhite.losses == 1);
    CHECK(sum.asBlack.wins == 1 && sum.asBlack.losses == 1);
    CHECK(sum.easy.wins == 1 && sum.normal.losses == 1 && sum.hard.wins == 1 && sum.hard.losses == 1);
    CHECK(sum.twoPlayerWhiteWins == 0 && sum.twoPlayerBlackWins == 1);
}

TEST(save_and_load) {
    const char *path = "stats_test_tmp.tsv";
    Stats s;
    s.add(rec("You", "Computer", "Easy", "White"));
    s.add(rec("Computer", "You", "Hard", "Black"));
    CHECK(s.save(path));

    Stats loaded;
    CHECK(loaded.load(path));
    CHECK(loaded.records().size() == 2);
    CHECK(loaded.records()[1].black == "You");
    std::remove(path);

    Stats missing;
    CHECK(!missing.load("does/not/exist.tsv"));
    CHECK(missing.records().empty());
}

int main() { return finish(); }
