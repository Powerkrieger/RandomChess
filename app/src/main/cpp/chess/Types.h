#ifndef RANDOMCHESS_CHESS_TYPES_H
#define RANDOMCHESS_CHESS_TYPES_H

#include <cstdint>
#include <string>

namespace chess {

constexpr int kBoardSize = 8;

enum class Color : uint8_t { White, Black };

inline Color opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

/*!
 * The order here matches the column order in assets/pieces.png so the renderer can index the
 * sprite sheet directly with static_cast<int>(type).
 */
enum class PieceType : uint8_t { King, Queen, Rook, Bishop, Knight, Pawn };

/*!
 * A relative displacement on the board, e.g. {1, 0} is "one file to the right".
 */
struct Offset {
    int file;
    int rank;
};

/*!
 * A location on the board. file 0..7 = a..h, rank 0..7 = 1..8 (rank 0 is white's home rank).
 */
struct Square {
    int file;
    int rank;

    constexpr bool operator==(const Square &other) const {
        return file == other.file && rank == other.rank;
    }

    constexpr bool operator!=(const Square &other) const { return !(*this == other); }

    constexpr bool isValid() const {
        return file >= 0 && file < kBoardSize && rank >= 0 && rank < kBoardSize;
    }

    constexpr Square operator+(const Offset &offset) const {
        return Square{file + offset.file, rank + offset.rank};
    }

    /*! Algebraic name such as "e4", useful for logging. */
    std::string name() const {
        return std::string(1, char('a' + file)) + std::to_string(rank + 1);
    }
};

struct Move {
    Square from;
    Square to;
    bool isCapture;
};

} // namespace chess

#endif //RANDOMCHESS_CHESS_TYPES_H
