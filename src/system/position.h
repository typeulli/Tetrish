#ifndef POSITION_H
#define POSITION_H

typedef pair<char, char> PosChar;

class Pos {
public:
    
    float x;
    float y;

    Pos(float x, float y): x(x), y(y) {}

    Pos operator+(const Pos& other) const {
        return {x + other.x, y + other.y};
    }
    void operator+=(const Pos& other) {
        this->x += other.x;
        this->y += other.y;
    }

    Pos operator-(const Pos& other) const {
        return {x - other.x, y - other.y};
    }
    void operator-=(const Pos& other) {
        this->x -= other.x;
        this->y -= other.y;
    }

    Pos operator-() const {
        return {-x, -y};
    }

    Pos operator*(float other) const {
        return {x * other, y * other};
    }

    bool operator==(const Pos& other) const {
        // Consider using a tolerance for floating-point comparisons
        constexpr float epsilon = 1e-6f;
        return std::abs(x - other.x) <= epsilon && std::abs(y - other.y) <= epsilon;
    }

    // Copy constructor
    Pos(const Pos& other) = default;

    // Copy assignment operator
    Pos& operator=(const Pos& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    // Move constructor
    Pos(Pos&& other) noexcept : x(other.x), y(other.y) {}

    // Move assignment operator
    Pos& operator=(Pos&& other) noexcept {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    [[nodiscard]] Pos copy() const { return {x, y}; }
};

#endif //POSITION_H