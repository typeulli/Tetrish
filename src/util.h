#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <queue>

using std::pair;
using std::tuple;
using std::vector;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::make_unique;
using std::queue;

vector<string> split_string_by_char(const string& str, char delimiter) {
    vector<string> ret;
    string token;
    std::stringstream ss(str);
    while (getline(ss, token, delimiter))
    {
        ret.push_back(token);
    }
    return ret;
}


#define MoveCursor(x, y) printf("\033[%d;%dH", y, x)


#include <random>
#include <iterator>

//template<typename Iter, typename RandomGenerator>
//Iter select_randomly_from_gen(Iter start, Iter end, RandomGenerator& g) {
//    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
//    std::advance(start, dis(g));
//    return start;
//}
//
//template<typename Iter>
//Iter select_randomly(Iter start, Iter end) {
//    static std::random_device rd;
//    static std::mt19937 gen(rd());
//    return select_randomly_from_gen(start, end, gen);
//}


template <typename T, std::size_t size>
class Meter {
    T data[size];

public:
    Meter() {
        std::fill(std::begin(data), std::end(data), 0);
    }

    template <typename U>
    inline U average() {
        return (U) std::accumulate(std::begin(data), std::end(data), 0) / (U) size;
    }
    void push_back(T item) {
        for (size_t i = 0; i < size - 1; ++i)
            data[i] = i + 1;

        data[size - 1] = item;
    }
};

#include <chrono>

class Clock {
    std::chrono::time_point<std::chrono::steady_clock> last{std::chrono::steady_clock::now()};

public:
    Clock() { getDeltaTimeMS(); }
    long long getDeltaTimeMS() {
        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
        this->last = now;
        return delta.count();
    }
};


#include <windows.h>

void initConsole_WINDOW() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, consoleMode);
}


void CursorView_WINDOW(bool show) {
    HANDLE hConsole;
    CONSOLE_CURSOR_INFO ConsoleCursor;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    ConsoleCursor.bVisible = show;
    ConsoleCursor.dwSize = 1;

    SetConsoleCursorInfo(hConsole, &ConsoleCursor);
}


#endif //UTIL_H
