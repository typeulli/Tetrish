#ifndef STACK_H
#define STACK_H


#include "../system/game.h"
#include "search.h"
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




    const auto stack_state_compare_function = [](pair<MinoState, double> t1, pair<MinoState, double> t2) -> bool {
        return t1.second > t2.second;
    };

    vector<pair<MinoState, double>> stack_state(Game* game, MinoState state, char empty_line) {
        auto reachable_list = Search::reachable(game, state);
        auto line_filtered_list = Search::filterEmptyLine(reachable_list, empty_line);
        auto stackable_list = Search::filterStacked(game, line_filtered_list);
        auto notHovering_list = Search::filterNotHovering(game, stackable_list);




        size_t minimum_hole_count = 1000;
        vector<pair<MinoState, double>> minimum_hole_state;
        for (auto stackable: notHovering_list) {
            auto copied_game = game->copy();
            copied_game->current = stackable.copy();
            copied_game->drop();
            auto height = Measurement::getHeights(copied_game);
            auto hole_count = Search::getHoles(height, empty_line).size();
            if (minimum_hole_count > hole_count) {
                minimum_hole_count = hole_count;
                minimum_hole_state.clear();
            }
            if (minimum_hole_count == hole_count) {
                minimum_hole_state.emplace_back(stackable, -(stackable.position.y / BOARD_HEIGHT / 4) - Engine::Measurement::evaluate(copied_game, empty_line));
            }
            delete copied_game;
        }

        std::sort(minimum_hole_state.begin(), minimum_hole_state.end(), stack_state_compare_function);

        return minimum_hole_state;
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
            auto holes = Search::getHoles(Measurement::getHeights(game), empty_lineno);
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

        auto holes = Search::findMinoHoles(game, empty_lineno);
        auto previous_hole_count = Search::findHoleRangeLine(game, 0, BOARD_HEIGHT, empty_lineno).size();
        for (auto holdState: holes) {
            if (holdState.mino == game->current.mino) {
                auto copied_game = game->copy();
                copied_game->current = holdState.copy();
                copied_game->drop();
                auto _hole_count = Search::findHoleRangeLine(copied_game, 0, BOARD_HEIGHT, empty_lineno).size();
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
                auto _hole_count = Search::findHoleRangeLine(copied_game, 0, BOARD_HEIGHT, empty_lineno).size();
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
        auto reachable_list = Search::reachable(game, state);
        auto line_filtered_list = Search::filterEmptyLine(reachable_list, empty_line);
        auto stackable_list = Search::filterStacked(game, line_filtered_list);

        auto previous_hovered_cells = Search::getHoveredPos(game);

        vector<tuple<MinoState, MinoState, PosChar, double>> pointed_list;
        for (auto state_case : stackable_list) {

            auto copied_game = game->copy();
            copied_game->current = state_case.copy();
            copied_game->drop();
            copied_game->acceptClear();

            vector<PosChar> newHoveredPos;
            for (auto checkHoveredPos : Search::getHoveredPos(copied_game)) {
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
        vector<tuple<char, char, vector<MinoState>>> downStack(Game* game) {

            vector<tuple<char, char, vector<MinoState>>> result;
            vector<tuple<Game*, char, char, vector<MinoState>>> search = {};
            vector<tuple<Game*, char, char, vector<MinoState>>> newSearch = {{game->copyState(), 0, 0, {}}};
            while (!newSearch.empty()) {
                search = newSearch;
                newSearch.clear();
                for (tuple<Game*, char, char, vector<MinoState>> path : search) {
                    bool is_any_added = false;

                    Game* last_game = get<0>(path);
                    for (auto reachable_current : Search::filterStacked(last_game, Search::reachable(last_game, last_game->current))) {
                        Game* copied_game = last_game->copyState();
                        copied_game->current = reachable_current;
                        copied_game->drop();
                        LineList cleared_line = copied_game->acceptClear().cleared_line;
                        char cleared_line_count = countLine(cleared_line);
                        if (cleared_line_count == 0) {
                            delete copied_game; continue;
                        }
                        is_any_added = true;
                        char new_clear_line = (char) (get<1>(path) + cleared_line_count);
                        char new_tetris_line = get<2>(path);
                        vector<MinoState> newStates;
                        for (auto state : get<3>(path)) newStates.push_back(state.copy());
                        newStates.push_back(reachable_current);
                        if (cleared_line_count == 4) ++new_tetris_line;
                        if (copied_game->generator->MaxLookUp() == 0) {
                            delete copied_game;
                            result.emplace_back(new_clear_line, new_tetris_line, newStates);
                        }
                        else newSearch.emplace_back(copied_game, new_clear_line, new_tetris_line, newStates);
                    }

                    if (last_game->hold_available && last_game->hold_mino != nullptr) {
                        for (auto reachable_hold : Search::filterStacked(last_game, Search::reachable(last_game, MinoState::spawnState(last_game->hold_mino)))) {
                            Game* copied_game = last_game->copyState();
                            copied_game->hold();
                            copied_game->current = reachable_hold;
                            copied_game->drop();
                            LineList cleared_line = copied_game->acceptClear().cleared_line;
                            char cleared_line_count = countLine(cleared_line);
                            if (cleared_line_count == 0) {
                                delete copied_game; continue;
                            }
                            is_any_added = true;
                            char new_clear_line = (char) (get<1>(path) + cleared_line_count);
                            char new_tetris_line = get<2>(path);
                            vector<MinoState> newStates;
                            for (auto state : get<3>(path)) newStates.push_back(state.copy());
                            newStates.push_back(reachable_hold);
                            if (cleared_line_count == 4) ++new_tetris_line;
                            if (copied_game->generator->MaxLookUp() == 0) {
                                delete copied_game;
                                result.emplace_back(new_clear_line, new_tetris_line, newStates);
                            }
                            else newSearch.emplace_back(copied_game, new_clear_line, new_tetris_line, newStates);

                        }
                    }
                    delete last_game;
                    if (!is_any_added) result.emplace_back(get<1>(path), get<2>(path), get<3>(path));
                }
            }


            vector<tuple<char, char, vector<MinoState>>> filtered;
            for (auto path : result)
                if (!get<2>(path).empty())
                    filtered.push_back(path);

            std::sort(filtered.begin(), filtered.end(),
                      [](tuple<char, char, vector<MinoState>> t1, tuple<char, char, vector<MinoState>> t2) -> bool {
                          return std::get<1>(t1) > std::get<1>(t2) || std::get<0>(t1) > std::get<0>(t2) || std::get<2>(t1).size() > std::get<2>(t2).size();
                      });
            return filtered;
        }

    }
}


#endif //STACK_H
