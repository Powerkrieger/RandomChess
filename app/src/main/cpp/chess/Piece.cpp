#include "Piece.h"

#include "Board.h"

namespace chess {

void Piece::slide(const Board &board, std::initializer_list<Offset> directions,
                  std::vector<Move> &out) const {
    for (const auto &direction: directions) {
        Square target = square_ + direction;
        while (target.isValid() && tryAdd(board, target, out)) {
            target = target + direction;
        }
    }
}

void Piece::step(const Board &board, std::initializer_list<Offset> offsets,
                 std::vector<Move> &out) const {
    for (const auto &offset: offsets) {
        tryAdd(board, square_ + offset, out);
    }
}

void Piece::walk(const Board &board, int length, std::vector<Move> &out) const {
    static constexpr Offset kDirections[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    auto alreadyAdded = [&out](Square to) {
        for (const auto &move: out) {
            if (move.to == to) {
                return true;
            }
        }
        return false;
    };

    for (const auto &direction: kDirections) {
        Square corner = square_;

        // Go straight for k squares; at each corner (k < length) optionally turn 90 degrees
        // and complete the remaining length - k squares sideways.
        for (int k = 1; k <= length; ++k) {
            corner = corner + direction;
            if (!corner.isValid()) {
                break;
            }

            if (k == length) {
                if (!alreadyAdded(corner)) {
                    tryAdd(board, corner, out);
                }
                break;
            }

            if (!board.isEmpty(corner)) {
                // blocked: can neither continue straight nor turn here
                break;
            }

            const Offset sideways[] = {{direction.rank, direction.file},
                                       {-direction.rank, -direction.file}};
            for (const auto &side: sideways) {
                Square target = corner;
                bool pathClear = true;
                for (int j = 1; j <= length - k; ++j) {
                    target = target + side;
                    if (!target.isValid() || (j < length - k && !board.isEmpty(target))) {
                        pathClear = false;
                        break;
                    }
                }
                if (pathClear && !alreadyAdded(target)) {
                    tryAdd(board, target, out);
                }
            }
        }
    }
}

bool Piece::tryAdd(const Board &board, Square to, std::vector<Move> &out) const {
    if (!to.isValid()) {
        return false;
    }

    const Piece *occupant = board.at(to);
    if (occupant == nullptr) {
        out.push_back(Move{square_, to, false});
        return true;
    }

    if (isEnemyOf(*occupant)) {
        out.push_back(Move{square_, to, true});
    }
    return false;
}

} // namespace chess
