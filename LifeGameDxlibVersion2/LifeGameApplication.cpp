#include "LifeGameApplication.h"

#include "AppConfig.h"
#include "DxLib.h"
#include "FileDialog.h"
#include "LifeFile.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

LifeGameApplication::LifeGameApplication()
    : board_(AppConfig::BoardWidth, AppConfig::BoardHeight),
      renderer_(AppConfig::ScreenWidth, AppConfig::ScreenHeight),
      camera_(
          AppConfig::BoardWidth,
          AppConfig::BoardHeight,
          AppConfig::ScreenWidth,
          AppConfig::ScreenHeight,
          AppConfig::MinCellSize),
      simulationSpeedIndex_(AppConfig::DefaultSimulationSpeedIndex) {
    board_.randomize();
    SetMainWindowText(AppConfig::WindowTitle);
}

int LifeGameApplication::run() {
    using Clock = std::chrono::steady_clock;

    const auto targetFrameDuration = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / AppConfig::TargetFps));

    auto previousFrameTime = Clock::now();
    auto nextFrameTime = previousFrameTime;
    auto fpsSampleStart = previousFrameTime;
    int fpsFrameCount = 0;

    while (ProcessMessage() == 0) {
        const auto frameStart = Clock::now();
        const double elapsedSeconds =
            std::chrono::duration<double>(frameStart - previousFrameTime).count();
        previousFrameTime = frameStart;

        const InputFrame input = inputController_.update();
        update(input, elapsedSeconds);
        draw();

        ++fpsFrameCount;
        const auto fpsSampleEnd = Clock::now();
        const double fpsSampleSeconds =
            std::chrono::duration<double>(fpsSampleEnd - fpsSampleStart).count();
        if (fpsSampleSeconds >= AppConfig::FpsSampleSeconds) {
            fps_ = fpsFrameCount / fpsSampleSeconds;
            fpsFrameCount = 0;
            fpsSampleStart = fpsSampleEnd;
        }

        if (input.save || input.load) {
            const auto resetTime = Clock::now();
            previousFrameTime = resetTime;
            nextFrameTime = resetTime;
            fpsSampleStart = resetTime;
            fpsFrameCount = 0;
        }

        nextFrameTime += targetFrameDuration;
        const auto afterFrame = Clock::now();
        if (nextFrameTime > afterFrame) {
            std::this_thread::sleep_until(nextFrameTime);
        } else {
            nextFrameTime = afterFrame;
        }
    }

    return 0;
}

void LifeGameApplication::update(const InputFrame& input, double elapsedSeconds) {
    if (input.togglePause) {
        setPaused(!paused_);
    }

    if (input.clearBoard) {
        board_.clear();
        generation_ = 0;
        simulationAccumulator_ = 0.0;
        setPaused(true);
    }

    if (input.randomizeBoard) {
        board_.randomize();
        generation_ = 0;
        simulationAccumulator_ = 0.0;
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

    if (input.speedUp) {
        changeSimulationSpeed(1);
    }

    if (input.speedDown) {
        changeSimulationSpeed(-1);
    }

    if (input.zoomIn) {
        camera_.zoomIn(AppConfig::MaxCellSize);
    }

    if (input.zoomOut) {
        camera_.zoomOut(AppConfig::MinCellSize);
    }

    if (input.mouseWheel > 0) {
        for (int i = 0; i < input.mouseWheel; ++i) {
            if (!camera_.zoomInAt(
                    input.mouseX,
                    input.mouseY,
                    AppConfig::MaxCellSize)) {
                break;
            }
        }
    } else if (input.mouseWheel < 0) {
        for (int i = 0; i < -input.mouseWheel; ++i) {
            if (!camera_.zoomOutAt(
                    input.mouseX,
                    input.mouseY,
                    AppConfig::MinCellSize)) {
                break;
            }
        }
    }

    camera_.move(
        input.cameraDeltaX * AppConfig::CameraMoveSpeed,
        input.cameraDeltaY * AppConfig::CameraMoveSpeed);

    if (input.mouseMiddleDown) {
        camera_.panByPixels(input.mouseDeltaX, input.mouseDeltaY);
    } else {
        camera_.endPan();
    }

    if (paused_) {
        simulationAccumulator_ = 0.0;
        editBoardWithMouse(input);
        if (input.singleStep) {
            advanceGeneration();
        }
        return;
    }

    const double simulationSeconds = std::min(
        elapsedSeconds,
        AppConfig::MaxSimulationDeltaSeconds);
    simulationAccumulator_ += simulationSeconds * currentSimulationSpeed();

    const int generationsToAdvance = static_cast<int>(simulationAccumulator_);
    simulationAccumulator_ -= generationsToAdvance;

    for (int i = 0; i < generationsToAdvance; ++i) {
        advanceGeneration();
    }
}

void LifeGameApplication::draw() const {
    ClearDrawScreen();
    renderer_.draw(board_, camera_, showGrid_);

    DrawFormatString(
        AppConfig::HudX,
        AppConfig::HudY,
        GetColor(
            AppConfig::HudColorR,
            AppConfig::HudColorG,
            AppConfig::HudColorB),
        "Generation: %llu   FPS: %.1f   Speed: %d gen/s%s",
        static_cast<unsigned long long>(generation_),
        fps_,
        currentSimulationSpeed(),
        paused_ ? "   [Paused]" : "");

    ScreenFlip();
}

void LifeGameApplication::setPaused(bool paused) {
    paused_ = paused;
    simulationAccumulator_ = 0.0;
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
    if (!LifeFile::save(board_, generation_, path, errorMessage)) {
        FileDialog::showError(errorMessage);
    }
}

void LifeGameApplication::loadBoard() {
    std::string path;
    if (!FileDialog::chooseLoadPath(path)) {
        return;
    }

    std::string errorMessage;
    std::uint64_t loadedGeneration = 0;
    if (LifeFile::load(board_, loadedGeneration, path, errorMessage)) {
        generation_ = loadedGeneration;
        simulationAccumulator_ = 0.0;
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

void LifeGameApplication::advanceGeneration() {
    board_.step();
    ++generation_;
}

void LifeGameApplication::changeSimulationSpeed(int direction) {
    if (direction > 0) {
        if (simulationSpeedIndex_ + 1 < AppConfig::SimulationSpeeds.size()) {
            ++simulationSpeedIndex_;
        }
    } else if (direction < 0 && simulationSpeedIndex_ > 0) {
        --simulationSpeedIndex_;
    }

    simulationAccumulator_ = 0.0;
}

int LifeGameApplication::currentSimulationSpeed() const noexcept {
    return AppConfig::SimulationSpeeds[simulationSpeedIndex_];
}
