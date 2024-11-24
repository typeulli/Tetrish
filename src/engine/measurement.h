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
    double evaluate(Game* game, char empty_line) {
        double delta_height_sum = 0;
        auto height = getHeights(game);

        for (char x = 0; x < 10 - 1; x++) {
            if (x == empty_line || x + 1 == empty_line) continue;
            delta_height_sum += static_cast<double>(pow(height[x] - height[x + 1], 2));
        }
        
        double standard_deviation_height = sqrt(delta_height_sum);
        return standard_deviation_height;
    }
}

#endif //MEASUREMENT_H
