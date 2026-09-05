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

    if (width_ % BitsPerWord == 0) {
        const auto addBits = [](Word value, Word& ones, Word& twos, Word& fours, Word& eights) {
            Word carry = ones & value;
            ones ^= value;

            Word nextCarry = twos & carry;
            twos ^= carry;
            carry = nextCarry;

            nextCarry = fours & carry;
            fours ^= carry;
            carry = nextCarry;

            eights ^= carry;
        };

        for (int y = 0; y < height_; ++y) {
            const int northY = (y == 0) ? height_ - 1 : y - 1;
            const int southY = (y + 1 == height_) ? 0 : y + 1;

            const std::size_t northBase = static_cast<std::size_t>(northY) * wordsPerRow_;
            const std::size_t centerBase = static_cast<std::size_t>(y) * wordsPerRow_;
            const std::size_t southBase = static_cast<std::size_t>(southY) * wordsPerRow_;

            for (int wordX = 0; wordX < wordsPerRow_; ++wordX) {
                const int leftWordX = (wordX == 0) ? wordsPerRow_ - 1 : wordX - 1;
                const int rightWordX = (wordX + 1 == wordsPerRow_) ? 0 : wordX + 1;

                const Word northLeft = current_[northBase + leftWordX];
                const Word north = current_[northBase + wordX];
                const Word northRight = current_[northBase + rightWordX];

                const Word centerLeft = current_[centerBase + leftWordX];
                const Word center = current_[centerBase + wordX];
                const Word centerRight = current_[centerBase + rightWordX];

                const Word southLeft = current_[southBase + leftWordX];
                const Word south = current_[southBase + wordX];
                const Word southRight = current_[southBase + rightWordX];

                const Word northWest = (north << 1) | (northLeft >> 63);
                const Word northEast = (north >> 1) | (northRight << 63);
                const Word west = (center << 1) | (centerLeft >> 63);
                const Word east = (center >> 1) | (centerRight << 63);
                const Word southWest = (south << 1) | (southLeft >> 63);
                const Word southEast = (south >> 1) | (southRight << 63);

                Word ones = 0;
                Word twos = 0;
                Word fours = 0;
                Word eights = 0;

                addBits(northWest, ones, twos, fours, eights);
                addBits(north, ones, twos, fours, eights);
                addBits(northEast, ones, twos, fours, eights);
                addBits(west, ones, twos, fours, eights);
                addBits(east, ones, twos, fours, eights);
                addBits(southWest, ones, twos, fours, eights);
                addBits(south, ones, twos, fours, eights);
                addBits(southEast, ones, twos, fours, eights);

                const Word lowCountMask = ~(eights | fours);
                const Word exactlyTwo = lowCountMask & twos & ~ones;
                const Word exactlyThree = lowCountMask & twos & ones;

                next_[centerBase + wordX] = exactlyThree | (center & exactlyTwo);
            }
        }

        current_.swap(next_);
        return;
    }

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
