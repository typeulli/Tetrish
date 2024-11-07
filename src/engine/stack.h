#ifndef STACK_H
#define STACK_H


#include "../system/game.h"
#include "explorer.h"
#include "measurement.h"
#include "tspin.h"

namespace Engine::Stack {

    [[nodiscard]] char stackHeight(Game *game, char empty_line) {
        for (char y = BOARD_HEIGHT-1; y >= 0; y--) {
            if (game->board[y][empty_line]) {
                return (char) (y + 4);
            }
        }
        return ENGINE_DEFAULT_STACK_HEIGHT;
    }

    char lowHeight(Game* game, char empty_line) {
        for (char y = BOARD_HEIGHT-1; y >= 0; y--) {
            bool is_this_line = true;
            for (char x = 0; x < 10; x++) {
                if (x == empty_line) continue;
                if (!game->board[y][x]) {
                    is_this_line = false;
                    break;
                }
            }
            if (is_this_line) return y;
        }
        return 0;
    }




    auto stack_state_compare_function = [](pair<MinoState, double> t1, pair<MinoState, double> t2) -> bool {
        return t1.second > t2.second;
    };

    vector<pair<MinoState, double>> stack_state(Game* game, MinoState state, char empty_line) {
        auto reachable_list = Explore::reachable(game, state);
        auto line_filtered_list = Explore::filterEmptyLine(reachable_list, empty_line);
        auto stackable_list = Explore::filterStacked(game, line_filtered_list);
        auto notHovering_list = Explore::filterNotHovering(game, stackable_list);


        size_t minimum_hole_count = 1000;
        vector<tuple<MinoState, vector<char>>> minimum_hole_state;
        for (auto stackable: notHovering_list) {
            auto copied_game = game->copy();
            copied_game->current = stackable.copy();
            copied_game->drop();
            auto height = Measurement::getHeights(copied_game);
            delete copied_game;
            auto hole_count = Explore::getHoles(height, empty_line).size();
            if (minimum_hole_count > hole_count) {
                minimum_hole_count = hole_count;
                minimum_hole_state.clear();
            }
            if (minimum_hole_count == hole_count) {
                minimum_hole_state.emplace_back(stackable, height);
            }
        }

        // Deprecapted for performance
        //        Mino* next_mino = game->generator->Lookup(0);
        //        Mino* hold_mino = game->hold_mino != nullptr? game->hold_mino : game->generator->Lookup(1);
        //        vector<tuple<MinoState, Game, vector<char>>> next_also_stackable;
        //        for (auto stackable_info : minimum_hole_state) {
        //            auto filterNotHovering = get<0>(stackable_info);
        //            auto copied_game = get<1>(stackable_info);
        //
        //            if (! (Engine::filterEmptyLine(copied_game,
        //                                           Engine::filterNotHovering(
        //                                                   copied_game,
        //                                                   Engine::reachable(
        //                                                           copied_game,
        //                                                           MinoState::spawnState(next_mino))),
        //                                           empty_lineno).empty()
        //                   && Engine::filterEmptyLine(copied_game,
        //                                              Engine::filterNotHovering(
        //                                                      copied_game,
        //                                                      Engine::reachable(
        //                                                              copied_game,
        //                                                              MinoState::spawnState(hold_mino))),
        //                                              empty_lineno).empty())
        //                    ) next_also_stackable.push_back(stackable_info);
        //
        //
        //        }

        vector<pair<MinoState, double>> pointed;
        for (auto state_case: minimum_hole_state) {
            double delta_height_sum = 0;
            auto height = std::get<1>(state_case);

            for (char x = 0; x < 10 - 1; x++) {
                if (x == empty_line || x + 1 == empty_line) continue;
                delta_height_sum += static_cast<double>(pow(height[x] - height[x + 1], 2));
            }
            double standard_deviation_height = sqrt(delta_height_sum);



            //            double point = pow(0.6849348892-standard_deviation_height, 2);

            auto test_state = std::get<0>(state_case);

            double point = -(test_state.position.y / BOARD_HEIGHT / 4) - standard_deviation_height;
            pointed.emplace_back(test_state, point);
        }

        std::sort(pointed.begin(), pointed.end(), stack_state_compare_function);


        return pointed;
    }

