#ifndef ENGINE_H
#define ENGINE_H
#include <fmt/core.h>
#include <iostream>
#include <functional>
#include <format>

#include "../system/game.h"
#include "stack.h"



namespace EngineData {

    namespace Opener {
        struct OpenerBook {
            vector<pair<Mino*, Mino*>> condition; // A > B to use
            vector<MinoState> data;
            vector<OpenerBook> nextBag;
            bool breakable = false;
        };

        const OpenerBook Opener_SBZ_36 = {
                {{Mino::ptr_L, Mino::ptr_O}},
                {
                    MinoState(Mino::ptr_L, {1, 0}, 0),
                    MinoState(Mino::ptr_O, {0, 1}, 0),
                    MinoState(Mino::ptr_L, {1, 3}, 2),
                    MinoState(Mino::ptr_I, {4.5, 1.5}, 1),
                    MinoState(Mino::ptr_T, {7, 0}, 0),
                    MinoState(Mino::ptr_S, {8, 1}, 1),
                    MinoState(Mino::ptr_Z, {6, 2}, 1),
                },
                {},
                false

        };

        //TODO make opener
    }
}

#include "bridge/bridge-sfinder-cpp.h"
namespace Engine {

    void inline printCell(char x, char y, const string& text) {
        MoveCursor(x * 2 + 3, BOARD_VISIBLE_HEIGHT - y + 1);
        fmt::print("{}", text);
    }

    template <size_t meter_size>
    void print(Game* game, Meter<long long, meter_size>* meter, MinoState currentTarget, const vector<MinoState>& stack_preview = {}, char empty_line = -1) {
        MoveCursor(1, 2);

        string string_board = game->view();

        char topHeight = Measurement::topHeight(game);
        char averageHeight = Measurement::averageHeight(game);
        char stackHeight = Stack::stackHeight(game, empty_line);
        char lowHeight = Stack::lowHeight(game, empty_line);


        vector<string> lines = split_string_by_char(string_board,'\n');
        for (char y = 0; y<BOARD_VISIBLE_HEIGHT; y++ ) {
            auto line = lines[y];
            if (y == BOARD_VISIBLE_HEIGHT-stackHeight-1){
                fmt::println("\033[1;32m│ {}\033[1;33m│ ◀ Stk\033[0m", line);
            }
            else if (y == BOARD_VISIBLE_HEIGHT-topHeight-1){
                fmt::println("\033[1;31m│ {}\033[1;31m│ ◀ Top\033[0m", line);
            }
            else if (y == BOARD_VISIBLE_HEIGHT-averageHeight-1){
                fmt::println("\033[1;33m│ {}\033[1;33m│ ◀ Avg\033[0m", line);
            }
            else if (y == BOARD_VISIBLE_HEIGHT-lowHeight-1){
                fmt::println("\033[1;32m│ {}\033[1;33m│ ◀ Low\033[0m", line);
            }
            else fmt::println("{}", "│ " + line + "│                ");
        }

//        for (MinoState minoHole : Engine::Stack::findMinoHoles(game, empty_line)) {
//            for (auto minoHolePosition: minoHole->cellPositions()) {
//                if (std::find(currentMinoPositions.begin(), currentMinoPositions.end(), minoHolePosition) == currentMinoPositions.end()) {
//                    MoveCursor(minoHolePosition.first*2+3, BOARD_VISIBLE_HEIGHT-minoHolePosition.second+1);
//                    auto colorstr = Cell::CellColor(minoHole->mino->id);
////                    colorstr[2] = '4'; // Change foreground to background
//                    fmt::print("{}{}{}\033[0m", colorstr, to_string(minoHole->mino->id), to_string(minoHole->mino->id));
//                }
//            }
//        }

        for (auto hole_info : Measurement::getHoles(Measurement::getHeights(game), empty_line)) {
            char x = std::get<0>(hole_info);
            for (char y = std::get<1>(hole_info); y <= std::get<2>(hole_info); y++)
                printCell(x, y, "\033[1;31m!!\033[0m");
        }

        {

            vector<Game*> clearSteps = {game->copy()};
            if (!currentTarget.is_empty_state()) {
                Game *copied_game = clearSteps.back()->copy();
                copied_game->acceptClear();
                copied_game->current = currentTarget.copy();
                copied_game->drop();
                clearSteps.push_back(copied_game);
            }
            for (auto state: stack_preview) {
                Game* copied_game = clearSteps.back()->copy();
                copied_game->acceptClear();
                copied_game->current = state.copy();
                copied_game->drop();
                clearSteps.push_back(copied_game);
            }

            std::reverse(clearSteps.begin(), clearSteps.end());
            Game* output = clearSteps.front();
            clearSteps.erase(clearSteps.begin());

            for (auto clear_step: clearSteps) {
                for (char y = 0; y < BOARD_HEIGHT - 1; ++y) {
                    bool clear_this_line = true;
                    for (auto cell: clear_step->board[y]) {
                        if (!cell) clear_this_line = false;
                    }
                    if (clear_this_line) {
                        for (char push_y = BOARD_HEIGHT - 2; push_y >= y; --push_y) {
                            std::copy(output->board[push_y], output->board[push_y] + 10, output->board[push_y + 1]);
                        }
                        std::copy(clear_step->board[y], clear_step->board[y] + 10, output->board[y]);
                    }
                }
                delete clear_step;
            }
            vector<pair<PosChar, char>> stack_preview_cell;
            for (char y = 0; y < BOARD_HEIGHT; ++y) {
                for (char x = 0; x < 10; ++x) {
                    if (output->board[y][x] != game->board[y][x])
                        stack_preview_cell.emplace_back(PosChar(x, y), output->board[y][x]);
                }
            }
            for (auto cellInfo: stack_preview_cell)
                printCell(cellInfo.first.first, cellInfo.first.second, Cell::CellColor(cellInfo.second) + "□\033[0m");
            delete output;
        }

        for (auto cell : game->current.cellPositions())
            printCell(cell.first, cell.second, Cell::CellString(game->current.mino->id));


        MoveCursor(0, 28);
        printf("Hold: ");
        if (game->hold_mino == nullptr) printf(" ");
        else fmt::print("{}{}\033[0m", Cell::CellColor(game->hold_mino->id), game->hold_mino->name);
        fmt::print(" | Current: {}{}\033[0m", Cell::CellColor(game->current.mino->id), game->current.mino->name);
        printf(" | Next: ");
        for (char next = 0; next < game->generator->MaxLookUp(); next++) {
            auto lookup = game->generator->Lookup(next);
            fmt::print("{}{}\033[0m ", Cell::CellColor(lookup->id), lookup->name);
        }
        printf("\n");

        auto delayed = meter->template average<double>();
        printf("%4.2f ms / %4.2f fps", delayed, 1/delayed*1000);
    }


    void initScreen() {
        CursorView_WINDOW(false);
        system("cls");
        fmt::println("┌─────────────────────┐");
        MoveCursor(1, BOARD_VISIBLE_HEIGHT + 2);
        fmt::println("└─────────────────────┘");
    }

}


#endif //ENGINE_H
