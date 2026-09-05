#include "DxLib.h"
#include "LifeBoard.h"
#include "LifeRenderer.h"

namespace {
constexpr int ScreenWidth = 1024;
constexpr int ScreenHeight = 1024;
constexpr int BoardWidth = 1536;
constexpr int BoardHeight = 1536;

void editBoardWithMouse(LifeBoard& board) {
    int mouseX = 0;
    int mouseY = 0;
    GetMousePoint(&mouseX, &mouseY);

    if (mouseX < 0 || mouseY < 0 ||
        mouseX >= ScreenWidth || mouseY >= ScreenHeight ||
        mouseX >= board.width() || mouseY >= board.height()) {
        return;
    }

    const int mouseInput = GetMouseInput();

    if ((mouseInput & MOUSE_INPUT_LEFT) != 0) {
        board.setAlive(mouseX, mouseY, true);
    }

    if ((mouseInput & MOUSE_INPUT_RIGHT) != 0) {
        board.setAlive(mouseX, mouseY, false);
    }
}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    SetMainWindowText("LifeGameDxlibVersion2");
    ChangeWindowMode(TRUE);
    SetGraphMode(ScreenWidth, ScreenHeight, 32);
    SetAlwaysRunFlag(TRUE);
    SetOutApplicationLogValidFlag(FALSE);

    if (DxLib_Init() == -1) {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    LifeBoard board(BoardWidth, BoardHeight);
    LifeRenderer renderer(ScreenWidth, ScreenHeight);
    board.randomize();

    bool paused = false;
    bool enterWasDown = false;

    while (ProcessMessage() == 0) {
        const bool enterIsDown = CheckHitKey(KEY_INPUT_RETURN) != 0;
        if (enterIsDown && !enterWasDown) {
            paused = !paused;
            SetMainWindowText(paused
                ? "LifeGameDxlibVersion2 - Paused"
                : "LifeGameDxlibVersion2");
        }
        enterWasDown = enterIsDown;

        if (paused) {
            editBoardWithMouse(board);
        } else {
            board.step();
        }

        ClearDrawScreen();
        renderer.draw(board);
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
