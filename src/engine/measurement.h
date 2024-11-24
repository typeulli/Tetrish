#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "../system/game.h"

namespace Engine::Measurement {
    [[nodiscard]] char topHeight(Game* game) {
        for (char height = BOARD_HEIGHT - 1; height > 0; height--) {
            for (char cell: game->board[height]) {
                if (cell) return (char) height;
            }
        }
        return 0;
    }

    [[nodiscard]] char averageHeight(Game* game) {
        int sum = 0;
        char count = 0;
        for (char height = BOARD_HEIGHT - 1; height > 0; height--) {
            auto &row = game->board[height];
            for (char cell: row) {
                if (cell) {
                    sum += height;
                    count++;
                }
                if (count == 10) break;
            }
            if (count == 10) break;
        }
        if (count == 0) return 0;
        return ceil((double) sum / (double) count);
    }

    float density(Game *game, char bottom, char top, char ignore_x = -1) {
        char count = 0;
        for (char y = bottom; y <= top; y++) {
            if (y == ignore_x) continue;
            for (char x = 0; x < 10; x++) {
                if (game->board[y][x]) count++;
            }
        }
        int height = top - bottom + 1;
        int all = ignore_x == -1 ? height * 10 : height * 9;
        return (float) count / (float) (height * all);
    }
    std::vector<char> getHeights(Game* game) {
        std::vector<char> height = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
        for (char x = 0; x < 10; x++) {
            for (char y = BOARD_HEIGHT - 1; y >= 0; y--) {
                if (game->board[y][x]) {
                    height[x] = y;
                    break;
                }
            }
        }
        return height;
    }



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
            for (const vector<PosChar> &hole2line: Engine::Measurement::findHoleRangeLine(game, start, (char) (start + 2), ignore_line_no)) {
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
            for (const vector<PosChar> &hole3line: Engine::Measurement::findHoleRangeLine(game, start, (char) (start + 3), ignore_line_no)) {
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

    vector<PosChar> getHoveredPos(Game* game) {
        vector<PosChar> result;
        for (char y = 0; y < BOARD_HEIGHT-1; y++) {
            for (char x = 0; x < 10; x++) {
                if (!game->board[y][x] && game->board[y+1][x]) result.emplace_back(x, y);
            }
        }
        return result;
    }

    double evaluate(Game* game, char empty_line = -1) {
        double delta_height_sum = 0;
        auto height = getHeights(game);

        for (char x = 0; x < 10 - 1; x++) {
            if (empty_line != -1 && (x == empty_line || x + 1 == empty_line)) continue;
            delta_height_sum += static_cast<double>(pow(height[x] - height[x + 1], 2));
        }

        double standard_deviation_height = sqrt(delta_height_sum);
        return standard_deviation_height + (double) Engine::Measurement::getHoveredPos(game).size();
    }
}

#endif //MEASUREMENT_H
