#include "Canvas.h"

#include <algorithm>

namespace {

constexpr Vector4 kOpaque{1.f, 1.f, 1.f, 1.f};

// pieces.png
constexpr int kSheetColumns = 6;
constexpr int kSheetRows = 2;

// stones.png / digits.png
constexpr int kStoneColumns = 4;
constexpr float kStoneScale = 0.92f;
constexpr int kDigitColumns = 4;
constexpr int kDigitRows = 2;
constexpr float kDigitScale = 0.5f;

// font.png: 16x6 cells of 32x48 pixels starting at ASCII 32
constexpr int kFontColumns = 16;
constexpr int kFontRows = 6;
constexpr int kFontFirstChar = 32;
constexpr float kGlyphAspect = 32.f / 48.f;  // cell width / cell height
constexpr float kGlyphAdvance = 0.46f;        // advance per glyph relative to the cell height (0.6 em, em = 36/48 cell)

} // namespace

Canvas::Canvas(Textures textures) : textures_(std::move(textures)) {}

void Canvas::setViewport(float halfWidth, float halfHeight, int pixelWidth, int pixelHeight) {
    halfWidth_ = halfWidth;
    halfHeight_ = halfHeight;
    pixelWidth_ = std::max(pixelWidth, 1);
    pixelHeight_ = std::max(pixelHeight, 1);
}

Vector2 Canvas::worldFromPixel(float pixelX, float pixelY) const {
    // pixels (origin top-left, y down) -> world (origin centre, y up)
    return Vector2{(pixelX / pixelWidth_ - 0.5f) * 2.f * halfWidth_,
                   (0.5f - pixelY / pixelHeight_) * 2.f * halfHeight_};
}

void Canvas::quad(const Rect &rect, Vector4 tint) {
    sprite(rect, textures_.white, 0.f, 0.f, 1.f, 1.f, tint);
}

void Canvas::sprite(const Rect &rect, const std::shared_ptr<TextureAsset> &texture,
                    float u0, float v0, float u1, float v1, Vector4 tint) {
    const float halfW = rect.w * 0.5f;
    const float halfH = rect.h * 0.5f;

    /*
     * Texture v grows downwards (row 0 of the image is v = 0), world y grows upwards:
     * 0 --- 1      (top-left, top-right)
     * | \   |
     * 3 --- 2      (bottom-left, bottom-right)
     */
    std::vector<Vertex> vertices = {
            Vertex(Vector3{rect.cx - halfW, rect.cy + halfH, 0}, Vector2{u0, v0}),
            Vertex(Vector3{rect.cx + halfW, rect.cy + halfH, 0}, Vector2{u1, v0}),
            Vertex(Vector3{rect.cx + halfW, rect.cy - halfH, 0}, Vector2{u1, v1}),
            Vertex(Vector3{rect.cx - halfW, rect.cy - halfH, 0}, Vector2{u0, v1}),
    };
    std::vector<Index> indices = {0, 1, 2, 0, 2, 3};

    models_.emplace_back(std::move(vertices), std::move(indices), texture, tint);
}

float Canvas::textWidth(float height, size_t glyphs) {
    return glyphs == 0 ? 0.f : height * (kGlyphAdvance * (glyphs - 1) + kGlyphAspect);
}

void Canvas::text(float x, float y, float height, const std::string &text, Vector4 tint,
                  Align align) {
    const float width = textWidth(height, text.size());
    float penX = x;
    if (align == Align::Center) {
        penX -= width * 0.5f;
    } else if (align == Align::Right) {
        penX -= width;
    }

    const float glyphW = height * kGlyphAspect;
    for (char c: text) {
        int code = static_cast<unsigned char>(c) - kFontFirstChar;
        if (code < 0 || code >= kFontColumns * kFontRows) {
            code = '?' - kFontFirstChar;
        }
        const int column = code % kFontColumns;
        const int row = code / kFontColumns;
        sprite(Rect{penX + glyphW * 0.5f, y, glyphW, height}, textures_.font,
               static_cast<float>(column) / kFontColumns, static_cast<float>(row) / kFontRows,
               static_cast<float>(column + 1) / kFontColumns,
               static_cast<float>(row + 1) / kFontRows, tint);
        penX += height * kGlyphAdvance;
    }
}

void Canvas::piece(float cx, float cy, float size, const chess::Piece &piece) {
    const bool isWhite = piece.color() == chess::Color::White;
    const int power = piece.power();

    if (power < 1 || power > kDigitColumns) {
        // classic chess piece: sprite from the sheet
        const int column = static_cast<int>(piece.type());
        const int row = isWhite ? 0 : 1;
        sprite(Rect{cx, cy, size, size}, textures_.pieceSheet,
               static_cast<float>(column) / kSheetColumns,
               static_cast<float>(row) / kSheetRows,
               static_cast<float>(column + 1) / kSheetColumns,
               static_cast<float>(row + 1) / kSheetRows);
        return;
    }

    // Shogun stone with the number in the middle
    const bool isKing = piece.type() == chess::PieceType::King;
    const int stone = (isWhite ? 0 : 2) + (isKing ? 1 : 0);
    sprite(Rect{cx, cy, size * kStoneScale, size * kStoneScale}, textures_.stones,
           static_cast<float>(stone) / kStoneColumns, 0.f,
           static_cast<float>(stone + 1) / kStoneColumns, 1.f);

    const int digitRow = isWhite ? 1 : 0;  // dark digit on a light stone and vice versa
    sprite(Rect{cx, cy, size * kDigitScale, size * kDigitScale}, textures_.digits,
           static_cast<float>(power - 1) / kDigitColumns,
           static_cast<float>(digitRow) / kDigitRows,
           static_cast<float>(power) / kDigitColumns,
           static_cast<float>(digitRow + 1) / kDigitRows);
}
