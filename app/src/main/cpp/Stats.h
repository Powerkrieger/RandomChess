#ifndef RANDOMCHESS_STATS_H
#define RANDOMCHESS_STATS_H

#include <string>
#include <vector>

/*!
 * One finished game, as kept in the results history.
 */
struct GameRecord {
    std::string date;         // "2026-09-15 14:02"
    std::string white;        // "You", "Computer" or "Player 1"
    std::string black;        // "You", "Computer" or "Player 2"
    std::string difficulty;   // "Easy", "Normal", "Hard" or "-" for two-player games
    std::string winner;       // "White" or "Black"
    int moves = 0;            // plies played
    std::string startSetup;   // starting numbers, white home rank then black: "41432332/13112414"
};

/*!
 * The results history: a list of finished games persisted as a tab-separated text file, plus
 * the aggregates the history screen shows. Pure C++ so it can be unit-tested on the host.
 */
class Stats {
public:
    struct Tally {
        int wins = 0;
        int losses = 0;

        std::string text() const { return std::to_string(wins) + " - " + std::to_string(losses); }
    };

    struct Summary {
        Tally vsComputer;       // from the human's point of view
        Tally asWhite;          // human as white vs computer
        Tally asBlack;          // human as black vs computer
        Tally easy, normal, hard;
        int twoPlayerWhiteWins = 0;
        int twoPlayerBlackWins = 0;
    };

    /*! Loads the file if it exists; a missing or unreadable file gives an empty history. */
    bool load(const std::string &path);

    bool save(const std::string &path) const;

    void add(const GameRecord &record) { records_.push_back(record); }

    /*! Oldest first. */
    const std::vector<GameRecord> &records() const { return records_; }

    Summary summary() const;

    /*! Serialisation of a record as one line (fields separated by tabs). */
    static std::string toLine(const GameRecord &record);

    /*! Parses a line written by toLine(); returns false if it's malformed. */
    static bool fromLine(const std::string &line, GameRecord &out);

private:
    std::vector<GameRecord> records_;
};

#endif //RANDOMCHESS_STATS_H
