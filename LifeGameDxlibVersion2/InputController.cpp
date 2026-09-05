#include "InputController.h"

#include "DxLib.h"

namespace {
bool pressedThisFrame(bool isDown, bool& wasDown) {
    const bool pressed = isDown && !wasDown;
    wasDown = isDown;
    return pressed;
}
}

InputFrame InputController::update() {
    InputFrame input;

    const bool enterIsDown = CheckHitKey(KEY_INPUT_RETURN) != 0;
    input.togglePause = pressedThisFrame(enterIsDown, enterWasDown_);

    const bool deleteIsDown = CheckHitKey(KEY_INPUT_DELETE) != 0;
    input.clearBoard = pressedThisFrame(deleteIsDown, deleteWasDown_);

    const bool controlIsDown =
        CheckHitKey(KEY_INPUT_LCONTROL) != 0 ||
        CheckHitKey(KEY_INPUT_RCONTROL) != 0;

    const bool saveIsDown = controlIsDown && CheckHitKey(KEY_INPUT_S) != 0;
    input.save = pressedThisFrame(saveIsDown, saveWasDown_);

    const bool loadIsDown = controlIsDown && CheckHitKey(KEY_INPUT_L) != 0;
    input.load = pressedThisFrame(loadIsDown, loadWasDown_);

    const bool spaceIsDown = CheckHitKey(KEY_INPUT_SPACE) != 0;
    input.singleStep = pressedThisFrame(spaceIsDown, spaceWasDown_);

    const bool gIsDown = CheckHitKey(KEY_INPUT_G) != 0;
    input.toggleGrid = pressedThisFrame(gIsDown, gWasDown_);

    const bool shiftIsDown =
        CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
        CheckHitKey(KEY_INPUT_RSHIFT) != 0;
    const bool plusIsDown =
        CheckHitKey(KEY_INPUT_ADD) != 0 ||
        (shiftIsDown && CheckHitKey(KEY_INPUT_SEMICOLON) != 0);
    input.zoomIn = pressedThisFrame(plusIsDown, plusWasDown_);

    const bool minusIsDown =
        CheckHitKey(KEY_INPUT_SUBTRACT) != 0 ||
        CheckHitKey(KEY_INPUT_MINUS) != 0;
    input.zoomOut = pressedThisFrame(minusIsDown, minusWasDown_);

    const bool pageUpIsDown = CheckHitKey(KEY_INPUT_PGUP) != 0;
    input.speedUp = pressedThisFrame(pageUpIsDown, pageUpWasDown_);

    const bool pageDownIsDown = CheckHitKey(KEY_INPUT_PGDN) != 0;
    input.speedDown = pressedThisFrame(pageDownIsDown, pageDownWasDown_);

    input.cameraDeltaX =
        (CheckHitKey(KEY_INPUT_RIGHT) != 0 ? 1 : 0) -
        (CheckHitKey(KEY_INPUT_LEFT) != 0 ? 1 : 0);
    input.cameraDeltaY =
        (CheckHitKey(KEY_INPUT_DOWN) != 0 ? 1 : 0) -
        (CheckHitKey(KEY_INPUT_UP) != 0 ? 1 : 0);

    GetMousePoint(&input.mouseX, &input.mouseY);
    if (mousePositionInitialized_) {
        input.mouseDeltaX = input.mouseX - previousMouseX_;
        input.mouseDeltaY = input.mouseY - previousMouseY_;
    } else {
        mousePositionInitialized_ = true;
    }
    previousMouseX_ = input.mouseX;
    previousMouseY_ = input.mouseY;

    input.mouseWheel = GetMouseWheelRotVol();

    const int mouseInput = GetMouseInput();
    input.mouseLeftDown = (mouseInput & MOUSE_INPUT_LEFT) != 0;
    input.mouseRightDown = (mouseInput & MOUSE_INPUT_RIGHT) != 0;
    input.mouseMiddleDown = (mouseInput & MOUSE_INPUT_MIDDLE) != 0;

    return input;
}
