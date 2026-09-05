#pragma once

class LifeBoard;

class LifeRenderer {
public:
    LifeRenderer(int screenWidth, int screenHeight);

    void draw(const LifeBoard& board) const;

private:
    int screenWidth_;
    int screenHeight_;
    unsigned int aliveColor_;
};
