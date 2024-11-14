#ifndef MINO_H
#define MINO_H

#include <stdexcept>

#include "position.h"
#include "../setting.h"
#include "../util.h"

class Mino {
public:
    const char id;
    const char name;
    const Pos spawn_offset;
    Pos shape_set[4][4];
    PosChar kick_table_cw[4][5];
    PosChar kick_table_ccw[4][5];
    PosChar kick_table_180[4][6];

    static Mino I;
    static Mino O;
    static Mino T;
    static Mino Z;
    static Mino S;
    static Mino L;
    static Mino J;

    static Mino* ptr_I;
    static Mino* ptr_O;
    static Mino* ptr_T;
    static Mino* ptr_Z;
    static Mino* ptr_S;
    static Mino* ptr_L;
    static Mino* ptr_J;

    static Mino* fromID(char id) {
        switch (id) {
            case 2: return ptr_I;
            case 3: return ptr_O;
            case 4: return ptr_T;
            case 5: return ptr_Z;
            case 6: return ptr_S;
            case 7: return ptr_L;
            case 8: return ptr_J;
            default: throw std::invalid_argument("id must in range 2-8");
        }
    }
    static Mino* fromChar(char id) {
        switch (id) {
            case 'I': return ptr_I;
            case 'O': return ptr_O;
            case 'T': return ptr_T;
            case 'Z': return ptr_Z;
            case 'S': return ptr_S;
            case 'L': return ptr_L;
            case 'J': return ptr_J;
            default: throw std::invalid_argument("id must be mino name");
        }
    }
};

class MinoState {
public:
    Mino* mino;
    Pos position;
    char rotation;

    static inline MinoState spawnState(Mino* mino) {
        return {
                mino,
                Pos(4, BOARD_SPAWN_HEIGHT) + mino->spawn_offset,
                0
        };
    }

    static inline MinoState emptyState() {
        return {
                nullptr,
                {0, 0},
                0
        };
    }

    [[nodiscard]] inline bool is_empty_state() const { return this->mino == nullptr; }


    MinoState(Mino* mino, Pos position, char rotation) : mino(mino), position(std::move(position)), rotation(rotation) {}
    [[nodiscard]] MinoState copy() const {
        return {
            mino,
            position.copy(),
            rotation
        };
    }


    [[nodiscard]] vector<PosChar> cellPositions() const {
        vector<PosChar> result;
        for (int shapeNum = 0; shapeNum < 4; shapeNum++) {
            auto p = this->mino->shape_set[this->rotation][shapeNum] + this->position;
            result.emplace_back((int) p.x, (int) p.y);
        }
        return result;
    }
    [[nodiscard]] vector<Pos> cellPositionsE() const {
        vector<Pos> result;
        for (int shapeNum = 0; shapeNum < 4; shapeNum++) {
            auto p = this->mino->shape_set[this->rotation][shapeNum] + this->position;
            result.push_back(p);
        }
        return result;
    }

    inline bool operator==(const MinoState& other) const {
        return this->mino->id == other.mino->id && this->rotation == other.rotation && this->position == other.position;
    }


    [[nodiscard]] static MinoState convertHoleToState(vector<PosChar> hole_positions) {
        char right = 0;
        char top = 0;
        for (auto p: hole_positions) {
            if (p.first > right)
                right = p.first;
            if (p.second > top)
                top = p.second;
        }
        for (char id = 2; id < 9; id++) {
            Mino* mino = Mino::fromID(id);
            for (char rotation = 0; rotation < 4; rotation++) {
                MinoState state(mino, mino->spawn_offset, rotation);

                char stateRight = 0;
                char stateTop = 0;
                auto positions = state.cellPositions();
                for (auto p: positions) {
                    if (p.first > stateRight)
                        stateRight = p.first;
                    if (p.second > stateTop)
                        stateTop = p.second;
                }

                char deltaX = (char) (right - stateRight);
                char deltaY = (char) (top - stateTop);
                bool find = true;
                for (auto p: positions) {
                    PosChar p_moved = {p.first + deltaX, p.second + deltaY};
                    if (std::find(hole_positions.begin(), hole_positions.end(), p_moved) == hole_positions.end()) {
                        find = false;
                        break;
                    }
                }

                if (find) {
                    state.position.x += (float) deltaX;
                    state.position.y += (float) deltaY;
                    return state;
                }
            }

        }
        throw std::logic_error("Failed to convert at convertHoleToState");
    }
};

