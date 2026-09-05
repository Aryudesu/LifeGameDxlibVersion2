#include "DxLib.h"
#include "AppConfig.h"
#include "InputController.h"
#include "LifeBoard.h"
#include "LifeFile.h"
#include "LifeRenderer.h"

#include <algorithm>
#include <commdlg.h>
#include <cstring>
#include <string>
#include <windows.h>

#pragma comment(lib, "Comdlg32.lib")

namespace {
void setWindowTitle(bool paused) {
    SetMainWindowText(paused
        ? AppConfig::PausedWindowTitle
        : AppConfig::WindowTitle);
}

bool chooseSavePath(std::string& path) {
    char fileName[MAX_PATH] = {};
    strcpy_s(fileName, AppConfig::SaveFileDefaultName);

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetMainWindowHandle();
    dialog.lpstrFilter = AppConfig::SaveFileFilter;
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = AppConfig::SaveFileExtension;
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&dialog) == FALSE) {
        return false;
    }

    path = fileName;
    return true;
}

bool chooseLoadPath(std::string& path) {
    char fileName[MAX_PATH] = {};
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetMainWindowHandle();
    dialog.lpstrFilter = AppConfig::SaveFileFilter;
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = AppConfig::SaveFileExtension;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&dialog) == FALSE) {
        return false;
    }

    path = fileName;
    return true;
}

void showFileError(const std::string& message) {
    MessageBoxA(
        GetMainWindowHandle(),
        message.c_str(),
        AppConfig::WindowTitle,
        MB_OK | MB_ICONERROR);
}

void clampCamera(const LifeBoard& board, int cellSize, int& cameraX, int& cameraY) {
    const int visibleColumns = (AppConfig::ScreenWidth + cellSize - 1) / cellSize;
    const int visibleRows = (AppConfig::ScreenHeight + cellSize - 1) / cellSize;
    const int maxCameraX = std::max(0, board.width() - visibleColumns);
    const int maxCameraY = std::max(0, board.height() - visibleRows);

    cameraX = std::clamp(cameraX, 0, maxCameraX);
    cameraY = std::clamp(cameraY, 0, maxCameraY);
}

void updateCamera(
    const LifeBoard& board,
    const InputFrame& input,
    int cellSize,
    int& cameraX,
    int& cameraY) {
    cameraX += input.cameraDeltaX * AppConfig::CameraMoveSpeed;
    cameraY += input.cameraDeltaY * AppConfig::CameraMoveSpeed;
    clampCamera(board, cellSize, cameraX, cameraY);
}

void editBoardWithMouse(
    LifeBoard& board,
    const InputFrame& input,
    int cameraX,
    int cameraY,
    int cellSize) {
    if (input.mouseX < 0 || input.mouseY < 0 ||
        input.mouseX >= AppConfig::ScreenWidth ||
        input.mouseY >= AppConfig::ScreenHeight) {
        return;
    }

    const int boardX = cameraX + input.mouseX / cellSize;
    const int boardY = cameraY + input.mouseY / cellSize;
    if (boardX < 0 || boardY < 0 ||
        boardX >= board.width() || boardY >= board.height()) {
        return;
    }

    if (input.mouseLeftDown) {
        board.setAlive(boardX, boardY, true);
    }

    if (input.mouseRightDown) {
        board.setAlive(boardX, boardY, false);
    }
}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    SetMainWindowText(AppConfig::WindowTitle);
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

    LifeBoard board(AppConfig::BoardWidth, AppConfig::BoardHeight);
    LifeRenderer renderer(AppConfig::ScreenWidth, AppConfig::ScreenHeight);
    InputController inputController;
    board.randomize();

    bool paused = false;
    bool showGrid = false;
    int cameraX = 0;
    int cameraY = 0;
    int cellSize = AppConfig::MinCellSize;

    while (ProcessMessage() == 0) {
        const InputFrame input = inputController.update();

        if (input.togglePause) {
            paused = !paused;
            setWindowTitle(paused);
        }

        if (input.clearBoard) {
            board.clear();
            paused = true;
            setWindowTitle(paused);
        }

        if (input.save) {
            std::string path;
            if (chooseSavePath(path)) {
                std::string errorMessage;
                if (!LifeFile::save(board, path, errorMessage)) {
                    showFileError(errorMessage);
                }
            }
        }

        if (input.load) {
            std::string path;
            if (chooseLoadPath(path)) {
                std::string errorMessage;
                if (LifeFile::load(board, path, errorMessage)) {
                    paused = true;
                    setWindowTitle(paused);
                } else {
                    showFileError(errorMessage);
                }
            }
        }

        if (input.toggleGrid) {
            showGrid = !showGrid;
        }

        if (input.zoomIn && cellSize < AppConfig::MaxCellSize) {
            cellSize *= 2;
            clampCamera(board, cellSize, cameraX, cameraY);
        }

        if (input.zoomOut && cellSize > AppConfig::MinCellSize) {
            cellSize /= 2;
            clampCamera(board, cellSize, cameraX, cameraY);
        }

        updateCamera(board, input, cellSize, cameraX, cameraY);

        if (paused) {
            editBoardWithMouse(board, input, cameraX, cameraY, cellSize);
            if (input.singleStep) {
                board.step();
            }
        } else {
            board.step();
        }

        ClearDrawScreen();
        renderer.draw(board, cameraX, cameraY, cellSize, showGrid);
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
