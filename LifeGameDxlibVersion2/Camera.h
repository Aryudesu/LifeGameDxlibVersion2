#pragma once

#include <utility>

class Camera {
public:
    Camera(int boardWidth, int boardHeight, int screenWidth, int screenHeight, int cellSize);

    int x() const noexcept { return x_; }
    int y() const noexcept { return y_; }
    int cellSize() const noexcept { return cellSize_; }

    int visibleColumns() const noexcept;
    int visibleRows() const noexcept;

    void move(int deltaX, int deltaY) noexcept;
    bool zoomIn(int maxCellSize) noexcept;
    bool zoomOut(int minCellSize) noexcept;

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

    static int wrap(int value, int size) noexcept;
};
