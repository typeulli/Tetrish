#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine.h"

enum EngineMode : char {
    Opener = 0,
    Stack = 1,
    Tetris = 2,
    DownStack = 3,
    PC = 4,

};
const string EngineModeName[5] = {"Opener", "Stack", "Tetris", "DownStack", "PC"};

class EngineController {
public:
    Game* game;
    Clock* clock;
    Meter<long long, 10>* meter;
    vector<MinoState> preview;

    EngineMode mode;
    char empty_line;

    vector<EngineData::Opener::OpenerBook> openerBooks;
    EngineData::Opener::OpenerBook currentOpener;
    char currentOpenerLoop = 0;
    char maxOpenerLoop;
    explicit EngineController(Game* game, char empty_line, vector<EngineData::Opener::OpenerBook> openerBooks = {}, char maxOpenerLoop = 2) {
        this->game = game;
        this->clock = new Clock();
        this->meter = new Meter<long long, 10>();

        this->mode = openerBooks.empty()? EngineMode::Stack : EngineMode::Opener;
        this->empty_line = empty_line;

        this->openerBooks = vector<EngineData::Opener::OpenerBook>(openerBooks.size());
        std::copy(openerBooks.begin(), openerBooks.end(), this->openerBooks.begin());
        this->maxOpenerLoop = maxOpenerLoop;
    }

    ~EngineController() {
        delete clock;
        delete meter;
    }

    [[nodiscard]] EngineMode inferMode() const {
        if (this->mode == EngineMode::Opener) {
            if (!currentOpener.breakable && !preview.empty()) { return EngineMode::Opener; }
            if (currentOpenerLoop >= maxOpenerLoop) {  return EngineMode::Stack; }
            for (char x = 0; x < 10; x++) {
                for (auto & y : this->game->board) {
                    if (y[x] == Cell::Garbage) {
                        return EngineMode::Stack;
                    }
                }
            }
            return EngineMode::Opener;
        }
        if ((!openerBooks.empty()) && this->game->isEmpty()) { return EngineMode::Opener; } //TODO do pc instead

        char empty_line = this->inferEmptyHole();
        if (empty_line == 1 || empty_line == 8) return EngineMode::DownStack;
        if (this->mode == EngineMode::DownStack) {
            for (auto & y : this->game->board)
                if (y[empty_line]) return EngineMode::DownStack;
            return EngineMode::Stack;
        }

        if (this->game->current.mino->id == Mino::I.id
            || (this->game->hold_mino != nullptr && this->game->hold_mino->id == Mino::I.id)
            || (this->game->hold_mino == nullptr && this->game->generator->Lookup(0)->id == Mino::I.id)) {
            bool do_tetris = true;
            for (auto hole_info : Engine::Explore::getHoles(Engine::Measurement::getHeights(game), this->empty_line)) {
                if (get<2>(hole_info) >= 3 && !((this->empty_line == 1 || this->empty_line == 8) ^ get<0>(hole_info) >= Engine::Stack::stackHeight(this->game, this->empty_line))) {
                    do_tetris = false;
                    break;
                }
            }


            if (do_tetris && Engine::Stack::lowHeight(this->game, this->empty_line) >= Engine::Stack::stackHeight(this->game, this->empty_line)) {
                return EngineMode::Tetris;
            }
        }


        //TODO
        return EngineMode::Stack;
    }
    void applyMode() {
        auto mode = this->inferMode();
        if (mode != EngineMode::Opener) this->currentOpenerLoop = 0;
        this->mode = mode;
    }
    [[nodiscard]] char inferEmptyHole() const {
        for (char y = BOARD_HEIGHT-1; y >= 0; y--) {
            char empty_cell_x = -1;
            bool select_this = false;
            for (char x = 0; x < 10; x++) {
                if (!game->board[y][x]) {
                    if (empty_cell_x != -1) {
                        select_this = false;
                        break;
                    }
                    select_this = true;
                    empty_cell_x = x;
                }
            }
            if (select_this) {
                return empty_cell_x;
            }
        }
        return this->empty_line;
    }
    inline void applyEmptyHole() {
        this->empty_line = this->inferEmptyHole();
    }

    vector<MinoState> inferNextOpener() { //TODO
        throw std::logic_error("EngineController::inferNextOpener for Opener is not implemented yet.");
    }



