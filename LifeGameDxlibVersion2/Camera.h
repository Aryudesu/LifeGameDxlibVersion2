#pragma once

#include "BoundaryMode.h"

#include <utility>

class Camera {
public:
    Camera(int boardWidth, int boardHeight, int screenWidth, int screenHeight, int cellSize);

    int x() const noexcept { return x_; }
    int y() const noexcept { return y_; }
    int cellSize() const noexcept { return cellSize_; }
    BoundaryMode boundaryMode() const noexcept { return boundaryMode_; }
    void setBoundaryMode(BoundaryMode mode) noexcept;

    int visibleColumns() const noexcept;
    int visibleRows() const noexcept;

    void move(int deltaX, int deltaY) noexcept;
    void panByPixels(int deltaScreenX, int deltaScreenY) noexcept;
    void endPan() noexcept;

    bool zoomIn(int maxCellSize) noexcept;
    bool zoomOut(int minCellSize) noexcept;
    bool zoomInAt(int screenX, int screenY, int maxCellSize) noexcept;
    bool zoomOutAt(int screenX, int screenY, int minCellSize) noexcept;

    int boardXAtColumn(int column) const noexcept;
    int boardYAtRow(int row) const noexcept;
    std::pair<int, int> screenToBoard(int screenX, int screenY) const noexcept;

private:
    int boardWidth_;
    int boardHeight_;
    int screenWidth_;
    int screenHeight_;
    int x_;
    int y_;
    int cellSize_;
    int panRemainderX_ = 0;
    int panRemainderY_ = 0;
    BoundaryMode boundaryMode_ = BoundaryMode::Dead;

    static int wrap(int value, int size) noexcept;
    static int clamp(int value, int minimum, int maximum) noexcept;
    void normalizePosition() noexcept;
    bool zoomAt(int screenX, int screenY, int newCellSize) noexcept;
};
