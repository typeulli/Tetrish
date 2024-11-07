#ifndef CORE_FIELD_HPP
#define CORE_FIELD_HPP

#include <cassert>

#include "types.hpp"
#include "bits.hpp"
#include "piece.hpp"

namespace core {
    union Field {
    public:
        Bitboard boards[4];
        struct {
            Bitboard xBoardLow;
            Bitboard xBoardMidLow;
            Bitboard xBoardMidHigh;
            Bitboard xBoardHigh;
        };

        Field() : xBoardLow(0), xBoardMidLow(0), xBoardMidHigh(0), xBoardHigh(0) {};

        void setBlock(int x, int y);

        void removeBlock(int x, int y);

        [[nodiscard]] bool isEmpty(int x, int y) const;

        void put(const Blocks &blocks, int x, int y);

        void putAtMaskIndex(const Blocks &blocks, int leftX, int lowerY);

        void remove(const Blocks &blocks, int x, int y);

        void removeAtMaskIndex(const Blocks &blocks, int leftX, int lowerY);

        [[nodiscard]] bool canPut(const Blocks &blocks, int x, int y) const;

        [[nodiscard]] bool canPutAtMaskIndex(const Blocks &blocks, int leftX, int lowerY) const;

        [[nodiscard]] bool isOnGround(const Blocks &blocks, int x, int y) const;

        [[nodiscard]] int getYOnHarddrop(const Blocks &blocks, int x, int startY) const;

        [[nodiscard]] bool canReachOnHarddrop(const Blocks &blocks, int x, int y) const;

        void clearLine();

        int clearLineReturnNum();

        LineKey clearLineReturnKey();

        [[nodiscard]] int getBlockOnX(int x, int maxY) const;

        [[nodiscard]] bool isWallBetween(int x, int maxY) const;

        [[nodiscard]] std::string toString(int height) const;

    private:
        void deleteLine_(LineKey low, LineKey midLow, LineKey midHigh, LineKey high);
    };

    inline bool operator==(const Field &lhs, const Field &rhs) {
        return lhs.xBoardLow == rhs.xBoardLow && lhs.xBoardMidLow == rhs.xBoardMidLow
               && lhs.xBoardMidHigh == rhs.xBoardMidHigh && lhs.xBoardHigh == rhs.xBoardHigh;
    }

    inline bool operator!=(const Field &lhs, const Field &rhs) {
        return !(lhs == rhs);
    }

    Field createField(std::string marks);
}

#endif //CORE_FIELD_HPP