    vector<pair<MinoState, double>> stackCell(Game* game, MinoState state, PosChar cell, char empty_line) {
        auto stack_state = Stack::stack_state(game, state, empty_line);

        vector<pair<MinoState, double>> filtered;
        for (auto state_case: stack_state) {
            for (auto pos: state_case.first.cellPositions()) {
                if (pos.first == cell.first && pos.second == cell.second) {
                    filtered.emplace_back(state_case.first.copy(), state_case.second);
                    break;
                }
            }
        }
        stack_state.clear();
        stack_state.shrink_to_fit();
        return filtered;
    }

    pair<bool, MinoState> stack(Game* game, char empty_lineno) {
        //put I mino if a hole whose depth is deeper than 3 blocks exists.
        bool is_current_I = game->current.mino == Mino::ptr_I;
        bool is_hold_I = game->hold_mino == Mino::ptr_I;
        if (is_current_I || is_hold_I) {
            auto holes = Explore::getHoles(Measurement::getHeights(game), empty_lineno);
            tuple<char, char, char> hole_to_fill = {-1, 0, 0};
            int last_depth = 0;
            for (auto hole_info: holes) {
                int depth = get<2>(hole_info) - get<1>(hole_info);
                if (depth > last_depth) {
                    last_depth = depth;
                    hole_to_fill = hole_info;
                }
            }
            holes.clear();
            holes.shrink_to_fit();
            if (get<0>(hole_to_fill) != -1)
                return {
                        is_hold_I,
                        MinoState(
                                Mino::ptr_I,
                                {static_cast<float>(get<0>(hole_to_fill) - .5),
                                 static_cast<float>(get<1>(hole_to_fill) + 1.5)},
                                1
                        )};
        }

        auto holes = Explore::findMinoHoles(game, empty_lineno);
        auto previous_hole_count = Explore::findHoleRangeLine(game, 0, BOARD_HEIGHT, empty_lineno).size();
        for (auto holdState: holes) {
            if (holdState.mino == game->current.mino) {
                auto copied_game = game->copy();
                copied_game->current = holdState.copy();
                copied_game->drop();
                auto _hole_count = Explore::findHoleRangeLine(copied_game, 0, BOARD_HEIGHT, empty_lineno).size();
                delete copied_game;
                if (_hole_count <= previous_hole_count)
                    return {false, holdState};
            }
        }
        for (auto holdState: holes) {
            if (holdState.mino == game->hold_mino) {
                auto copied_game = game->copy();
                copied_game->current = holdState.copy();
                copied_game->drop();
                auto _hole_count = Explore::findHoleRangeLine(copied_game, 0, BOARD_HEIGHT, empty_lineno).size();
                delete copied_game;
                if (_hole_count <= previous_hole_count)
                    return {true, holdState};
            }
        }


        auto available_current = stack_state(game, game->current, empty_lineno);
        vector<pair<MinoState, double>> available_hold;
        if (game->hold_available) {
            if (game->hold_mino == nullptr)
                available_hold = stack_state(game, MinoState::spawnState(game->generator->Lookup(0)), empty_lineno);
            else available_hold = stack_state(game, MinoState::spawnState(game->hold_mino), empty_lineno);
        }
        bool is_current_empty = available_current.empty();
        bool is_hold_empty = available_hold.empty();
        if (is_current_empty && is_hold_empty) return {false, MinoState::emptyState()};
        if (is_current_empty) return {true, available_hold[0].first};
        if (is_hold_empty) return {false, available_current[0].first};
        auto rate_current = available_current[0].second;
        auto rate_hold = available_hold[0].second;
        if (rate_current >= rate_hold) return {false, available_current[0].first};
        else return {true, available_hold[0].first};

    }

