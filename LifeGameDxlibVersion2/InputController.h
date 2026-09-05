#pragma once

struct InputFrame {
    bool togglePause = false;
    bool clearBoard = false;
    bool randomizeBoard = false;
    bool save = false;
    bool load = false;
    bool singleStep = false;
    bool toggleGrid = false;
    bool zoomIn = false;
    bool zoomOut = false;
    bool speedUp = false;
    bool speedDown = false;

    int cameraDeltaX = 0;
    int cameraDeltaY = 0;

    int mouseX = 0;
    int mouseY = 0;
    int mouseDeltaX = 0;
    int mouseDeltaY = 0;
    int mouseWheel = 0;
    bool mouseLeftDown = false;
    bool mouseRightDown = false;
    bool mouseMiddleDown = false;
};

class InputController {
public:
    InputFrame update();

private:
    bool enterWasDown_ = false;
    bool deleteWasDown_ = false;
    bool rWasDown_ = false;
    bool saveWasDown_ = false;
    bool loadWasDown_ = false;
    bool spaceWasDown_ = false;
    bool gWasDown_ = false;
    bool plusWasDown_ = false;
    bool minusWasDown_ = false;
    bool pageUpWasDown_ = false;
    bool pageDownWasDown_ = false;
    bool mousePositionInitialized_ = false;
    int previousMouseX_ = 0;
    int previousMouseY_ = 0;
};
