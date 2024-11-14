#pragma GCC optimize("Ofast")

#include <iostream>

#include "engine/controller.h"
#include "benchmark.h"
#include "license.h"
//#include <string>
//#include <random>

#define OPTION_FORCE 1
#define OPTION_STEP  2

int mode_inf(int option) {
    initConsole_WINDOW();

    printf("\n");
    printf("Press any key to start.");
    system("pause > nul");

    Engine::initScreen();
    auto gen = new MinoGenerator_7Bag(5);
    auto *game = new Game(gen);
    auto *controller = new EngineController(game, 9);

    auto* clock = new Clock();
    int total_clear = 0;
    while (true) {
        controller->step();
        auto pair_work = controller->getNextState();
        if (pair_work.second.is_empty_state()) {
            fmt::println("No possible");
            break;
        }
        controller->print(pair_work.second);

        if (pair_work.first) game->hold();
        game->current = pair_work.second.copy();
        if (option & OPTION_STEP) system("pause > nul");
        game->drop();
        total_clear += countLine(game->acceptClear().cleared_line);
        system(fmt::format("title Cleared: {}", total_clear).c_str());
    }
    controller->print();
    MoveCursor(0, 30);
    printf("Time used for die: %.4fs", (double) clock->getDeltaTimeMS() / 1000);
    if (!option & OPTION_FORCE) {
        printf(" / Press any key to exit.");
        system("pause > nul");
    }
    CursorView_WINDOW(true);
    return EXIT_SUCCESS;
}

int mode_40l(int option) {
    initConsole_WINDOW();

    printf("\n");
    printf("Press any key to start.");
    system("pause > nul");

    Engine::initScreen();
    auto gen = new MinoGenerator_7Bag(5);
    auto *game = new Game(gen);
    auto *controller = new EngineController(game, 9);

    auto* clock_40l = new Clock();
    int total_clear = 0;
    while (true) {
        controller->step();
        auto pair_work = controller->getNextState();
        if (pair_work.second.is_empty_state()) {
            fmt::println("No possible");
            break;
        }
        controller->print(pair_work.second);

        if (pair_work.first) game->hold();
        game->current = pair_work.second.copy();
        if (option & OPTION_STEP) system("pause > nul");
        game->drop();
        total_clear += countLine(game->acceptClear().cleared_line);
        system(fmt::format("title 40L: {}/40", total_clear).c_str());
        if (total_clear >= 40) break;
    }
    controller->print();
    MoveCursor(0, 30);
    printf("Time used for 40 lines clear: %.4fs", (double) clock_40l->getDeltaTimeMS() / 1000);
    if (!option & OPTION_FORCE) {
        printf(" / Press any key to exit.");
        system("pause > nul");
    }
    CursorView_WINDOW(true);
    return EXIT_SUCCESS;
}

int mode_pco(int option) {
    initConsole_WINDOW();

    printf("\n");
    printf("Press any key to start.");
    system("pause > nul");

    Engine::initScreen();
    auto gen = new MinoGenerator_7Bag(5);
    gen->items.insert(gen->items.begin(), Mino::I.id);

    auto *game = new Game(gen);
    game->build("###     ##\n###    ###\n###   ####\n###    ###");
    auto *controller = new EngineController(game, 9);

    for (auto step : Engine::PCSolver::solve(game, 4, true)) controller->preview.push_back(step);

    controller->print();
    system("pause");
    while (true) {
        auto pair_work = controller->getNextState();
        if (pair_work.second.is_empty_state()) break;

        auto target = pair_work.second.copy();
        if (pair_work.first) {
            game->hold();
            controller->print(target);
            system("pause");
        }
        game->current = target;
        controller->print(target);
        if (option & OPTION_STEP) system("pause > nul");
        game->drop();
        game->acceptClear();
        system("pause");
    }

    if (!option & OPTION_FORCE) {
        printf("Press any key to exit.");
        system("pause > nul");
    }
    CursorView_WINDOW(true);
    return EXIT_SUCCESS;
}


