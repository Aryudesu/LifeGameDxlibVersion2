#include "DxLib.h"
#include "LifeBoard.h"
#include "LifeFile.h"
#include "LifeRenderer.h"

#include <algorithm>
#include <commdlg.h>
#include <string>
#include <windows.h>

#pragma comment(lib, "Comdlg32.lib")

namespace {
constexpr int ScreenWidth = 1024;
constexpr int ScreenHeight = 1024;
constexpr int BoardWidth = 1536;
constexpr int BoardHeight = 1536;
constexpr int MinCellSize = 1;
constexpr int MaxCellSize = 32;
constexpr int CameraMoveSpeed = 8;

void setWindowTitle(bool paused) {
    SetMainWindowText(paused
        ? "LifeGameDxlibVersion2 - Paused"
        : "LifeGameDxlibVersion2");
}

bool chooseSavePath(std::string& path) {
    char fileName[MAX_PATH] = "lifegame.ary2";
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetMainWindowHandle();
    dialog.lpstrFilter =
        "LifeGame v2 (*.ary2)\0*.ary2\0"
        "All Files (*.*)\0*.*\0";
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = "ary2";
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
    dialog.lpstrFilter =
        "LifeGame v2 (*.ary2)\0*.ary2\0"
        "All Files (*.*)\0*.*\0";
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = "ary2";
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
        "LifeGameDxlibVersion2",
        MB_OK | MB_ICONERROR);
}

void clampCamera(const LifeBoard& board, int cellSize, int& cameraX, int& cameraY) {
    const int visibleColumns = (ScreenWidth + cellSize - 1) / cellSize;
    const int visibleRows = (ScreenHeight + cellSize - 1) / cellSize;
    const int maxCameraX = std::max(0, board.width() - visibleColumns);
    const int maxCameraY = std::max(0, board.height() - visibleRows);

    cameraX = std::clamp(cameraX, 0, maxCameraX);
    cameraY = std::clamp(cameraY, 0, maxCameraY);
}

void updateCamera(const LifeBoard& board, int cellSize, int& cameraX, int& cameraY) {
    if (CheckHitKey(KEY_INPUT_LEFT) != 0) {
        cameraX -= CameraMoveSpeed;
    }
    if (CheckHitKey(KEY_INPUT_RIGHT) != 0) {
        cameraX += CameraMoveSpeed;
    }
    if (CheckHitKey(KEY_INPUT_UP) != 0) {
        cameraY -= CameraMoveSpeed;
    }
    if (CheckHitKey(KEY_INPUT_DOWN) != 0) {
        cameraY += CameraMoveSpeed;
    }

    clampCamera(board, cellSize, cameraX, cameraY);
}

void editBoardWithMouse(LifeBoard& board, int cameraX, int cameraY, int cellSize) {
    int mouseX = 0;
    int mouseY = 0;
    GetMousePoint(&mouseX, &mouseY);

    if (mouseX < 0 || mouseY < 0 ||
        mouseX >= ScreenWidth || mouseY >= ScreenHeight) {
        return;
    }

    const int boardX = cameraX + mouseX / cellSize;
    const int boardY = cameraY + mouseY / cellSize;
    if (boardX < 0 || boardY < 0 ||
        boardX >= board.width() || boardY >= board.height()) {
        return;
    }

    const int mouseInput = GetMouseInput();

    if ((mouseInput & MOUSE_INPUT_LEFT) != 0) {
        board.setAlive(boardX, boardY, true);
    }

    if ((mouseInput & MOUSE_INPUT_RIGHT) != 0) {
        board.setAlive(boardX, boardY, false);
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
    bool spaceWasDown = false;
    bool plusWasDown = false;
    bool minusWasDown = false;
    bool gWasDown = false;
    bool deleteWasDown = false;
    bool saveWasDown = false;
    bool loadWasDown = false;
    bool showGrid = false;
    int cameraX = 0;
    int cameraY = 0;
    int cellSize = 1;

    while (ProcessMessage() == 0) {
        const bool enterIsDown = CheckHitKey(KEY_INPUT_RETURN) != 0;
        if (enterIsDown && !enterWasDown) {
            paused = !paused;
            setWindowTitle(paused);
        }
        enterWasDown = enterIsDown;

        const bool deleteIsDown = CheckHitKey(KEY_INPUT_DELETE) != 0;
        if (deleteIsDown && !deleteWasDown) {
            board.clear();
            paused = true;
            setWindowTitle(paused);
        }
        deleteWasDown = deleteIsDown;

        const bool controlIsDown =
            CheckHitKey(KEY_INPUT_LCONTROL) != 0 ||
            CheckHitKey(KEY_INPUT_RCONTROL) != 0;

        const bool saveIsDown = controlIsDown && CheckHitKey(KEY_INPUT_S) != 0;
        if (saveIsDown && !saveWasDown) {
            std::string path;
            if (chooseSavePath(path)) {
                std::string errorMessage;
                if (!LifeFile::save(board, path, errorMessage)) {
                    showFileError(errorMessage);
                }
            }
        }
        saveWasDown = saveIsDown;

        const bool loadIsDown = controlIsDown && CheckHitKey(KEY_INPUT_L) != 0;
        if (loadIsDown && !loadWasDown) {
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
        loadWasDown = loadIsDown;

        const bool spaceIsDown = CheckHitKey(KEY_INPUT_SPACE) != 0;
        const bool singleStepRequested = paused && spaceIsDown && !spaceWasDown;
        spaceWasDown = spaceIsDown;

        const bool gIsDown = CheckHitKey(KEY_INPUT_G) != 0;
        if (gIsDown && !gWasDown) {
            showGrid = !showGrid;
        }
        gWasDown = gIsDown;

        const bool shiftIsDown =
            CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
            CheckHitKey(KEY_INPUT_RSHIFT) != 0;
        const bool plusIsDown =
            CheckHitKey(KEY_INPUT_ADD) != 0 ||
            (shiftIsDown && CheckHitKey(KEY_INPUT_SEMICOLON) != 0);
        if (plusIsDown && !plusWasDown && cellSize < MaxCellSize) {
            cellSize *= 2;
            clampCamera(board, cellSize, cameraX, cameraY);
        }
        plusWasDown = plusIsDown;

        const bool minusIsDown =
            CheckHitKey(KEY_INPUT_SUBTRACT) != 0 ||
            CheckHitKey(KEY_INPUT_MINUS) != 0;
        if (minusIsDown && !minusWasDown && cellSize > MinCellSize) {
            cellSize /= 2;
            clampCamera(board, cellSize, cameraX, cameraY);
        }
        minusWasDown = minusIsDown;

        updateCamera(board, cellSize, cameraX, cameraY);

        if (paused) {
            editBoardWithMouse(board, cameraX, cameraY, cellSize);
            if (singleStepRequested) {
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
