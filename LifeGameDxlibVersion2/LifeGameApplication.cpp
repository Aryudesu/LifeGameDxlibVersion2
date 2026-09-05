#include "LifeGameApplication.h"

#include "AppConfig.h"
#include "DxLib.h"
#include "FileDialog.h"
#include "LifeFile.h"

#include <string>

LifeGameApplication::LifeGameApplication()
    : board_(AppConfig::BoardWidth, AppConfig::BoardHeight),
      renderer_(AppConfig::ScreenWidth, AppConfig::ScreenHeight),
      camera_(
          AppConfig::BoardWidth,
          AppConfig::BoardHeight,
          AppConfig::ScreenWidth,
          AppConfig::ScreenHeight,
          AppConfig::MinCellSize) {
    board_.randomize();
    SetMainWindowText(AppConfig::WindowTitle);
}

int LifeGameApplication::run() {
    while (ProcessMessage() == 0) {
        const InputFrame input = inputController_.update();
        update(input);
        draw();
    }

    return 0;
}

void LifeGameApplication::update(const InputFrame& input) {
    if (input.togglePause) {
        setPaused(!paused_);
    }

    if (input.clearBoard) {
        board_.clear();
        setPaused(true);
    }

    if (input.save) {
        saveBoard();
    }

    if (input.load) {
        loadBoard();
    }

    if (input.toggleGrid) {
        showGrid_ = !showGrid_;
    }

    if (input.zoomIn) {
        camera_.zoomIn(AppConfig::MaxCellSize);
    }

    if (input.zoomOut) {
        camera_.zoomOut(AppConfig::MinCellSize);
    }

    camera_.move(
        input.cameraDeltaX * AppConfig::CameraMoveSpeed,
        input.cameraDeltaY * AppConfig::CameraMoveSpeed);

    if (paused_) {
        editBoardWithMouse(input);
        if (input.singleStep) {
            board_.step();
        }
    } else {
        board_.step();
    }
}

void LifeGameApplication::draw() const {
    ClearDrawScreen();
    renderer_.draw(board_, camera_, showGrid_);
    ScreenFlip();
}

void LifeGameApplication::setPaused(bool paused) {
    paused_ = paused;
    SetMainWindowText(paused_
        ? AppConfig::PausedWindowTitle
        : AppConfig::WindowTitle);
}

void LifeGameApplication::saveBoard() {
    std::string path;
    if (!FileDialog::chooseSavePath(path)) {
        return;
    }

    std::string errorMessage;
    if (!LifeFile::save(board_, path, errorMessage)) {
        FileDialog::showError(errorMessage);
    }
}

void LifeGameApplication::loadBoard() {
    std::string path;
    if (!FileDialog::chooseLoadPath(path)) {
        return;
    }

    std::string errorMessage;
    if (LifeFile::load(board_, path, errorMessage)) {
        setPaused(true);
        return;
    }

    FileDialog::showError(errorMessage);
}

void LifeGameApplication::editBoardWithMouse(const InputFrame& input) {
    if (input.mouseX < 0 || input.mouseY < 0 ||
        input.mouseX >= AppConfig::ScreenWidth ||
        input.mouseY >= AppConfig::ScreenHeight) {
        return;
    }

    const auto [boardX, boardY] = camera_.screenToBoard(input.mouseX, input.mouseY);

    if (input.mouseLeftDown) {
        board_.setAlive(boardX, boardY, true);
    }

    if (input.mouseRightDown) {
        board_.setAlive(boardX, boardY, false);
    }
}
