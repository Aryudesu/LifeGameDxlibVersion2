#pragma once

class Camera;
class LifeBoard;

class LifeRenderer {
public:
    LifeRenderer(int screenWidth, int screenHeight);

    void draw(
        const LifeBoard& board,
        const Camera& camera,
        bool showGrid) const;

private:
    int screenWidth_;
    int screenHeight_;
    unsigned int aliveColor_;
    unsigned int gridColor_;
};
