#pragma once

class LifeBoard;

class LifeRenderer {
public:
    LifeRenderer(int screenWidth, int screenHeight);

    void draw(const LifeBoard& board, int cameraX, int cameraY, int cellSize) const;

private:
    int screenWidth_;
    int screenHeight_;
    unsigned int aliveColor_;
};
