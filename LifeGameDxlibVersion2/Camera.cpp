#include "Camera.h"

#include <cassert>

Camera::Camera(
    int boardWidth,
    int boardHeight,
    int screenWidth,
    int screenHeight,
    int cellSize)
    : boardWidth_(boardWidth),
      boardHeight_(boardHeight),
      screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      x_(0),
      y_(0),
      cellSize_(cellSize) {
    assert(boardWidth_ > 0);
    assert(boardHeight_ > 0);
    assert(screenWidth_ > 0);
    assert(screenHeight_ > 0);
    assert(cellSize_ > 0);
}

int Camera::wrap(int value, int size) noexcept {
    value %= size;
    return value < 0 ? value + size : value;
}

int Camera::visibleColumns() const noexcept {
    return (screenWidth_ + cellSize_ - 1) / cellSize_;
}

int Camera::visibleRows() const noexcept {
    return (screenHeight_ + cellSize_ - 1) / cellSize_;
}

void Camera::move(int deltaX, int deltaY) noexcept {
    x_ = wrap(x_ + deltaX, boardWidth_);
    y_ = wrap(y_ + deltaY, boardHeight_);
}

bool Camera::zoomIn(int maxCellSize) noexcept {
    if (cellSize_ >= maxCellSize) {
        return false;
    }

    cellSize_ *= 2;
    return true;
}

bool Camera::zoomOut(int minCellSize) noexcept {
    if (cellSize_ <= minCellSize) {
        return false;
    }

    cellSize_ /= 2;
    return true;
}

int Camera::boardXAtColumn(int column) const noexcept {
    return wrap(x_ + column, boardWidth_);
}

int Camera::boardYAtRow(int row) const noexcept {
    return wrap(y_ + row, boardHeight_);
}

std::pair<int, int> Camera::screenToBoard(int screenX, int screenY) const noexcept {
    return {
        boardXAtColumn(screenX / cellSize_),
        boardYAtRow(screenY / cellSize_)
    };
}
