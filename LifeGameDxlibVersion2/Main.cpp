#include "DxLib.h"
#include "LifeBoard.h"
#include "LifeRenderer.h"

namespace {
constexpr int ScreenWidth = 1024;
constexpr int ScreenHeight = 1024;
constexpr int BoardWidth = 1536;
constexpr int BoardHeight = 1536;
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

    while (ProcessMessage() == 0) {
        board.step();

        ClearDrawScreen();
        renderer.draw(board);
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