Mino Mino::I = {2, 'I', {.5, .5},
     {
         {{-1.5, .5}, {-.5, .5}, {.5, .5}, {1.5, .5}},
         {{.5, 1.5}, {.5, .5}, {.5, -.5}, {.5, -1.5}},
         {{-1.5, -.5}, {-.5, -.5}, {.5, -.5}, {1.5, -.5}},
         {{-.5, 1.5}, {-.5, .5}, {-.5, -.5}, {-.5, -1.5}}
     },
     {
         {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
         {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
         {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
         {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}
     },
     {
         {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
         {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},
         {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
         {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}}
     },
     { //TODO I-kick
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}
     }
};

Mino Mino::O = {3, 'O',{0, 0},// {.5, .5},
//      {
//          {{.5, .5}, {-.5, .5}, {-.5, -.5}, {.5, -.5}},
//          {{.5, .5}, {-.5, .5}, {-.5, -.5}, {.5, -.5}},
//          {{.5, .5}, {-.5, .5}, {-.5, -.5}, {.5, -.5}},
//          {{.5, .5}, {-.5, .5}, {-.5, -.5}, {.5, -.5}}
//      },
//      {
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}
//      },
//      {
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//      },
//      {
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
//      }
      {
          {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
          {{0, 0}, {1, 0}, {0, -1}, {1, -1}},
          {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}},
          {{0, 0}, {-1, 0}, {0, 1}, {-1, 1}}
      },
      {
          {{0, 1},{0, 1},{0, 1},{0, 1},{0, 1}},
          {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}},
          {{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}},
          {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}
      },
      {
          {{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}},
          {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},
          {{0, 1}, {0, 1},  {0, 1}, {0, 1}, {0, 1}},
          {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}
      },
      {
          {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
          {{1, -1}, {1, -1}, {1, -1}, {1, -1}, {1, -1}, {1, -1}},
          {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
          {{-1, 1}, {-1, 1}, {-1, 1}, {-1, 1}, {-1, 1}, {-1, -1}}
      }
};
// 0->R, R->2, 2->L, L->0
#define WALL_KICK_TZSLJ_CW { \
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},\
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},\
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},\
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},\
}
// L->0, 0->R, R->2, 2->L
#define WALL_KICK_TZSLJ_CCW {\
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},\
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},\
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},\
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},\
}
#define WALL_KICK_TZSLJ_180 {\
    {{0, 0}, {0, 1}, {1, 1}, {-1, 1}, {1, 0}, {-1,0}},\
    {{0, 0}, {0, 1}, {-1, -1}, {1, -1}, {-1, 0}, {1,0}},\
    {{0, 0}, {1, 0}, {1, 2}, {1, 1}, {0, 2}, {0,1}},\
    {{0, 0}, {-1, 0}, {-1, 2}, {-1, 1}, {0, 2}, {0,1}},\
}
Mino Mino::T = {4, 'T', {0, 0},
    {
          {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},
          {{0, 1}, {0, 0}, {0, -1}, {1, 0}},
          {{-1, 0}, {0, 0}, {1, 0}, {0, -1}},
          {{0, 1}, {0, 0}, {0, -1}, {-1, 0}},
      },
                WALL_KICK_TZSLJ_CW, WALL_KICK_TZSLJ_CCW, WALL_KICK_TZSLJ_180
};
Mino Mino::Z = {5, 'Z', {0, 0},
    {
        {{-1,1}, {0,1}, {0,0}, {1,0}},
        {{1,1}, {1,0}, {0,0}, {0,-1}},
        {{-1,0}, {0,0}, {0,-1}, {1,-1}},
        {{0,1}, {0,0}, {-1,0}, {-1,-1}},
    },
                WALL_KICK_TZSLJ_CW, WALL_KICK_TZSLJ_CCW, WALL_KICK_TZSLJ_180
};
Mino Mino::S = {6, 'S', {0, 0},
    {

        {{-1,0}, {0,0}, {0,1}, {1,1}},
        {{0,1}, {0,0}, {1,0}, {1,-1}},
        {{-1,-1}, {0,-1}, {0,0}, {1,0}},
        {{-1,1}, {-1,0}, {0,0}, {0,-1}},
    },
                WALL_KICK_TZSLJ_CW, WALL_KICK_TZSLJ_CCW, WALL_KICK_TZSLJ_180
};
Mino Mino::L = {7, 'L', {0, 0},
    {
            {{-1,0}, {0,0}, {1,0}, {1,1}},
            {{0,1}, {0,0}, {0,-1}, {1,-1}},
            {{-1,-1}, {-1,0}, {0,0}, {1,0}},
            {{-1,1}, {0,1}, {0,0}, {0,-1}},
      },
                WALL_KICK_TZSLJ_CW, WALL_KICK_TZSLJ_CCW, WALL_KICK_TZSLJ_180
};
Mino Mino::J = {8, 'J', {0, 0},
    {
        {{-1,1}, {-1,0}, {0,0}, {1,0}},
        {{0,1}, {0,0}, {0,-1}, {1,1}},
        {{-1,0}, {0,0}, {1,0}, {1,-1}},
        {{0,1}, {0,0}, {0,-1}, {-1,-1}},
    },
                WALL_KICK_TZSLJ_CW, WALL_KICK_TZSLJ_CCW, WALL_KICK_TZSLJ_180
};

Mino* Mino::ptr_I = &I;
Mino* Mino::ptr_O = &O;
Mino* Mino::ptr_T = &T;
Mino* Mino::ptr_S = &S;
Mino* Mino::ptr_Z = &Z;
Mino* Mino::ptr_L = &L;
Mino* Mino::ptr_J = &J;
#endif //MINO_H