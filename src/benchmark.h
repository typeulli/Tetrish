#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>
#include "engine/engine.h"
#include "engine/controller.h"


namespace BenchMark {


    [[maybe_unused]] void test_reachable(int n = 10000) {
        string test_field = """        xx\n        xx\nx  xx  xxx\nxxxx   xxx\nxxxx xxxxx\nxxxx  xxxx\nxxxx xxxxx\nxxxxx xxxx\nxxxx   xxx\nxxxxx xxxx""";
        std::cout << "Getting ready to test Engine::Explore::reachable" << std::endl;
        auto gen = new MinoGenerator_7Bag();
        auto game = new Game(gen);
        auto state = MinoState::spawnState(Mino::ptr_T);
        std::cout << "Testing Engine::reachable" << std::endl;
        auto clock = new Clock();
        for (int i = 0; i < n; i++) {
            Engine::Explore::reachable(game, state).shrink_to_fit();
        }
        auto dt = clock->getDeltaTimeMS();
        delete clock;

        printf("Time: %.4fms | Average: %.4fms | (%d tries)", dt, dt/(float)n, n);
    }
}

#endif //BENCHMARK_H
