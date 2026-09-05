#pragma once

#include "Camera.h"
#include "InputController.h"
#include "LifeBoard.h"
#include "LifeRenderer.h"

class LifeGameApplication {
public:
    LifeGameApplication();

    int run();

private:
    void update(const InputFrame& input);
    void draw() const;
    void setPaused(bool paused);
    void saveBoard();
    void loadBoard();
    void editBoardWithMouse(const InputFrame& input);

    LifeBoard board_;
    LifeRenderer renderer_;
    Camera camera_;
    InputController inputController_;
    bool paused_ = false;
    bool showGrid_ = false;
};
