#include "LifeRenderer.h"

#include "DxLib.h"
#include "LifeBoard.h"

#include <algorithm>

LifeRenderer::LifeRenderer(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      aliveColor_(GetColor(0, 255, 0)) {
}

void LifeRenderer::draw(const LifeBoard& board) const {
    const int drawWidth = std::min(screenWidth_, board.width());
    const int drawHeight = std::min(screenHeight_, board.height());

    for (int y = 0; y < drawHeight; ++y) {
        for (int x = 0; x < drawWidth; ++x) {
            if (board.isAlive(x, y)) {
                DrawPixel(x, y, aliveColor_);
            }
        }
    }
}
