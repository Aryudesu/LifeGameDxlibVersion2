#pragma once

#include "BoundaryMode.h"

#include <cstdint>
#include <random>
#include <vector>

class LifeBoard {
public:
    using Word = std::uint64_t;
    static constexpr int BitsPerWord = 64;

    LifeBoard(int width, int height);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    BoundaryMode boundaryMode() const noexcept { return boundaryMode_; }
    void setBoundaryMode(BoundaryMode mode) noexcept { boundaryMode_ = mode; }

    bool isAlive(int x, int y) const noexcept;
    void setAlive(int x, int y, bool alive) noexcept;
    void clear() noexcept;
    void randomize(double aliveProbability = 1.0 / 6.0);
    std::uint64_t aliveCellCount() const noexcept;
    void step();

private:
    int width_;
    int height_;
    int wordsPerRow_;
    std::vector<Word> current_;
    std::vector<Word> next_;
    std::mt19937 randomEngine_;
    BoundaryMode boundaryMode_ = BoundaryMode::Dead;

    bool inBounds(int x, int y) const noexcept;
    int wrapX(int x) const noexcept;
    int wrapY(int y) const noexcept;
    std::size_t wordIndex(int x, int y) const noexcept;
    Word bitMask(int x) const noexcept;
    int countNeighbors(int x, int y) const noexcept;
    void clearUnusedBits() noexcept;
};
