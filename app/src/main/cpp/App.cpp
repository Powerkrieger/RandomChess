#include "App.h"

#include <algorithm>
#include <ctime>

#include "AndroidOut.h"

using chess::Color;

namespace {

// Minimum time between the human's move and the computer's reply, so the reply reads as a move.
constexpr auto kAiReplyDelay = std::chrono::milliseconds(600);

constexpr Vector4 kText{0.93f, 0.91f, 0.87f, 1.f};
constexpr Vector4 kMutedText{0.62f, 0.60f, 0.56f, 1.f};
constexpr Vector4 kButton{0.24f, 0.23f, 0.21f, 1.f};
constexpr Vector4 kButtonSelected{0.71f, 0.53f, 0.39f, 1.f};
constexpr Vector4 kPanel{0.18f, 0.17f, 0.16f, 1.f};
constexpr Vector4 kDim{0.f, 0.f, 0.f, 0.6f};
constexpr Vector4 kWhiteTurn{1.f, 1.f, 1.f, 1.f};
constexpr Vector4 kBlackTurn{0.12f, 0.12f, 0.12f, 1.f};

constexpr float kMargin = 0.06f;
// Extra space above the header so it clears the camera cutout in immersive mode.
constexpr float kTopInset = 0.14f;

constexpr const char *kStatsFile = "results.tsv";

std::string now() {
    char buffer[32];
    const std::time_t t = std::time(nullptr);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", std::localtime(&t));
    return buffer;
}

/*! The numbers on a home rank, a-file first, e.g. "41432332". */
std::string rankNumbers(const chess::Board &board, int rank) {
    std::string out;
    for (int file = 0; file < chess::kBoardSize; ++file) {
        const chess::Piece *piece = board.at({file, rank});
        out += piece ? static_cast<char>('0' + piece->power()) : '.';
    }
    return out;
}

} // namespace

App::App() : ai_(searchDepth(Difficulty::Normal)) {}

void App::setDataDir(const std::string &dir) {
    dataDir_ = dir;
    stats_.load(dataDir_ + "/" + kStatsFile);
    aout << "Loaded " << stats_.records().size() << " results from " << dataDir_ << std::endl;
}

int App::searchDepth(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            return 1;
        case Difficulty::Normal:
            return 2;
        case Difficulty::Hard:
            return 3;
    }
    return 2;
}

// ---------------------------------------------------------------------------------------------
// state changes
// ---------------------------------------------------------------------------------------------

void App::newGame() {
    // Drop any search that's still running for the previous game.
    if (pendingAiMove_.valid()) {
        pendingAiMove_.wait();
        pendingAiMove_ = {};
    }

    game_.reset();
    if (opponent_ == Opponent::Computer) {
        game_.setAutomatedSide(chess::opposite(humanSide_));
        ai_.setDepth(searchDepth(difficulty_));
    } else {
        game_.setAutomatedSide(std::nullopt);
    }

    startSetup_ = rankNumbers(game_.board(), 0) + "/" +
                  rankNumbers(game_.board(), chess::kBoardSize - 1);
    resultRecorded_ = false;
    screen_ = Screen::Game;
}

std::string App::playerName(Color side) const {
    if (opponent_ == Opponent::Human) {
        return side == Color::White ? "Player 1" : "Player 2";
    }
    return side == humanSide_ ? "You" : "Computer";
}

std::string App::difficultyName() const {
    if (opponent_ == Opponent::Human) {
        return "-";
    }
    switch (difficulty_) {
        case Difficulty::Easy:
            return "Easy";
        case Difficulty::Normal:
            return "Normal";
        case Difficulty::Hard:
            return "Hard";
    }
    return "-";
}

void App::recordResultIfOver() {
    if (resultRecorded_ || !game_.isOver()) {
        return;
    }
    resultRecorded_ = true;

    GameRecord record;
    record.date = now();
    record.white = playerName(Color::White);
    record.black = playerName(Color::Black);
    record.difficulty = difficultyName();
    record.winner = *game_.winner() == Color::White ? "White" : "Black";
    record.moves = static_cast<int>(game_.history().size());
    record.startSetup = startSetup_;
    stats_.add(record);

    if (!dataDir_.empty() && !stats_.save(dataDir_ + "/" + kStatsFile)) {
        aout << "Could not save results to " << dataDir_ << std::endl;
    }
}

