#ifndef EXPLORER_H
#define EXPLORER_H

#include "../system/game.h"
#include "measurement.h"

namespace Engine::Explore {

    const vector<std::function<bool(Game*)>> moveFunctionList = {
            [](Game* g) -> bool{return g->moveLeft();},
            [](Game* g) -> bool{return g->moveRight();},
            [](Game* g) -> bool{return g->moveDown();},
            [](Game* g) -> bool{return g->rotateCW();},
            [](Game* g) -> bool{return g->rotateCCW();},
            [](Game* g) -> bool{return g->rotate180();},
    };

    [[nodiscard]] vector<vector<PosChar>>
    findHoleRangeLine(Game *game, char start, char end, char ignore_line = -1) {
        vector<vector<PosChar>> holes;
        vector<PosChar> current_check;
        vector<PosChar> check_position;
        for (char i = 0; i < 10; i++) {
            if (i == ignore_line) continue;
            for (char j = start; j < end; j++) {
                if (!game->board[j][i])
                    check_position.emplace_back(i, j);
            }
        }
        while (!check_position.empty()) {
            current_check = {};
            current_check.push_back(check_position.back());
            check_position.pop_back();
            while (true) {
                vector<PosChar> adjacents;
                for (auto current_cell: current_check) {
                    adjacents.emplace_back(current_cell.first + 1, current_cell.second);
                    adjacents.emplace_back(current_cell.first - 1, current_cell.second);
                    adjacents.emplace_back(current_cell.first, current_cell.second + 1);
                    adjacents.emplace_back(current_cell.first, current_cell.second - 1);
                }
                bool quit = true;
                for (auto check: adjacents) {
                    auto find = std::find(check_position.begin(), check_position.end(), check);
                    if (find != check_position.end()) {
                        quit = false;
                        current_check.push_back(check);
                        check_position.erase(find);
                    }
                }
                if (quit) break;

            }
            holes.push_back(current_check);
        }
        return holes;
    }



    [[nodiscard]] vector<MinoState> findMinoHoles(Game *game, char ignore_line_no = -1) {
        vector<vector<PosChar>> holes;
        vector<PosChar> checked_positions;
        if (ignore_line_no != -1) {
            for (char y = 0; y < BOARD_HEIGHT; y++)
                checked_positions.emplace_back(ignore_line_no, y);
        }

        for (char start = 0; start < BOARD_HEIGHT - 2; start++) {
            for (const vector<PosChar> &hole2line: Engine::Explore::findHoleRangeLine(game, start, (char) (start + 2), ignore_line_no)) {
                if (hole2line.size() != 4) continue;

                bool contain_checked_position = false;
                for (auto position: hole2line) {
                    if (std::find(checked_positions.begin(), checked_positions.end(), position) !=
                        checked_positions.end()) {
                        contain_checked_position = true;
                        break;
                    }
                }
                if (contain_checked_position) continue;


                for (auto position: hole2line) checked_positions.push_back(position);

                holes.push_back(hole2line);
            }
            for (const vector<PosChar> &hole3line: Engine::Explore::findHoleRangeLine(game, start, (char) (start + 3), ignore_line_no)) {
                if (hole3line.size() != 4) continue;

                bool contain_checked_position = false;
                for (auto position: hole3line) {
                    if (std::find(checked_positions.begin(), checked_positions.end(), position) !=
                        checked_positions.end()) {
                        contain_checked_position = true;
                        break;
                    }
                }
                if (contain_checked_position) continue;


                for (auto position: hole3line) checked_positions.push_back(position);

                holes.push_back(hole3line);
            }
        }

        vector<MinoState> converted;
        for (const auto &holeData: holes)
            converted.push_back(MinoState::convertHoleToState(holeData));

        holes.clear();
        holes.shrink_to_fit();
        checked_positions.clear();
        checked_positions.shrink_to_fit();
        return converted;
    }

    //stacks
    vector<tuple<char, char, char>> getHoles(vector<char> height, char ignore_line = -1) {

        vector<tuple<char, char, char>> result;
        for (char x = 0; x < 10; x++) {
            if (x == ignore_line) continue;

            char h = height[x];
            if (
                    (x == 0 || x - 1 == ignore_line || height[x - 1] > h + 2)
                    && (x == 9 || x + 1 == ignore_line || height[x + 1] > h + 2)
                    ) {
                char nearHeight = BOARD_HEIGHT;
                if (x != 0 && x - 1 != ignore_line) nearHeight = std::min(nearHeight, height[x - 1]);
                if (x != 9 && x + 1 != ignore_line) nearHeight = std::min(nearHeight, height[x + 1]);

                result.emplace_back(x, h + 1, nearHeight);
            }
        }

        return result;
    }


