#ifndef RANDOMCHESS_CHESS_GAME_H
#define RANDOMCHESS_CHESS_GAME_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Board.h"
#include "Rules.h"
#include "Types.h"

namespace chess {

/*!
 * Turn handling and the piece-selection state machine. The UI only ever calls tap(square) and
 * then reads back the state (board, selected square, legal moves, whose turn, winner) to draw
 * itself. Variant-specific behaviour is delegated to the Rules object.
 */
class Game {
public:
    enum class TapResult { Ignored, Selected, Deselected, Moved, Restarted };

    /*! One played move, as remembered for the history list. */
    struct Record {
        Move move;
        Color side;
        PieceType piece;
        int powerBefore;                 // 0 for variants without numbers
        int powerAfter;
        std::optional<PieceType> captured;

        /*! Short notation such as "a1-a5>3" or "b8xa6>4" (the number after '>' is the re-roll). */
        std::string notation() const;
    };

    /*! Defaults to Shogun rules. */
    Game();

    explicit Game(std::unique_ptr<Rules> rules);

    /*! Starting position for the current rules, white to move, nothing selected. */
    void reset();

    /*!
     * Handles a tap on a square:
     *  - game over                               -> restart
     *  - nothing selected + own piece tapped     -> select it
     *  - selected + legal target tapped          -> move (capturing if occupied), switch turn
     *  - selected + the same square tapped again -> deselect
     *  - selected + another own piece tapped     -> switch selection to that piece
     *  - anything else                           -> deselect
     */
    TapResult tap(Square square);

    /*! Selects the piece on @a square if it belongs to the side to move. */
    bool select(Square square);

    void deselect();

    /*! Moves the selected piece to @a to if that is one of its legal moves. */
    bool tryMove(Square to);

    /*! Plays a full move for the side to move (used by the AI). Returns false if illegal. */
    bool play(const Move &move);

    /*!
     * Marks a side as computer-controlled: taps are ignored while it is that side's turn, so
     * the human can't move the AI's pieces while it is "thinking".
     */
    void setAutomatedSide(std::optional<Color> side) { automatedSide_ = side; }

    std::optional<Color> automatedSide() const { return automatedSide_; }

    /*! True when it's the automated side's turn and the game isn't over. */
    bool isAutomatedTurn() const {
        return automatedSide_ && *automatedSide_ == turn_ && !isOver();
    }

    const Board &board() const { return board_; }

    Color turn() const { return turn_; }

    std::optional<Square> selected() const { return selected_; }

    /*! Legal moves of the currently selected piece (empty if nothing is selected). */
    const std::vector<Move> &legalMoves() const { return legalMoves_; }

    /*! The legal move of the selected piece that ends on @a to, if there is one. */
    const Move *legalMoveTo(Square to) const;

    std::optional<Move> lastMove() const { return lastMove_; }

    const std::vector<Record> &history() const { return history_; }

    /*! Set once the game is over. */
    std::optional<Color> winner() const { return winner_; }

    bool isOver() const { return winner_.has_value(); }

private:
    void endTurn();

    std::unique_ptr<Rules> rules_;
    Board board_;
    Color turn_ = Color::White;
    std::optional<Square> selected_;
    std::vector<Move> legalMoves_;
    std::optional<Move> lastMove_;
    std::vector<Record> history_;
    std::optional<Color> winner_;
    std::optional<Color> automatedSide_;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_GAME_H
