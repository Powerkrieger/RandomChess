#ifndef RANDOMCHESS_APP_H
#define RANDOMCHESS_APP_H

#include <chrono>
#include <future>
#include <optional>
#include <string>
#include <vector>

#include "BoardView.h"
#include "Canvas.h"
#include "Stats.h"
#include "chess/Game.h"
#include "chess/ShogunAi.h"

/*!
 * The whole application above the rendering layer: which screen is shown, the settings chosen
 * in the menu, the running game and the computer opponent. The renderer calls tap() with world
 * coordinates, update() once per frame, and draw() to get the frame's contents.
 *
 * This object outlives the renderer (and its GL context), so it must not hold GL resources.
 */
class App {
public:
    enum class Opponent { Computer, Human };

    enum class Difficulty { Easy, Normal, Hard };

    App();

    /*! Directory for persistent files (the results history). Loads the history from there. */
    void setDataDir(const std::string &dir);

    void tap(float x, float y);

    /*! Advances non-input state, currently: lets the computer opponent move. */
    void update();

    void draw(Canvas &canvas);

private:
    enum class Screen { Menu, Game, History };

    /*! Identifies what a tappable rectangle does. */
    enum class Action {
        PlayWhite, PlayBlack, TwoPlayers,
        Easy, Normal, Hard,
        Start, Menu, NewGame, History
    };

    struct Button {
        Rect rect;
        Action action;
        std::string label;
        bool selected;
    };

    void newGame();

    void perform(Action action);

    /*! Adds the finished game to the results history (once) and saves it. */
    void recordResultIfOver();

    std::string playerName(chess::Color side) const;

    std::string difficultyName() const;

    void drawMenu(Canvas &canvas);

    void drawGame(Canvas &canvas);

    void drawHistory(Canvas &canvas);

    /*! Draws a button and registers it for hit-testing until the next draw(). */
    void button(Canvas &canvas, const Rect &rect, Action action, const std::string &label,
                bool selected = false, float textHeight = 0.09f);

    void drawMoveList(Canvas &canvas, const Rect &area);

    std::string statusText() const;

    static int searchDepth(Difficulty difficulty);

    // settings
    chess::Color humanSide_ = chess::Color::White;
    Opponent opponent_ = Opponent::Computer;
    Difficulty difficulty_ = Difficulty::Normal;

    Screen screen_ = Screen::Menu;
    chess::Game game_;
    BoardView boardView_;
    std::vector<Button> buttons_;

    // results history
    std::string dataDir_;
    Stats stats_;
    std::string startSetup_;     // starting numbers of the running game
    bool resultRecorded_ = false;

    // computer opponent
    chess::ShogunAi ai_;
    std::future<std::optional<chess::Move>> pendingAiMove_;
    std::chrono::steady_clock::time_point aiStartedAt_;
};

#endif //RANDOMCHESS_APP_H