    vector<MinoState> reachable(Game* game, MinoState state) {
        char topHeight = Engine::Measurement::topHeight(game);

        MinoState start_state = state.copy();
        while (start_state.position.y >= static_cast<float>(topHeight + 3) + 1)
            --start_state.position.y;


        vector<MinoState> checked;
        vector<MinoState> current_check;
        vector<MinoState> next_check = {start_state};

        while (!(next_check.empty() && current_check.empty())) {
            for (auto last_checked_state: current_check)
                checked.push_back(last_checked_state);
            current_check.clear();
            for (auto to_check_state: next_check)
                current_check.push_back(to_check_state);
            next_check.clear();

            for (MinoState checking: current_check) {
                for (const std::function<bool(Game *)> &moveFunction: Explore::moveFunctionList) {
                    std::unique_ptr<Game> copied_game(game->copy());
                    copied_game->current = checking.copy();
                    bool successMove = moveFunction(copied_game.get());
                    if (!successMove) {
                        continue;
                    }

                    MinoState moved_state = copied_game->current.copy();
                    bool found = false;
                    for (MinoState compare_next_check: next_check) {
                        if (compare_next_check.rotation ==  moved_state.rotation  // check rotation first for speed
                            && compare_next_check.position == moved_state.position) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;
                    for (MinoState compare_current_check: current_check) {
                        if (compare_current_check.rotation == moved_state.rotation  // check rotation first for speed
                            && compare_current_check.position == moved_state.position) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;
                    for (MinoState compare_checked: checked) {
                        if (compare_checked.rotation == moved_state.rotation  // check rotation first for speed
                            && compare_checked.position == moved_state.position) {
                            found = true;
                            break;
                        }
                    }
                    if (found) continue;
                    next_check.push_back(moved_state);

                }

            }
        }

        current_check.clear();
        current_check.shrink_to_fit();
        next_check.clear();
        next_check.shrink_to_fit();
        return checked;
    }

    [[nodiscard]] vector<MinoState> filterStacked(Game *game, const vector<MinoState> &reachable) {
        vector<MinoState> checked;
        for (auto test: reachable) {
            for (auto cell: test.cellPositions()) {
                if (cell.second == 0) {
                    checked.push_back(test);
                    break;
                }
                if (game->board[cell.second - 1][cell.first]) {
                    checked.push_back(test);
                    break;
                }
            }
        }

        return checked;
    }

    [[nodiscard]] vector<MinoState> filterNotHovering(Game *game, const vector<MinoState> &reachable) {
        vector<MinoState> checked;
        for (auto test: reachable) {
            char min_y[10] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127};
            for (auto cell: test.cellPositions())
                min_y[cell.first] = std::min(min_y[cell.first], cell.second);
            bool available = true;
            for (char x = 0; x < 10; x++) {
                auto test_y = min_y[x];
                if (test_y == 127) continue;
                if (test_y <= 0) continue;
                if (game->board[test_y - 1][x] == 0) {
                    available = false;
                    break;
                }
            }
            if (available) checked.push_back(test);
        }

        return checked;
    }

    [[nodiscard]] vector<MinoState> filterEmptyLine(const vector<MinoState> &stackable, char empty_line) {
        vector<MinoState> filtered;
        for (auto stackable_state: stackable) {
            bool lineno_test = true;
            for (auto cell: stackable_state.cellPositions()) {
                if (cell.first == empty_line) {
                    lineno_test = false;
                    break;
                }
            }
            if (lineno_test) filtered.push_back(stackable_state);
        }
        return filtered;
    }

    vector<PosChar> getHoveredPos(Game* game) {
        vector<PosChar> result;
        for (char y = 0; y < BOARD_HEIGHT-1; y++) {
            for (char x = 0; x < 10; x++) {
                if (!game->board[y][x] && game->board[y+1][x]) result.emplace_back(x, y);
            }
        }
        return result;
    }
}

#endif //EXPLORER_H