void App::perform(Action action) {
    switch (action) {
        case Action::PlayWhite:
            humanSide_ = Color::White;
            opponent_ = Opponent::Computer;
            break;
        case Action::PlayBlack:
            humanSide_ = Color::Black;
            opponent_ = Opponent::Computer;
            break;
        case Action::TwoPlayers:
            humanSide_ = Color::White;
            opponent_ = Opponent::Human;
            break;
        case Action::Easy:
            difficulty_ = Difficulty::Easy;
            break;
        case Action::Normal:
            difficulty_ = Difficulty::Normal;
            break;
        case Action::Hard:
            difficulty_ = Difficulty::Hard;
            break;
        case Action::Start:
        case Action::NewGame:
            newGame();
            break;
        case Action::Menu:
            screen_ = Screen::Menu;
            break;
        case Action::History:
            historyPage_ = 0;
            screen_ = Screen::History;
            break;
        case Action::HistoryNewer:
            historyPage_ = std::max(0, historyPage_ - 1);
            break;
        case Action::HistoryOlder:
            ++historyPage_;
            break;
    }
}

void App::tap(float x, float y) {
    for (const auto &button: buttons_) {
        if (button.rect.contains(x, y)) {
            perform(button.action);
            return;
        }
    }

    if (screen_ == Screen::Game) {
        if (auto square = boardView_.squareAt(x, y)) {
            const bool wasOver = game_.isOver();
            auto result = game_.tap(*square);
            aout << "Tap on " << square->name() << " -> " << static_cast<int>(result) << std::endl;
            if (wasOver && result == chess::Game::TapResult::Restarted) {
                newGame();
            } else {
                recordResultIfOver();
            }
        }
    }
}

void App::update() {
    if (screen_ != Screen::Game || !game_.isAutomatedTurn()) {
        return;
    }

    // The search runs on its own thread on a copy of the board so the UI keeps rendering.
    if (!pendingAiMove_.valid()) {
        aiStartedAt_ = std::chrono::steady_clock::now();
        pendingAiMove_ = std::async(std::launch::async,
                                    [this, board = game_.board(), side = game_.turn()]() mutable {
                                        return ai_.chooseMove(board, side);
                                    });
        return;
    }

    const bool ready =
            pendingAiMove_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    const bool delayPassed = std::chrono::steady_clock::now() - aiStartedAt_ >= kAiReplyDelay;
    if (!ready || !delayPassed) {
        return;
    }

    std::optional<chess::Move> move = pendingAiMove_.get();
    if (move) {
        aout << "AI plays " << move->from.name() << " -> " << move->to.name() << std::endl;
        game_.play(*move);
        recordResultIfOver();
    } else {
        aout << "AI has no legal move" << std::endl;
    }
}

// ---------------------------------------------------------------------------------------------
// drawing
// ---------------------------------------------------------------------------------------------

void App::draw(Canvas &canvas) {
    buttons_.clear();
    switch (screen_) {
        case Screen::Menu:
            drawMenu(canvas);
            break;
        case Screen::Game:
            drawGame(canvas);
            break;
        case Screen::History:
            drawHistory(canvas);
            break;
    }
}

void App::button(Canvas &canvas, const Rect &rect, Action action, const std::string &label,
                 bool selected, float textHeight) {
    canvas.quad(rect, selected ? kButtonSelected : kButton);
    canvas.text(rect.cx, rect.cy, textHeight, label, kText, Align::Center);
    buttons_.push_back(Button{rect, action, label, selected});
}

