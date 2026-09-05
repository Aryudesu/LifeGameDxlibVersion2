#include "LifeBoard.h"

#include <algorithm>
#include <cassert>

LifeBoard::LifeBoard(int width, int height)
    : width_(width),
      height_(height),
      wordsPerRow_((width + BitsPerWord - 1) / BitsPerWord),
      current_(static_cast<std::size_t>(wordsPerRow_) * height_),
      next_(static_cast<std::size_t>(wordsPerRow_) * height_),
      randomEngine_(std::random_device{}()) {
    assert(width_ > 0);
    assert(height_ > 0);
}

int LifeBoard::wrapX(int x) const noexcept {
    x %= width_;
    return x < 0 ? x + width_ : x;
}

int LifeBoard::wrapY(int y) const noexcept {
    y %= height_;
    return y < 0 ? y + height_ : y;
}

std::size_t LifeBoard::wordIndex(int x, int y) const noexcept {
    return static_cast<std::size_t>(y) * wordsPerRow_ + (x / BitsPerWord);
}

LifeBoard::Word LifeBoard::bitMask(int x) const noexcept {
    return Word{1} << (x % BitsPerWord);
}

bool LifeBoard::isAlive(int x, int y) const noexcept {
    x = wrapX(x);
    y = wrapY(y);
    return (current_[wordIndex(x, y)] & bitMask(x)) != 0;
}

void LifeBoard::setAlive(int x, int y, bool alive) noexcept {
    x = wrapX(x);
    y = wrapY(y);
    auto& word = current_[wordIndex(x, y)];
    const Word mask = bitMask(x);

    if (alive) {
        word |= mask;
    } else {
        word &= ~mask;
    }
}

void LifeBoard::clear() noexcept {
    std::fill(current_.begin(), current_.end(), Word{0});
    std::fill(next_.begin(), next_.end(), Word{0});
}

void LifeBoard::randomize(double aliveProbability) {
    std::bernoulli_distribution distribution(aliveProbability);
    clear();

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (distribution(randomEngine_)) {
                setAlive(x, y, true);
            }
        }
    }
}

int LifeBoard::countNeighbors(int x, int y) const noexcept {
    int count = 0;

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            if (isAlive(x + dx, y + dy)) {
                ++count;
            }
        }
    }

    return count;
}

void LifeBoard::clearUnusedBits() noexcept {
    const int usedBitsInLastWord = width_ % BitsPerWord;
    if (usedBitsInLastWord == 0) {
        return;
    }

    const Word validMask = (Word{1} << usedBitsInLastWord) - 1;
    for (int y = 0; y < height_; ++y) {
        next_[static_cast<std::size_t>(y) * wordsPerRow_ + (wordsPerRow_ - 1)] &= validMask;
    }
}

void LifeBoard::step() {
    std::fill(next_.begin(), next_.end(), Word{0});

    // 初回移植では正しさを優先し、BitBoard に格納したセルを素直に走査する。
    // 後続で 64 セル単位の bit-parallel 更新へ置き換える。
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            const int neighbors = countNeighbors(x, y);
            const bool nextAlive = neighbors == 3 || (neighbors == 2 && isAlive(x, y));
            if (nextAlive) {
                next_[wordIndex(x, y)] |= bitMask(x);
            }
        }
    }

    clearUnusedBits();
    current_.swap(next_);
}
