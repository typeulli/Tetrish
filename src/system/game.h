#ifndef GAME_H
#define GAME_H

#include "../util.h"
#include "mino.h"
#include "../setting.h"

#include <fmt/core.h>

class Cell {
    public:
        const static char Empty = 0;
        const static char Garbage = 1;
        const static char I = 2;
        const static char O = 3;
        const static char T = 4;
        const static char Z = 5;
        const static char S = 6;
        const static char L = 7;
        const static char J = 8;

        constexpr const static string cell_char = "\u2588\u2588\033[0m";

        static string CellColor(char id) {
            switch (id) {
                case Cell::Empty: return "";
                case Cell::Garbage: return "\033[38;5;8m";
                case Cell::I: return "\033[38;5;14m";
                case Cell::O: return "\033[38;5;3m";
                case Cell::T: return "\033[38;5;13m";
                case Cell::Z: return "\033[38;5;9m";
                case Cell::S: return "\033[38;5;10m";
                case Cell::L:  return "\033[38;5;208m";
                case Cell::J: return "\033[38;5;12m";
                default: return "";
            }
        }
        static string CellString(char id) {
            if (id == Cell::Empty) return "  ";
            if (0 < id && id < 9) return CellColor(id) + cell_char;
            return "??";
        }
};


class MinoGenerator {
public:
    virtual Mino* Pop() = 0;

    virtual Mino* Lookup(char n) = 0;
    virtual char MaxLookUp() = 0;

    virtual MinoGenerator* copy() = 0;
};

class MinoGenerator_Fake : public MinoGenerator {
public:
    vector<char> items;
    char lookup;
    explicit MinoGenerator_Fake(char lookup) {
        this->lookup = lookup;
    }

    Mino* Pop() override {
        auto begin = items.begin();
        auto item = *begin;
        items.erase(begin);
        return Mino::fromID(item);
    }
    Mino* Lookup(char n) override {
        if (n >= lookup) {
            throw std::invalid_argument("n must less than " + to_string(lookup));
        }
        return Mino::fromID(items[n]);
    }
    inline char MaxLookUp() override { return std::min((char) items.size(), this->lookup); }
    MinoGenerator* copy() override {
        auto newGenerator = new MinoGenerator_Fake(this->lookup);
        newGenerator->items.clear();
        for (auto item : items)
            newGenerator->items.push_back(item);
        return newGenerator;
    }
};

class MinoGenerator_7Bag : public MinoGenerator {
public:
    vector<char> items;
    char lookup;

    void FillItem() {
        if (items.size() >= lookup) return;

        char bag[7] = {2, 3, 4, 5, 6, 7, 8};

        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(std::begin(bag), std::end(bag), gen);

        for (auto id: bag) items.push_back(id);
    }

    explicit MinoGenerator_7Bag(char lookup=5) {
        this->lookup = lookup;
        FillItem();
    }


    Mino* Pop() override {
        auto begin = items.begin();
        auto item = *begin;
        items.erase(begin);

        FillItem();

        return Mino::fromID(item);
    }
    Mino* Lookup(char n) override {
        if (n >= lookup) {
            throw std::invalid_argument("n must less than " + to_string(lookup));
        }
        return Mino::fromID(items[n]);
    }
    inline char MaxLookUp() override { return this->lookup; }
    MinoGenerator* copy() override {
        auto newGenerator = new MinoGenerator_7Bag(this->lookup);
        newGenerator->items.clear();
        for (auto item : items)
            newGenerator->items.push_back(item);
        return newGenerator;
    }
};

typedef unsigned int LineList;
inline LineList withLine(LineList previous, char adding) { return previous | (1<<adding); }
inline LineList isLineContained(LineList previous, char check) { return previous & (1<<check); }
inline char countLine(LineList target) { return (char) __builtin_popcount(target); }
inline LineList combineLine(LineList list1, LineList list2) { return list1 | list2; }
struct ClearInfo {
    unsigned int cleared_line;
};

class Game {
public:
    MinoGenerator* generator;
    MinoState current = MinoState::emptyState();
    Mino* hold_mino;
    bool hold_available = true;
    char board[BOARD_HEIGHT][10]{};
    explicit Game(MinoGenerator* generator) {
        for (auto & i : board) {
            for (char & j : i) {
                j = 0;
            }
        }
        this->generator = generator;
        this->current = this->spawn();
        this->hold_mino = nullptr;
    }

    ~Game() {
        delete generator;
    }

    Game* copy() {
        Game* newGame = new Game(this->generator->copy());
        newGame->current = this->current.copy();
        newGame->hold_mino = this->hold_mino;
        std::copy(&this->board[0][0], &this->board[0][0] + 10 * BOARD_HEIGHT, &newGame->board[0][0]);


        return newGame;
    }

    bool addMassLine(char target) {
         for (char x = 0; x < 10; ++x) {
            if (this->board[BOARD_HEIGHT - 1][x]) {
                return false;
            }
         }
//         for (char y = BOARD_HEIGHT - 2; y >= 0; --y) {
//             std::copy(this->board[y], this->board[y] + 10, this->board[y+1]);
//         }
        std::copy_backward(&this->board[0][0], &this->board[BOARD_HEIGHT - 1][0], &this->board[BOARD_HEIGHT][0]);

         for (char x=0; x < 10; ++x) {
             this->board[0][x] = Cell::Garbage;
         }
        this->board[0][target] = Cell::Empty;
        return true;
    };

