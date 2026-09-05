#include "LifeRenderer.h"

#include "AppConfig.h"
#include "DxLib.h"
#include "LifeBoard.h"

#include <algorithm>

LifeRenderer::LifeRenderer(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      aliveColor_(GetColor(
          AppConfig::AliveColorR,
          AppConfig::AliveColorG,
          AppConfig::AliveColorB)),
      gridColor_(GetColor(
          AppConfig::GridColorR,
          AppConfig::GridColorG,
          AppConfig::GridColorB)) {
}

void LifeRenderer::draw(
    const LifeBoard& board,
    int cameraX,
    int cameraY,
    int cellSize,
    bool showGrid) const {
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

    if (!showGrid || cellSize == 1) {
        return;
    }

    const int drawWidth = std::min(screenWidth_, (endX - cameraX) * cellSize);
    const int drawHeight = std::min(screenHeight_, (endY - cameraY) * cellSize);

    for (int x = 0; x <= drawWidth; x += cellSize) {
        DrawLine(x, 0, x, drawHeight, gridColor_);
    }
    for (int y = 0; y <= drawHeight; y += cellSize) {
        DrawLine(0, y, drawWidth, y, gridColor_);
    }
}
