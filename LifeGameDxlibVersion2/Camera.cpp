#include "Camera.h"

#include <algorithm>
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

int Camera::clamp(int value, int minimum, int maximum) noexcept {
    return std::max(minimum, std::min(value, maximum));
}

int Camera::visibleColumns() const noexcept {
    return (screenWidth_ + cellSize_ - 1) / cellSize_;
}

int Camera::visibleRows() const noexcept {
    return (screenHeight_ + cellSize_ - 1) / cellSize_;
}

void Camera::setBoundaryMode(BoundaryMode mode) noexcept {
    boundaryMode_ = mode;
    normalizePosition();
    endPan();
}

void Camera::normalizePosition() noexcept {
    if (boundaryMode_ == BoundaryMode::Toroidal) {
        x_ = wrap(x_, boardWidth_);
        y_ = wrap(y_, boardHeight_);
        return;
    }

    const int maxX = std::max(0, boardWidth_ - visibleColumns());
    const int maxY = std::max(0, boardHeight_ - visibleRows());
    x_ = clamp(x_, 0, maxX);
    y_ = clamp(y_, 0, maxY);
}

void Camera::move(int deltaX, int deltaY) noexcept {
    x_ += deltaX;
    y_ += deltaY;
    normalizePosition();
}

void Camera::panByPixels(int deltaScreenX, int deltaScreenY) noexcept {
    panRemainderX_ -= deltaScreenX;
    panRemainderY_ -= deltaScreenY;

    const int deltaCellsX = panRemainderX_ / cellSize_;
    const int deltaCellsY = panRemainderY_ / cellSize_;

    panRemainderX_ -= deltaCellsX * cellSize_;
    panRemainderY_ -= deltaCellsY * cellSize_;

    move(deltaCellsX, deltaCellsY);
}

void Camera::endPan() noexcept {
    panRemainderX_ = 0;
    panRemainderY_ = 0;
}

bool Camera::zoomIn(int maxCellSize) noexcept {
    if (cellSize_ >= maxCellSize) return false;
    cellSize_ *= 2;
    normalizePosition();
    endPan();
    return true;
}

bool Camera::zoomOut(int minCellSize) noexcept {
    if (cellSize_ <= minCellSize) return false;
    cellSize_ /= 2;
    normalizePosition();
    endPan();
    return true;
}

bool Camera::zoomInAt(int screenX, int screenY, int maxCellSize) noexcept {
    if (cellSize_ >= maxCellSize) return false;
    return zoomAt(screenX, screenY, cellSize_ * 2);
}

bool Camera::zoomOutAt(int screenX, int screenY, int minCellSize) noexcept {
    if (cellSize_ <= minCellSize) return false;
    return zoomAt(screenX, screenY, cellSize_ / 2);
}

bool Camera::zoomAt(int screenX, int screenY, int newCellSize) noexcept {
    if (newCellSize <= 0 || newCellSize == cellSize_) return false;

    const auto [anchorBoardX, anchorBoardY] = screenToBoard(screenX, screenY);
    cellSize_ = newCellSize;
    x_ = anchorBoardX - screenX / cellSize_;
    y_ = anchorBoardY - screenY / cellSize_;
    normalizePosition();
    endPan();
    return true;
}

int Camera::boardXAtColumn(int column) const noexcept {
    const int x = x_ + column;
    return boundaryMode_ == BoundaryMode::Toroidal ? wrap(x, boardWidth_) : x;
}

int Camera::boardYAtRow(int row) const noexcept {
    const int y = y_ + row;
    return boundaryMode_ == BoundaryMode::Toroidal ? wrap(y, boardHeight_) : y;
}

std::pair<int, int> Camera::screenToBoard(int screenX, int screenY) const noexcept {
    return {
        boardXAtColumn(screenX / cellSize_),
        boardYAtRow(screenY / cellSize_)
    };
}