void App::drawMenu(Canvas &canvas) {
    const float hw = canvas.halfWidth();
    const float hh = canvas.halfHeight();
    const float width = std::min(2.f * hw, 2.f * hh) - 2.f * kMargin;

    canvas.text(0.f, hh * 0.55f - kTopInset * 0.5f, 0.28f, "SHOGUN", kText, Align::Center);
    canvas.text(0.f, hh * 0.55f - 0.28f, 0.08f, "the number says how far you go", kMutedText,
                Align::Center);

    // a row of equally wide buttons
    auto row = [&](float y, std::initializer_list<std::pair<Action, const char *>> items,
                   auto isSelected) {
        const float gap = 0.04f;
        const float w = (width - gap * (items.size() - 1)) / items.size();
        float x = -width * 0.5f + w * 0.5f;
        for (const auto &[action, label]: items) {
            button(canvas, Rect{x, y, w, 0.22f}, action, label, isSelected(action));
            x += w + gap;
        }
    };

    float y = hh * 0.55f - 0.7f;
    canvas.text(-width * 0.5f, y, 0.09f, "You play", kMutedText);
    y -= 0.2f;
    row(y, {{Action::PlayWhite, "White"}, {Action::PlayBlack, "Black"},
            {Action::TwoPlayers, "2 players"}},
        [this](Action a) {
            if (opponent_ == Opponent::Human) return a == Action::TwoPlayers;
            return a == (humanSide_ == Color::White ? Action::PlayWhite : Action::PlayBlack);
        });

    y -= 0.4f;
    canvas.text(-width * 0.5f, y, 0.09f, "Computer strength", kMutedText);
    y -= 0.2f;
    row(y, {{Action::Easy, "Easy"}, {Action::Normal, "Normal"}, {Action::Hard, "Hard"}},
        [this](Action a) {
            return a == (difficulty_ == Difficulty::Easy ? Action::Easy
                         : difficulty_ == Difficulty::Normal ? Action::Normal : Action::Hard);
        });

    y -= 0.5f;
    button(canvas, Rect{0.f, y, width, 0.28f}, Action::Start, "Start game", true, 0.12f);

    y -= 0.42f;
    button(canvas, Rect{0.f, y, width, 0.24f}, Action::History, "Results history", false, 0.09f);
}

