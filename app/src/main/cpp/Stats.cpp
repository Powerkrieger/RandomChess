#include "Stats.h"

#include <cstdio>
#include <fstream>
#include <sstream>

bool Stats::load(const std::string &path) {
    records_.clear();
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    std::string line;
    while (std::getline(in, line)) {
        GameRecord record;
        if (fromLine(line, record)) {
            records_.push_back(record);
        }
    }
    return true;
}

bool Stats::save(const std::string &path) const {
    // Write to a temp file and rename it over the old one, so the app being killed mid-write
    // leaves the previous history intact instead of a truncated file.
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) {
            return false;
        }
        for (const auto &record: records_) {
            out << toLine(record) << '\n';
        }
        if (!out) {
            return false;
        }
    }
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::remove(tmp.c_str());
        return false;
    }
    return true;
}

std::string Stats::toLine(const GameRecord &r) {
    std::ostringstream out;
    out << r.date << '\t' << r.white << '\t' << r.black << '\t' << r.difficulty << '\t'
        << r.winner << '\t' << r.moves << '\t' << r.startSetup;
    return out.str();
}

bool Stats::fromLine(const std::string &line, GameRecord &out) {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream in(line);
    while (std::getline(in, field, '\t')) {
        fields.push_back(field);
    }
    if (fields.size() != 7) {
        return false;
    }
    out.date = fields[0];
    out.white = fields[1];
    out.black = fields[2];
    out.difficulty = fields[3];
    out.winner = fields[4];
    try {
        out.moves = std::stoi(fields[5]);
    } catch (...) {
        return false;
    }
    out.startSetup = fields[6];
    return true;
}

Stats::Summary Stats::summary() const {
    Summary s;
    for (const auto &r: records_) {
        const bool computerWhite = r.white == "Computer";
        const bool computerBlack = r.black == "Computer";
        const bool whiteWon = r.winner == "White";

        if (!computerWhite && !computerBlack) {
            (whiteWon ? s.twoPlayerWhiteWins : s.twoPlayerBlackWins)++;
            continue;
        }
        if (computerWhite && computerBlack) {
            continue;  // computer vs computer: not tracked
        }

        const bool humanWhite = !computerWhite;
        const bool humanWon = humanWhite == whiteWon;
        auto count = [humanWon](Tally &tally) { (humanWon ? tally.wins : tally.losses)++; };
        count(s.vsComputer);
        count(humanWhite ? s.asWhite : s.asBlack);
        if (r.difficulty == "Easy") count(s.easy);
        else if (r.difficulty == "Normal") count(s.normal);
        else if (r.difficulty == "Hard") count(s.hard);
    }
    return s;
}
