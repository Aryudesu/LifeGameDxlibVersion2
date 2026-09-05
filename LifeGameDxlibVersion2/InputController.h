#pragma once

struct InputFrame {
    bool togglePause = false;
    bool clearBoard = false;
    bool save = false;
    bool load = false;
    bool singleStep = false;
    bool toggleGrid = false;
    bool zoomIn = false;
    bool zoomOut = false;

    int cameraDeltaX = 0;
    int cameraDeltaY = 0;

    int mouseX = 0;
    int mouseY = 0;
    bool mouseLeftDown = false;
    bool mouseRightDown = false;
};

class InputController {
public:
    InputFrame update();

private:
    bool enterWasDown_ = false;
    bool deleteWasDown_ = false;
    bool saveWasDown_ = false;
    bool loadWasDown_ = false;
    bool spaceWasDown_ = false;
    bool gWasDown_ = false;
    bool plusWasDown_ = false;
    bool minusWasDown_ = false;
};