void App::drawHistory(Canvas &canvas) {
    const float hw = canvas.halfWidth();
    const float hh = canvas.halfHeight();
    const float left = -hw + kMargin;
    const float right = hw - kMargin;
    const float width = right - left;
    const float pad = 0.06f;

    // --- header --------------------------------------------------------------------------------
    const float headerY = hh - kTopInset - 0.11f;
    canvas.text(left, headerY, 0.14f, "RESULTS", kText);
    button(canvas, Rect{right - 0.21f, headerY, 0.42f, 0.2f}, Action::Menu, "Back", false, 0.08f);

    // --- summary: two columns of label/value pairs in a panel ---------------------------------
    const Stats::Summary s = stats_.summary();
    const float textH = 0.075f;
    const float rowH = 0.12f;
    const Rect summary{0.f, headerY - 0.2f - (4 * rowH + 2 * pad) * 0.5f, width, 4 * rowH + 2 * pad};
    canvas.quad(summary, kPanel);

    const float colW = (width - 2 * pad) * 0.5f;
    auto cell = [&](int column, int row, const std::string &label, const std::string &value) {
        const float x = summary.left() + pad + column * colW;
        const float y = summary.top() - pad - textH * 0.5f - row * rowH;
        canvas.text(x, y, textH, label, kMutedText);
        canvas.text(x + colW - 0.1f, y, textH, value, kText, Align::Right);
    };
    cell(0, 0, "vs computer", s.vsComputer.text());
    cell(0, 1, "  as White", s.asWhite.text());
    cell(0, 2, "  as Black", s.asBlack.text());
    cell(0, 3, "2 players W-B",
         std::to_string(s.twoPlayerWhiteWins) + " - " + std::to_string(s.twoPlayerBlackWins));
    cell(1, 0, "by strength", "");
    cell(1, 1, "  Easy", s.easy.text());
    cell(1, 2, "  Normal", s.normal.text());
    cell(1, 3, "  Hard", s.hard.text());

    // --- games table, newest first, paged ------------------------------------------------------
    const float navY = -hh + kMargin + 0.11f;
    const Rect table{0.f, (summary.bottom() - kMargin + navY + 0.14f) * 0.5f, width,
                     summary.bottom() - kMargin - (navY + 0.14f)};
    canvas.quad(table, kPanel);

    const float lineH = 0.105f;
    const int gamesPerPage = std::max(1, static_cast<int>((table.h - 2 * pad - 0.1f) / lineH));
    const auto &records = stats_.records();
    const int pages = std::max(1, (static_cast<int>(records.size()) + gamesPerPage - 1) / gamesPerPage);
    historyPage_ = std::min(historyPage_, pages - 1);

    // fixed-width columns (monospace font): glyph advance decides the x positions
    const float smallH = 0.062f;
    const float adv = Canvas::textWidth(smallH, 2) - Canvas::textWidth(smallH, 1);
    const float x0 = table.left() + pad;
    const float xDate = x0;
    const float xWinner = x0 + adv * 7;
    const float xLevel = x0 + adv * 21;
    const float xStart = x0 + adv * 29;
    const float xMoves = table.right() - pad;

    float y = table.top() - pad - smallH * 0.5f;
    canvas.text(xDate, y, smallH, "Date", kMutedText);
    canvas.text(xWinner, y, smallH, "Winner", kMutedText);
    canvas.text(xLevel, y, smallH, "Level", kMutedText);
    canvas.text(xStart, y, smallH, "Start W / B", kMutedText);
    canvas.text(xMoves, y, smallH, "Moves", kMutedText, Align::Right);
    y -= 0.1f;

    if (records.empty()) {
        canvas.text(table.cx, y - 0.1f, textH, "no games played yet", kMutedText, Align::Center);
    }

    const int first = static_cast<int>(records.size()) - 1 - historyPage_ * gamesPerPage;
    const int last = std::max(-1, first - gamesPerPage);
    for (int i = first; i > last && y - lineH * 0.5f > table.bottom(); --i) {
        const GameRecord &r = records[i];
        const bool whiteWon = r.winner == "White";
        const std::string winner = (whiteWon ? r.white : r.black) + (whiteWon ? " (W)" : " (B)");
        const std::string level = r.difficulty == "-" ? "2 pl." : r.difficulty;
        std::string start = r.startSetup;
        const auto slash = start.find('/');
        if (slash != std::string::npos) {
            start = start.substr(0, slash) + " " + start.substr(slash + 1);
        }

        canvas.text(xDate, y, smallH, r.date.substr(5, 5), kMutedText);
        canvas.text(xWinner, y, smallH, winner, kText);
        canvas.text(xLevel, y, smallH, level, kText);
        canvas.text(xStart, y, smallH, start, kText);
        canvas.text(xMoves, y, smallH, std::to_string(r.moves), kMutedText, Align::Right);
        y -= lineH;
    }

    // --- paging --------------------------------------------------------------------------------
    canvas.text(0.f, navY, textH,
                std::to_string(records.size()) + " games  -  page " +
                std::to_string(historyPage_ + 1) + "/" + std::to_string(pages),
                kMutedText, Align::Center);
    if (historyPage_ > 0) {
        button(canvas, Rect{left + 0.21f, navY, 0.42f, 0.2f}, Action::HistoryNewer, "Newer",
               false, 0.08f);
    }
    if (historyPage_ < pages - 1) {
        button(canvas, Rect{right - 0.21f, navY, 0.42f, 0.2f}, Action::HistoryOlder, "Older",
               false, 0.08f);
    }
}

std::string App::statusText() const {
    if (auto winner = game_.winner()) {
        return std::string(*winner == Color::White ? "White" : "Black") + " wins";
    }
    if (game_.isAutomatedTurn()) {
        return "Computer is thinking...";
    }
    return std::string(game_.turn() == Color::White ? "White" : "Black") + " to move";
}

