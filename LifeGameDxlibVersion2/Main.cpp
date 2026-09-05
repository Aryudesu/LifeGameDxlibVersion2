#include "DxLib.h"
#include "AppConfig.h"
#include "LifeGameApplication.h"

#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    ChangeWindowMode(TRUE);
    SetGraphMode(
        AppConfig::ScreenWidth,
        AppConfig::ScreenHeight,
        AppConfig::ScreenColorDepth);
    SetAlwaysRunFlag(TRUE);
    SetOutApplicationLogValidFlag(FALSE);

    if (DxLib_Init() == -1) {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    LifeGameApplication application;
    const int result = application.run();

    DxLib_End();
    return result;
}