    // As both next and current minos are unavailable, both them have similar scores, so consider only current mino.
    vector<tuple<MinoState, MinoState, PosChar, double>> stackForce(Game* game, MinoState state, char empty_line, const vector<Mino*>& nexts) {
        auto reachable_list = Explore::reachable(game, state);
        auto line_filtered_list = Explore::filterEmptyLine(reachable_list, empty_line);
        auto stackable_list = Explore::filterStacked(game, line_filtered_list);

        auto previous_hovered_cells = Explore::getHoveredPos(game);

        vector<tuple<MinoState, MinoState, PosChar, double>> pointed_list;
        for (auto state_case : stackable_list) {

            auto copied_game = game->copy();
            copied_game->current = state_case.copy();
            copied_game->drop();
            copied_game->acceptClear();

            vector<PosChar> newHoveredPos;
            for (auto checkHoveredPos : Explore::getHoveredPos(copied_game)) {
                if (std::find(previous_hovered_cells.begin(), previous_hovered_cells.end(), checkHoveredPos) == previous_hovered_cells.end()) {
                    newHoveredPos.push_back(checkHoveredPos);
                }
            }

            if (newHoveredPos.size() - previous_hovered_cells.size() > 1) { delete copied_game; continue; }

            for (size_t next_state_n = 0; next_state_n < nexts.size(); next_state_n++) {
                for (auto way: Stack::stackCell(copied_game, MinoState::spawnState(nexts[next_state_n]), newHoveredPos[0], empty_line))
                    pointed_list.emplace_back(state_case, way.first, newHoveredPos[0], way.second - 0.2 * static_cast<double>(next_state_n));
            }
            delete copied_game;
        }
        std::sort(pointed_list.begin(), pointed_list.end(),
                  [](tuple<MinoState, MinoState, PosChar, double> t1, tuple<MinoState, MinoState, PosChar, double> t2) -> bool {
                      return std::get<3>(t1) > std::get<3>(t2);
                  });

        return pointed_list;
    }

    namespace DownStack {
        //TODO
        vector<tuple<char, char, vector<MinoState>>> downStack(Game* game) {
            vector<tuple<char, char, vector<MinoState>>> result;
            vector<tuple<Game*, char, char, vector<MinoState>>> search = {};
            vector<tuple<Game*, char, char, vector<MinoState>>> newSearch = {{game->copy(), 0, 0, {}}};
            while (!newSearch.empty()) {
                search = newSearch;
                newSearch.clear();
                for (tuple<Game*, char, char, vector<MinoState>> path : search) {
                    bool is_any_added = false;

                    {
                        for (auto reachable_current : Explore::reachable(get<0>(path), get<0>(path)->current)) {
                            Game* copied_game = get<0>(path)->copy();
                            copied_game->current = reachable_current;
                            copied_game->drop();
                            LineList cleared_line = copied_game->acceptClear().cleared_line;
                            char cleared_line_count = countLine(cleared_line);
                            char new_clear_line = (char) (get<1>(path) + cleared_line_count);
                            char new_tetris_line = get<2>(path);
                            vector<MinoState> newStates;
                            for (auto state : get<3>(path)) newStates.push_back(state);
                            newStates.push_back(reachable_current);
                            if (cleared_line_count == 4) ++new_tetris_line;
                            if (cleared_line) {
                                is_any_added = true;
                                newSearch.emplace_back(copied_game, new_clear_line, new_tetris_line, newStates);
                            } else {
                                delete copied_game;
                            }
                        }
                    }

                    if (get<0>(path)->hold_available) {
                        for (auto reachable_hold : Explore::reachable(get<0>(path), MinoState::spawnState(get<0>(path)->hold_mino))) {
                            Game* copied_game = get<0>(path)->copy();
                            copied_game->hold();
                            copied_game->current = reachable_hold;
                            copied_game->drop();
                            LineList cleared_line = copied_game->acceptClear().cleared_line;
                            char cleared_line_count = countLine(cleared_line);
                            char new_clear_line = (char) (get<1>(path) + cleared_line_count);
                            char new_tetris_line = get<2>(path);
                            vector<MinoState> newStates;
                            for (auto state : get<3>(path)) newStates.push_back(state);
                            newStates.push_back(reachable_hold);
                            if (cleared_line_count == 4) ++new_tetris_line;
                            if (cleared_line) {
                                is_any_added = true;
                                newSearch.emplace_back(copied_game, new_clear_line, new_tetris_line, newStates);
                            } else {
                                delete copied_game;
                            }
                        }
                    }
                    delete get<0>(path);
                    if (!is_any_added) result.emplace_back(get<1>(path), get<2>(path), get<3>(path));

                }
            }
            std::sort(result.begin(), result.end(),
                      [](tuple<char, char, vector<MinoState>> t1, tuple<char, char, vector<MinoState>> t2) -> bool {
                          return std::get<1>(t1) > std::get<1>(t2) || std::get<0>(t1) > std::get<0>(t2) || std::get<2>(t1).size() > std::get<2>(t2).size();
                      });
            return result;
        }

    }
}


#endif //STACK_H
