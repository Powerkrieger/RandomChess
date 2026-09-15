#ifndef RANDOMCHESS_BOARDVIEW_H
#define RANDOMCHESS_BOARDVIEW_H

#include <optional>

#include "Canvas.h"
#include "chess/Game.h"

/*!
 * Draws a chess::Game's board into a Rect and translates taps back into squares. It owns no
 * game state, only the layout of the last draw.
 */
class BoardView {
public:
    /*!
     * @param area the square area to draw into
     * @param flipped false: white's home rank at the bottom; true: black's
     */
    void draw(Canvas &canvas, const chess::Game &game, const Rect &area, bool flipped);

    /*! The square under a world position, or nullopt if outside the board. */
    std::optional<chess::Square> squareAt(float x, float y) const;

private:
    float centerX(chess::Square square) const;

    float centerY(chess::Square square) const;

    Rect area_{0.f, 0.f, 1.f, 1.f};
    float squareSize_ = 0.125f;
    bool flipped_ = false;
};

#endif //RANDOMCHESS_BOARDVIEW_H