    [[nodiscard]] inline MinoState spawn() const {
        return MinoState::spawnState(generator->Pop());
    }

    [[nodiscard]] bool isEmpty() const {
        for (char x = 0; x < 10; ++x) {
            for (auto &y: this->board) {
                if (y[x]) return false;
            }
        }
        return true;
    }

    // TODO make move function here
    bool hold() {
        if (!hold_available) return false;
        hold_available = false;
        auto temp = hold_mino;
        if (temp == nullptr) temp = generator->Pop();
        hold_mino = this->current.mino;
        this->current = MinoState::spawnState(temp);
        return true;
    }

    inline bool isAvailableState(MinoState state) {
        for (int shapeNum = 0; shapeNum < 4; ++shapeNum) {
            auto p= state.mino->shape_set[state.rotation][shapeNum] + state.position;
            if (   ((int) p.x) < 0 || (int) p.x > 9
                || ((int) p.y) < 0 || (int) p.y > BOARD_HEIGHT-1
                ) return false;
            if (this->board[(int)p.y][(int)p.x]) return false;
        }
        return true;
    }

    bool move(Pos direction) {
        auto nextState = this->current.copy();
        nextState.position += direction;
        if (not isAvailableState(nextState)) return false;
        this->current = nextState;
        return true;
    }

    inline bool moveLeft() {
        return move({-1, 0});
    };

    inline bool moveRight() {
        return move({1, 0});
    };

    inline bool moveDown() {
        return move({0, -1});
    };
    template <std::size_t kicktable_length>
    bool rotate(char rotate, PosChar kicktable[4][kicktable_length]){
        auto previousRotation = this->current.rotation;
        auto kickset = kicktable[previousRotation];
        auto rotateState = this->current.copy();
        rotateState.rotation = (char) ((previousRotation + rotate + 4) % 4); // Plus 4 when rotate < 0 (ensure that -4 <= rotate)
        for (int testNum = 0; testNum < kicktable_length; ++testNum) {
            auto kicktest = kickset[testNum];
            auto nextState = rotateState.copy();
            nextState.position.x += kicktest.first;
            nextState.position.y += kicktest.second;
            if (isAvailableState(nextState)) {
                this->current = nextState;
                return true;
            }
        }
        return false;
    }

    inline bool rotateCW() {
        return rotate(
            +1, this->current.mino->kick_table_cw
        );
    }

    inline bool rotateCCW() {
        return rotate(
                -1, this->current.mino->kick_table_ccw
        );
    }

    inline bool rotate180() {
        return rotate(
                +2, this->current.mino->kick_table_180
        );
    }
    ClearInfo acceptClear() {
        ClearInfo info{};


        char board[BOARD_HEIGHT][10] = {0};
        char next_stack = 0;


        for (char y = 0; y < BOARD_HEIGHT; ++y) {
            bool clear_this_line = true;
            for (auto cell : this->board[y]) {
                if (!cell) clear_this_line = false;
            }
            if (clear_this_line)
                info.cleared_line = withLine(info.cleared_line, y);
            else
                std::copy(this->board[y], this->board[y] + 10, board[next_stack++]);
        }

        std::memcpy(this->board, board, sizeof(board));

        return info;
    }

    void drop() {
        while (this->isAvailableState(this->current)) {
            --this->current.position.y;
        }
        ++this->current.position.y;
        put();
    }

    void put() {
        auto mino = this->current.mino;
        for (int shapeNum = 0; shapeNum < 4; ++shapeNum) {
            auto p = mino->shape_set[this->current.rotation][shapeNum] + this->current.position;
            this->board[(int)p.y][(int)p.x] = mino->id;
        }
        this->current = this->spawn();
        hold_available = true;
    }

    string view() {
        char copied_board[BOARD_HEIGHT][10]{};
        std::copy(&this->board[0][0], &this->board[0][0]+BOARD_HEIGHT*10, &copied_board[0][0]);

        for (auto currentMinoCell : this->current.cellPositions()) {

            copied_board[currentMinoCell.second][currentMinoCell.first] = this->current.mino->id;
        }

        string result;
        for (int linenum = BOARD_VISIBLE_HEIGHT-1; linenum >= 0 ; linenum--) {
            auto line = copied_board[linenum];

            for (char cellnum = 0; cellnum < 10; ++cellnum) {
                auto cell = line[cellnum];
                result += Cell::CellString(cell);
            }
            if (linenum != 0) result += '\n';
        }
        return result;
    }

    void build(const std::string& source, char delimiter='\n') {

        auto lines = split_string_by_char(source, delimiter);
        auto iter = lines.end();

        short target_line_num = 0;

        while (iter != lines.begin()) {
            --iter;

            string line = *iter;
            if (line.length() != 10) {
                throw std::invalid_argument("Each line of 'source' must has 10 cols");
            }

            auto chars = line.c_str();
            for (int col = 0; col < 10; ++col) {
                this->board[target_line_num][col] = (chars[col] == ' ')? 0 : 1;
            }
            ++target_line_num;
        }
    }
};

#endif //GAME_H