int mode_down_stack(int option) {
    initConsole_WINDOW();

    printf("\n");
    printf("Press any key to start.");
    system("pause > nul");

    Engine::initScreen();
    auto gen = new MinoGenerator_7Bag(5);
    auto *game = new Game(gen);
    for (int i=0; i < 14; ++i) game->addMassLine(rand() % 10);
    auto *controller = new EngineController(game, 9);
    controller->print();

    auto* clock = new Clock();
    int total_clear = 0;
    while (true) {
        controller->step();
        auto pair_work = controller->getNextState();
        if (pair_work.second.is_empty_state()) {
            fmt::println("No possible");
            break;
        }
        controller->print(pair_work.second);

        if (pair_work.first) game->hold();
        game->current = pair_work.second.copy();
        if (option & OPTION_STEP) system("pause > nul");
        game->drop();
        total_clear += countLine(game->acceptClear().cleared_line);
        system(fmt::format("title Cleared: {}", total_clear).c_str());
    }
    controller->print();
    MoveCursor(0, 30);
    printf("Time used for die: %.4fs", (double) clock->getDeltaTimeMS() / 1000);
    if (!option & OPTION_FORCE) {
        printf(" / Press any key to exit.");
        system("pause > nul");
    }
    CursorView_WINDOW(true);
    return EXIT_SUCCESS;
}

int mode_communicate() {
    auto gen = new MinoGenerator_7Bag(5);
    Game* game = new Game(gen);
    auto* controller = new EngineController(game, 9);

    string command;

    while (true) {
        std::getline(std::cin, command);
        if (command == "exit") break;
        if (command.starts_with("board ")) {
            game->build(command.substr(6), '/');
            continue;
        }
        if (command.starts_with("next ")) {
            gen->items.clear();
            for (const string& mino : split_string_by_char(command.substr(5), ' ')) {
                if (!mino.empty())
                    try { gen->items.push_back(Mino::fromChar(mino.c_str()[0])->id); }
                    catch (std::invalid_argument& e) {

                    }
            }
            continue;
        }
        if (command.starts_with("current ")) {
            game->current = MinoState::spawnState(Mino::fromChar(command[8]));
            continue;
        }
        if (command.starts_with("hold ")) {
            game->hold_mino = Mino::fromChar(command[5]);
            continue;
        }
        if (command.starts_with("next_size ")) {
            gen->lookup = (char) stoi(command.substr(9));
            continue;
        }
        if (command == "print") {
            controller->print(game->current);
            continue;
        }
        if (command == "cls") {
            system("cls");
            continue;
        }
        if (command == "moveDown") {
            controller->game->moveDown();
            continue;
        }
        if (command == "moveLeft") {
            controller->game->moveLeft();
            continue;
        }
        if (command == "moveRight") {
            controller->game->moveRight();
            continue;
        }
        if (command == "rotateCW") {
            controller->game->rotateCW();
            continue;
        }
        if (command == "rotateCCW") {
            controller->game->rotateCCW();
            continue;
        }
        if (command == "rotate180") {
            controller->game->rotate180();
            continue;
        }
        if (command == "hold") {
            controller->game->hold();
            continue;
        }
        if (command == "hard_drop") {
            controller->game->drop();
            controller->game->acceptClear();
            continue;
        }
    }

    delete game;
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    initConsole_WINDOW();

    printf("* License Acknowledgement\n");
    fmt::println("{:<{}} {:<{}} {}", "Name", LICENSE_NAME_LENGTH, "CopyRight (License Type)", LICENSE_COPYRIGHT_LENGTH+4, "Url");
    for (auto license : Licenses) {
        fmt::println("{:<{}} {:<{}} {}", license.name, LICENSE_NAME_LENGTH, string(license.copyright) + "(" + license.type + ")", LICENSE_COPYRIGHT_LENGTH+4 , license.url);
    }

    char mode = 0;
    int  option = 0;

    for (int count = 1; count < argc; count++) {
        if ((!mode) & strcmp(argv[count], "--inf") == 0) {
            mode = 1;
        }
        else if ((!mode) & strcmp(argv[count], "--40l") == 0) {
            mode = 2;
        }
        else if ((!mode) & strcmp(argv[count], "--pco") == 0) {
            mode = 3;
        }
        else if ((!mode) & strcmp(argv[count], "--down") == 0) {
            mode = 4;
        }
        if (strcmp(argv[count], "-f") == 0 || strcmp(argv[count], "--force") == 0) {
            option |= OPTION_FORCE;
        }
        if (strcmp(argv[count], "-s") == 0 || strcmp(argv[count], "--step") == 0) {
            option |= OPTION_STEP;
        }
    }
    if (mode == 1) {
        return mode_inf(option);
    } else if (mode == 2) {
        return mode_40l(option);
    } else if (mode == 3) {
        return mode_pco(option);
    } else if (mode == 4) {
        return mode_down_stack(option);
    } else {
        printf("Communicate mode\n");
        return mode_communicate();
    }


}
