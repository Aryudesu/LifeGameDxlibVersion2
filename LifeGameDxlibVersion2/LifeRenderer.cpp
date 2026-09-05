#include "LifeRenderer.h"

#include "AppConfig.h"
#include "Camera.h"
#include "DxLib.h"
#include "LifeBoard.h"

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
    const Camera& camera,
    bool showGrid) const {
    const int cellSize = camera.cellSize();
    const int visibleColumns = camera.visibleColumns();
    const int visibleRows = camera.visibleRows();

    for (int row = 0; row < visibleRows; ++row) {
        const int boardY = camera.boardYAtRow(row);
        const int screenY = row * cellSize;

        for (int column = 0; column < visibleColumns; ++column) {
            const int boardX = camera.boardXAtColumn(column);
            if (!board.isAlive(boardX, boardY)) {
                continue;
            }

            const int screenX = column * cellSize;
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

    for (int x = 0; x <= screenWidth_; x += cellSize) {
        DrawLine(x, 0, x, screenHeight_, gridColor_);
    }
    for (int y = 0; y <= screenHeight_; y += cellSize) {
        DrawLine(0, y, screenWidth_, y, gridColor_);
    }
}