void App::drawGame(Canvas &canvas) {
    const float hw = canvas.halfWidth();
    const float hh = canvas.halfHeight();
    const bool flipped = opponent_ == Opponent::Computer && humanSide_ == Color::Black;

    // --- layout -------------------------------------------------------------------------------
    const float headerH = 0.5f + kTopInset - kMargin;
    Rect boardArea;
    Rect panelArea;
    if (canvas.isPortrait()) {
        const float side = 2.f * hw - 2.f * kMargin;
        boardArea = Rect{0.f, hh - headerH - side * 0.5f, side, side};
        const float panelTop = boardArea.bottom() - 0.12f;
        panelArea = Rect{0.f, (panelTop - hh) * 0.5f + kMargin * 0.5f, side, panelTop + hh - kMargin};
    } else {
        const float side = 2.f * hh - headerH - kMargin;
        boardArea = Rect{-hw + kMargin + side * 0.5f, -hh + kMargin + side * 0.5f, side, side};
        const float left = boardArea.right() + kMargin;
        panelArea = Rect{(left + hw - kMargin) * 0.5f, boardArea.cy, hw - kMargin - left, side};
    }

    // --- header: title, status, buttons ------------------------------------------------------
    const float headerY = hh - kTopInset - 0.11f;
    canvas.text(-hw + kMargin, headerY, 0.14f, "SHOGUN", kText);
    const float buttonW = 0.42f;
    button(canvas, Rect{hw - kMargin - buttonW * 0.5f, headerY, buttonW, 0.2f}, Action::Menu,
           "Menu", false, 0.08f);
    button(canvas, Rect{hw - kMargin - buttonW * 1.5f - 0.04f, headerY, buttonW, 0.2f},
           Action::NewGame, "New", false, 0.08f);

    const float statusY = headerY - 0.24f;
    canvas.quad(Rect{-hw + kMargin + 0.05f, statusY, 0.1f, 0.1f},
                game_.turn() == Color::White ? kWhiteTurn : kBlackTurn);
    canvas.text(-hw + kMargin + 0.17f, statusY, 0.085f, statusText(), kMutedText);

    // --- board ---------------------------------------------------------------------------------
    boardView_.draw(canvas, game_, boardArea, flipped);

    if (auto winner = game_.winner()) {
        canvas.quad(boardArea, kDim);
        canvas.text(boardArea.cx, boardArea.cy + 0.08f, 0.2f,
                    *winner == Color::White ? "White wins" : "Black wins", kText, Align::Center);
        canvas.text(boardArea.cx, boardArea.cy - 0.12f, 0.08f, "tap the board to play again",
                    kMutedText, Align::Center);
    }

    // --- move list -----------------------------------------------------------------------------
    drawMoveList(canvas, panelArea);
}

void App::drawMoveList(Canvas &canvas, const Rect &area) {
    canvas.quad(area, kPanel);

    const float textH = 0.085f;
    const float rowH = 0.13f;
    const float pad = 0.05f;
    const int rows = std::max(1, static_cast<int>((area.h - 2.f * pad) / rowH));

    const auto &history = game_.history();
    if (history.empty()) {
        canvas.text(area.cx, area.cy, textH, "no moves yet", kMutedText, Align::Center);
        return;
    }

    // one row per move pair; show the most recent rows that fit
    const int pairs = static_cast<int>((history.size() + 1) / 2);
    const int firstPair = std::max(0, pairs - rows);

    const float numberX = area.left() + pad;
    const float whiteX = numberX + Canvas::textWidth(textH, 4);
    const float blackX = whiteX + Canvas::textWidth(textH, 11);

    float y = area.top() - pad - textH * 0.5f;
    for (int pair = firstPair; pair < pairs; ++pair) {
        canvas.text(numberX, y, textH, std::to_string(pair + 1) + ".", kMutedText);

        const size_t whiteIndex = pair * 2;
        canvas.text(whiteX, y, textH, history[whiteIndex].notation(), kText);
        if (whiteIndex + 1 < history.size()) {
            canvas.text(blackX, y, textH, history[whiteIndex + 1].notation(), kText);
        }
        y -= rowH;
    }
}
