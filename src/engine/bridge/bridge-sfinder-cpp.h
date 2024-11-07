#ifndef BRIDGE_SFINDER_CPP_H
#define BRIDGE_SFINDER_CPP_H




#include <iostream>
#include <chrono>
#include "../../../sfinder-cpp/core/field.hpp"
#include "../../../sfinder-cpp/finder/perfect.hpp"
#include "../../../sfinder-cpp/core/types.hpp"
#include "../../../sfinder-cpp/core/srs.hpp"
#include "../../../sfinder-cpp/core/moves.hpp"
#include "../../system/game.h"
namespace Engine::PCSolver {

    namespace Test {

        template<int N>
        std::array<core::PieceType, N> toPieces(int value) {
            int arr[N];

            for (int index = N - 1; 0 <= index; --index) {
                int scale = 7 - index;
                arr[index] = value % scale;
                value /= scale;
            }

            for (int select = N - 2; 0 <= select; --select) {
                for (int adjust = select + 1; adjust < N; ++adjust) {
                    if (arr[select] <= arr[adjust]) {
                        arr[adjust] += 1;
                    }
                }
            }

            std::array<core::PieceType, N> pieces = {};
            for (int index = 0; index < N; ++index) {
                pieces[index] = static_cast<core::PieceType>(arr[index]);
            }

            return pieces;
        }

        void benchmark() {
            using namespace std::literals::string_literals;

            std::cout << "# Initialize" << std::endl;

            auto start = std::chrono::system_clock::now();

            auto field = core::createField(
                    "XX________"s +
                    "XX________"s +
                    "XXX______X"s +
                    "XXXXXXX__X"s +
                    "XXXXXX___X"s +
                    "XXXXXXX_XX"s +
                    ""
            );

            auto factory = core::Factory::create();
            auto moveGenerator = core::srs::MoveGenerator(factory);
            auto finder = finder::PerfectFinder<core::srs::MoveGenerator>(factory, moveGenerator);

            {
                auto elapsed = std::chrono::system_clock::now() - start;
                auto time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
                std::cout << time << " micro seconds" << std::endl;
            }

            std::cout << "# Measuring" << std::endl;

            const int maxDepth = 7;
            const int maxLine = 6;

            int success = 0;
            long long int totalTime = 0L;
            int max = 5040;
            for (int value = 0; value < max; ++value) {
                auto arr = toPieces<maxDepth>(value);
                auto pieces = std::vector(arr.begin(), arr.end());

                auto start2 = std::chrono::system_clock::now();

                auto result = finder.run(field, pieces, maxDepth, maxLine, false);

                if (!result.empty()) {
                    success += 1;
//            std::cout << value << std::endl;
//            for (const auto &item : result) {
//                std::cout << item.pieceType << "," << item.rotateType << "," << item.x << "," << item.y << " ";
//            }
//            std::cout << "" << std::endl;
                } else {
                    // 975, 2295
//            std::cout << value << std::endl;
//            for (int i = 0; i < pieces.size(); ++i) {
//                std::cout << pieces[i];
//            }
//            std::cout << "" << std::endl;
                }

                auto elapsed = std::chrono::system_clock::now() - start2;
                totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            }

            std::cout << "success: " << success << std::endl;

            std::cout << (double) totalTime / static_cast<double>(max) << " milli seconds" << std::endl;
        }

        void sample() {
            using namespace std::literals::string_literals;

            auto factory = core::Factory::create();
            auto moveGenerator = core::srs::MoveGenerator(factory);
            auto finder = finder::PerfectFinder<core::srs::MoveGenerator>(factory, moveGenerator);

            auto field = core::createField(
                    "_XXXXXX___"s +
                    "_XXXXXXXXX"s +
                    "XXXXXX_XXX"s +
                    ""
            );
            auto pieces = std::vector{
                    core::PieceType::I, core::PieceType::J, core::PieceType::J, core::PieceType::S, core::PieceType::O,
                    core::PieceType::L, core::PieceType::Z, core::PieceType::T, core::PieceType::I, core::PieceType::Z
            };

            const int maxDepth = 9;
            const int maxLine = 6;
            const bool holdEmpty = false;  // If true, hold is empty at start

            auto result = finder.run(field, pieces, maxDepth, maxLine, holdEmpty, false, 0);

            if (!result.empty()) {
                std::cout << "PC: success" << std::endl;

                for (const auto &item : result) {
                    std::cout << item.pieceType << ","  // enum PieceType in src/core/types.hpp
                              << item.rotateType << "," // enum RotateType in src/core/types.hpp
                              << item.x << ","
                              << item.y << " "
                              << std::endl;

                    field.put(factory.get(item.pieceType, item.rotateType), item.x, item.y);
                    field.clearLine();

                    std::cout << field.toString(maxLine) << std::endl;
                }
            } else {
                std::cout << "PC: failed" << std::endl;
            }
        }

    }

    core::Field toField(Game* game) {
        string field;
        for (char y = 0; y < BOARD_HEIGHT; y++) {
            bool is_empty = true;
            string line;
            for (char x : game->board[y]) {
                char c = x? 'X' : '_';
                is_empty = is_empty && c == '_';
                line += c;
            }
            if (y < 4) is_empty = false;
            if (is_empty) break;
            field.insert (0, line);
        }
        return core::createField(field);
    }
    core::PieceType MINO_ENCODE_LIST[7] = {core::PieceType::I, core::PieceType::O, core::PieceType::T, core::PieceType::Z, core::PieceType::S, core::PieceType::L, core::PieceType::J};
    Mino* MINO_DECODE_LIST[7] = {Mino::ptr_T, Mino::ptr_I, Mino::ptr_L, Mino::ptr_J, Mino::ptr_S, Mino::ptr_Z, Mino::ptr_O};
    inline core::PieceType encode_Mino(Mino* mino) {
        return MINO_ENCODE_LIST[mino->id - Mino::I.id];
    }
    inline Mino* decode_Mino(core::PieceType mino) {
        return MINO_DECODE_LIST[mino];
    }
    inline MinoState decode_MinoState(const core::Factory& factory, finder::Operation operation) {
        auto blocks = factory.get(operation.pieceType, operation.rotateType);
        vector<PosChar> cells;
        for (auto point : blocks.points)
            cells.emplace_back(point.x + operation.x, point.y + operation.y);
        return MinoState::convertHoleToState(cells);
    }
    vector<MinoState> solve(Game* game, int maxLine, bool leastLineClears) {
        auto field = toField(game);

        std::vector<core::PieceType> pieces;
        auto hold_mino_exist = game->hold_mino != nullptr;
        if (hold_mino_exist) pieces.push_back(encode_Mino(game->hold_mino));
        pieces.push_back(encode_Mino(game->current.mino));
        auto maxLookup = game->generator->MaxLookUp();
        for (char lookup = 0; lookup < maxLookup; lookup++)
            pieces.push_back(encode_Mino(game->generator->Lookup(lookup)));

        auto factory = core::Factory::create();
        auto moveGenerator = core::srs::MoveGenerator(factory);
        auto finder = finder::PerfectFinder<core::srs::MoveGenerator>(factory, moveGenerator);
        std::vector<finder::Operation> solution = finder.run(field, pieces, (int) pieces.size()-1, maxLine, !hold_mino_exist, leastLineClears, 0);
        std::vector<MinoState> result;

        for (auto step : solution)
            if (step.x != -1 && step.y != -1)
                result.push_back(decode_MinoState(factory, step));
        return result;
    }
}


#endif //BRIDGE_SFINDER_CPP_H
