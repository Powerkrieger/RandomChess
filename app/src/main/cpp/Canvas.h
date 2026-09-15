#ifndef RANDOMCHESS_CANVAS_H
#define RANDOMCHESS_CANVAS_H

#include <memory>
#include <string>
#include <vector>

#include "Model.h"
#include "TextureAsset.h"
#include "chess/Piece.h"

/*! An axis-aligned rectangle in world units, given by its centre and size. */
struct Rect {
    float cx, cy, w, h;

    bool contains(float x, float y) const {
        return x >= cx - w * 0.5f && x <= cx + w * 0.5f && y >= cy - h * 0.5f && y <= cy + h * 0.5f;
    }

    float left() const { return cx - w * 0.5f; }

    float right() const { return cx + w * 0.5f; }

    float top() const { return cy + h * 0.5f; }

    float bottom() const { return cy - h * 0.5f; }
};

/*! All textures the canvas draws with. */
struct Textures {
    std::shared_ptr<TextureAsset> pieceSheet;  // chess sprites: 6 columns (PieceType order) x 2 rows
    std::shared_ptr<TextureAsset> stones;      // white soldier, white king, black soldier, black king
    std::shared_ptr<TextureAsset> digits;      // 4 columns "1".."4"; row 0 light, row 1 dark
    std::shared_ptr<TextureAsset> font;        // ASCII 32..127, 16 columns x 6 rows, monospace
    std::shared_ptr<TextureAsset> white;       // 1x1 white, for flat-coloured quads
};

enum class Align { Left, Center, Right };

/*!
 * Immediate-mode 2D drawing: the app calls clear() and then the draw functions once per frame,
 * the renderer draws the resulting models. World space is the orthographic projection: origin in
 * the screen centre, y up, extents given by setViewport().
 */
class Canvas {
public:
    explicit Canvas(Textures textures);

    void setViewport(float halfWidth, float halfHeight, int pixelWidth, int pixelHeight);

    float halfWidth() const { return halfWidth_; }

    float halfHeight() const { return halfHeight_; }

    bool isPortrait() const { return halfHeight_ >= halfWidth_; }

    /*! Converts a touch position (pixels, origin top-left) to world coordinates. */
    Vector2 worldFromPixel(float pixelX, float pixelY) const;

    void clear() { models_.clear(); }

    const std::vector<Model> &models() const { return models_; }

    void quad(const Rect &rect, Vector4 tint);

    void sprite(const Rect &rect, const std::shared_ptr<TextureAsset> &texture,
                float u0, float v0, float u1, float v1, Vector4 tint = {1.f, 1.f, 1.f, 1.f});

    /*!
     * Draws @a text with glyphs of the given height. (x, y) is the anchor on the text's vertical
     * centre line; which end of the text it anchors depends on @a align.
     */
    void text(float x, float y, float height, const std::string &text, Vector4 tint,
              Align align = Align::Left);

    /*! Width the given number of glyphs take at the given height. */
    static float textWidth(float height, size_t glyphs);

    /*!
     * Draws a piece centred on (cx, cy) in a square of @a size. Pieces with a power (Shogun) are
     * drawn as a stone with the number in the middle, others as a chess sprite.
     */
    void piece(float cx, float cy, float size, const chess::Piece &piece);

private:
    Textures textures_;
    std::vector<Model> models_;

    float halfWidth_ = 1.f;
    float halfHeight_ = 1.f;
    int pixelWidth_ = 1;
    int pixelHeight_ = 1;
};

#endif //RANDOMCHESS_CANVAS_H