    [[nodiscard]] vector<MinoState> inferNextStack() const {

        auto stack = Engine::Stack::stack(this->game, this->empty_line).second;
        if (stack.is_empty_state()) {
            vector<Mino*> next;
            if (game->hold_mino != nullptr) next.push_back(game->hold_mino);
            for (char next_mino_n = 0; next_mino_n < std::min(ENGINE_STACK_FORCE_NEXT_LOOKUP, (int) this->game->generator->MaxLookUp()); next_mino_n++)
                next.push_back(game->generator->Lookup(next_mino_n));
            auto stackForceCases = Engine::Stack::stackForce(this->game, this->game->current, this->empty_line, next);
            if (stackForceCases.empty()) return {};
            auto stackForce = stackForceCases[0];





            auto state_fill = get<1>(stackForce);
            vector<MinoState> result = { get<0>(stackForce), state_fill };

            auto game_history = game->copy();
            game_history->current = get<0>(stackForce).copy();
            game_history->drop();
            game_history->acceptClear();

            for (auto lookup : next) {
                if (lookup->id == state_fill.mino->id) break;
                auto stackable = Engine::Stack::stack_state(game_history, MinoState::spawnState(lookup), empty_line);

                bool appended = false;
                for (auto stack_test : stackable) {
                    auto copied_game = game_history->copy();
                    copied_game->current = stack_test.first.copy();
                    copied_game->drop();
                    copied_game->acceptClear();

                    auto reachable = Engine::Explore::filterEmptyLine(Engine::Explore::reachable(copied_game, MinoState::spawnState(state_fill.mino)), empty_line);

                    for (auto reachable_test : reachable) {
                        if (reachable_test == state_fill) {
                            appended = true;
                            result.push_back(stack_test.first);
                            break;
                        }
                    }
                    if (appended) {
                        delete game_history;
                        game_history = copied_game;
                        break;
                    }
                    delete copied_game;
                }

                // Failed to recovery
                if (!appended) { delete game_history; return result; }
            }
            delete game_history;
            return result;
        }
        return { stack };
    }

    vector<MinoState> inferNext() {
        if (this->mode == EngineMode::Opener) return this->inferNextOpener();
        if (this->mode == EngineMode::Stack) {
            auto next = this->inferNextStack();
            if (!next.empty()) return next;
            this->mode = EngineMode::DownStack;
        }
        if (this->mode == EngineMode::DownStack) {
            auto next = Engine::Stack::DownStack::downStack(game);
            if (next.empty())
                return this->inferNextStack(); // such as when O mino is current mino
            return get<2>(next[0]);
        }
        if (this->mode == EngineMode::Tetris) {
            MinoState state(Mino::ptr_I, {(float) this->empty_line - .5f,  (float) BOARD_SPAWN_HEIGHT - .5f}, 1);
            while (this->game->isAvailableState(state))
                state.position.y --;
            state.position.y ++;

            bool leftFilled = true;
            for (char x = 0; x < 10; x++) {
                if (x == this->empty_line) continue;

                for (auto cell : state.cellPositions()) {
                    if (!game->board[cell.second][x]) {
                        leftFilled = false;
                        break;
                    }
                }
                if (!leftFilled) break;
            }
            if (!leftFilled) return this->inferNextStack();


            return {state};
        }
        return {};
    }

    inline void applyNext() {
        for (auto state : this->inferNext()) this->preview.push_back(state);
    }

    pair<bool, MinoState>  getNextState() {
        auto current_id = game->current.mino->id;
        auto hold_id = game->generator->Lookup(0)->id; // Next mino is a default mino to come by holding.
        if (game->hold_mino != nullptr) hold_id = game->hold_mino->id;
        auto size = preview.size();
        for (int test_num = 0; test_num < size; test_num++) {
            auto test_case = preview[test_num];
            auto test_id = test_case.mino->id;
            if (test_id != current_id && test_id != hold_id) continue;
            if (!game->isAvailableState(test_case)) continue;
            bool isAnyTouch = false;
            for (auto cell : test_case.cellPositions()) {
                auto y = cell.second;
                if (y == 0 || game->board[y-1][cell.first]) {
                    isAnyTouch = true;
                    break;
                }
            }
            if (!isAnyTouch) continue;
            preview.erase(preview.begin() + test_num);
            return {test_id == hold_id, test_case};
        }
        return {false, MinoState::emptyState()};
    }

    void step() {
        this->applyEmptyHole();
        this->applyMode();
        if (this->preview.empty()) this->applyNext();
    }

//    void
    Game* boardAfterAllDropped() {
        Game* copied_game = game->copy();
        for (auto state : preview) {
            for (auto cell : state.cellPositions()) {
                game->board[cell.second][cell.first] = state.mino->id;
            }
        }
        return copied_game;
    }

    void print(MinoState currentTarget = MinoState::emptyState()) const {
        this->meter->push_back(this->clock->getDeltaTimeMS());
        Engine::print<10>(this->game, this->meter, currentTarget, this->preview, this->empty_line);
        MoveCursor(1, BOARD_VISIBLE_HEIGHT+3);
        fmt::print("                        ");
        MoveCursor(3+this->empty_line*2, BOARD_VISIBLE_HEIGHT+3);
        printf("^^");
        MoveCursor(50, 2);
        printf("MODE: %s  ", EngineModeName[this->mode].c_str());
    }
};
#endif //CONTROLLER_H
