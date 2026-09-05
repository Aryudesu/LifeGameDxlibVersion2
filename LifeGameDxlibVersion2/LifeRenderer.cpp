#include "LifeRenderer.h"

#include "DxLib.h"
#include "LifeBoard.h"

#include <algorithm>

LifeRenderer::LifeRenderer(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      aliveColor_(GetColor(0, 255, 0)) {
}

void LifeRenderer::draw(const LifeBoard& board, int cameraX, int cameraY, int cellSize) const {
    const int visibleColumns = (screenWidth_ + cellSize - 1) / cellSize;
    const int visibleRows = (screenHeight_ + cellSize - 1) / cellSize;
    const int endX = std::min(board.width(), cameraX + visibleColumns);
    const int endY = std::min(board.height(), cameraY + visibleRows);

    for (int y = cameraY; y < endY; ++y) {
        const int screenY = (y - cameraY) * cellSize;
        for (int x = cameraX; x < endX; ++x) {
            if (!board.isAlive(x, y)) {
                continue;
            }

            const int screenX = (x - cameraX) * cellSize;
            if (cellSize == 1) {
                DrawPixel(screenX, screenY, aliveColor_);
            } else {
                DrawBox(
                    screenX,
                    screenY,
                    screenX + cellSize - 1,
                    screenY + cellSize - 1,
                    aliveColor_,
                    TRUE);
            }
        }
    }
}
