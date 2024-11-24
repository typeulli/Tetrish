#ifndef TSPIN_H
#define TSPIN_H

#include "measurement.h"

namespace Engine::TSpin {
    vector<MinoState> getTDonateTarget(Game* game) {
        auto height = Engine::Measurement::getHeights(game);
        vector<MinoState> result;
        for (char x=0+1; x<10-1; x++) {
            char height_x_plus_1 = (char) (height[x]+1);
            if (height[x-1] == height_x_plus_1 && height_x_plus_1 == height[x+1])
                result.emplace_back(Mino::ptr_T, Pos{(float) x, (float) height_x_plus_1}, 2);
        }
        return result;  
    }
    vector<vector<MinoState>> getTDonate(Game* game) {
        for (auto state : getTDonateTarget(game)) {
            char target_y_bottom = 0;
            vector<PosChar> cells = state.cellPositions();
            for (PosChar pos : cells)
                target_y_bottom = std::min(target_y_bottom, pos.second);

            vector<PosChar> donate_holes;
            for (char x = 0; x < 10; ++x) {
                if (!game->board[target_y_bottom][x]) donate_holes.emplace_back(x, target_y_bottom);
                if (!game->board[target_y_bottom+1][x]) donate_holes.emplace_back(x, target_y_bottom+1);
            }

        }
    }
}
#endif //TSPIN_H
