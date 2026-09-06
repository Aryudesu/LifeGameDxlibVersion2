#include "LifeGameApplication.h"

#include "AppConfig.h"
#include "DxLib.h"
#include "FileDialog.h"
#include "LifeFile.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <thread>

namespace {
PatternCell rotateCellClockwise(PatternCell cell, int quarterTurns) {
    int x = cell.x;
    int y = cell.y;
    for (int i = 0; i < quarterTurns; ++i) {
        const int nextX = -y;
        const int nextY = x;
        x = nextX;
        y = nextY;
    }
    return {x, y};
}

bool inRect(int x, int y, int left, int top, int right, int bottom) {
    return x >= left && x < right && y >= top && y < bottom;
}

constexpr int PanelX = AppConfig::BoardViewWidth;
constexpr int PanelPadding = 16;
constexpr int PanelContentX = PanelX + PanelPadding;
constexpr int PatternListY = 198;
constexpr int PatternRowHeight = 28;
constexpr std::array ToolCategories = {
    PatternCategory::StillLife,
    PatternCategory::Oscillator,
    PatternCategory::Methuselah,
    PatternCategory::Spaceship,
    PatternCategory::Gun,
};
}

LifeGameApplication::LifeGameApplication()
    : board_(AppConfig::BoardWidth, AppConfig::BoardHeight),
      renderer_(AppConfig::BoardViewWidth, AppConfig::ScreenHeight),
      camera_(AppConfig::BoardWidth, AppConfig::BoardHeight, AppConfig::BoardViewWidth,
          AppConfig::ScreenHeight, AppConfig::MinCellSize),
      simulationSpeedIndex_(AppConfig::DefaultSimulationSpeedIndex),
      randomAliveProbabilityIndex_(AppConfig::DefaultRandomAliveProbabilityIndex) {
    board_.randomize(currentRandomAliveProbability());
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
        const double elapsedSeconds = std::chrono::duration<double>(frameStart - previousFrameTime).count();
        previousFrameTime = frameStart;
        const InputFrame input = inputController_.update();
        update(input, elapsedSeconds);
        draw();
        ++fpsFrameCount;
        const auto fpsSampleEnd = Clock::now();
        const double fpsSampleSeconds = std::chrono::duration<double>(fpsSampleEnd - fpsSampleStart).count();
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
        if (nextFrameTime > afterFrame) std::this_thread::sleep_until(nextFrameTime);
        else nextFrameTime = afterFrame;
    }
    return 0;
}

void LifeGameApplication::update(const InputFrame& input, double elapsedSeconds) {
    handleToolPanel(input);
    if (input.togglePause) setPaused(!paused_);
    if (input.clearBoard) { board_.clear(); generation_ = 0; simulationAccumulator_ = 0.0; setPaused(true); }
    if (input.randomDensityIndex >= 0 && input.randomDensityIndex < static_cast<int>(AppConfig::RandomAliveProbabilities.size())) randomAliveProbabilityIndex_ = static_cast<std::size_t>(input.randomDensityIndex);
    if (input.randomizeBoard) { board_.randomize(currentRandomAliveProbability()); generation_ = 0; simulationAccumulator_ = 0.0; }
    if (input.save) saveBoard();
    if (input.load) loadBoard();
    if (input.toggleGrid) showGrid_ = !showGrid_;
    if (input.selectNextPattern) selectNextPattern();
    if (input.selectPreviousPattern) selectPreviousPattern();
    if (input.rotatePatternLeft) rotatePattern(-1);
    if (input.rotatePatternRight) rotatePattern(1);
    if (input.speedUp) changeSimulationSpeed(1);
    if (input.speedDown) changeSimulationSpeed(-1);
    if (input.zoomIn) camera_.zoomIn(AppConfig::MaxCellSize);
    if (input.zoomOut) camera_.zoomOut(AppConfig::MinCellSize);
    if (input.mouseX < AppConfig::BoardViewWidth) {
        if (input.mouseWheel > 0) for (int i = 0; i < input.mouseWheel; ++i) { if (!camera_.zoomInAt(input.mouseX, input.mouseY, AppConfig::MaxCellSize)) break; }
        else if (input.mouseWheel < 0) for (int i = 0; i < -input.mouseWheel; ++i) { if (!camera_.zoomOutAt(input.mouseX, input.mouseY, AppConfig::MinCellSize)) break; }
    }
    camera_.move(input.cameraDeltaX * AppConfig::CameraMoveSpeed, input.cameraDeltaY * AppConfig::CameraMoveSpeed);
    if (input.mouseMiddleDown && input.mouseX < AppConfig::BoardViewWidth) camera_.panByPixels(input.mouseDeltaX, input.mouseDeltaY);
    else camera_.endPan();
    if (paused_) {
        simulationAccumulator_ = 0.0;
        editBoardWithMouse(input);
        if (input.singleStep) advanceGeneration();
        return;
    }
    const double simulationSeconds = std::min(elapsedSeconds, AppConfig::MaxSimulationDeltaSeconds);
    simulationAccumulator_ += simulationSeconds * currentSimulationSpeed();
    const int generationsToAdvance = static_cast<int>(simulationAccumulator_);
    simulationAccumulator_ -= generationsToAdvance;
    for (int i = 0; i < generationsToAdvance; ++i) advanceGeneration();
}

void LifeGameApplication::draw() const { ClearDrawScreen(); renderer_.draw(board_, camera_, showGrid_); drawToolPanel(); ScreenFlip(); }

void LifeGameApplication::drawToolPanel() const {
    const unsigned int background = GetColor(28, 30, 34), section = GetColor(45, 48, 54), selected = GetColor(70, 105, 75), text = GetColor(235, 235, 235), muted = GetColor(170, 175, 180);
    DrawBox(PanelX, 0, AppConfig::WindowWidth, AppConfig::ScreenHeight, background, TRUE);
    DrawString(PanelContentX, 14, "LIFE GAME TOOLS", text);
    DrawBox(PanelContentX, 42, AppConfig::WindowWidth - PanelPadding, 72, selectedPatternIndex_ == 0 ? selected : section, TRUE);
    DrawString(PanelContentX + 10, 49, "Cell", text);
    for (std::size_t i = 0; i < ToolCategories.size(); ++i) {
        const int column = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        const int left = PanelContentX + column * 144;
        const int top = 86 + row * 36;
        DrawBox(left, top, left + 136, top + 28, toolCategory_ == ToolCategories[i] ? selected : section, TRUE);
        DrawString(left + 8, top + 6, PatternLibrary::categoryName(ToolCategories[i]), text);
    }
    int row = 0;
    for (std::size_t i = 1; i < PatternLibrary::size(); ++i) {
        const LifePattern& pattern = PatternLibrary::at(i);
        if (pattern.category != toolCategory_) continue;
        const int top = PatternListY + row * PatternRowHeight;
        DrawBox(PanelContentX, top, AppConfig::WindowWidth - PanelPadding, top + 24, selectedPatternIndex_ == i ? selected : section, TRUE);
        DrawString(PanelContentX + 8, top + 5, pattern.name, text);
        ++row;
    }
    const std::uint64_t aliveCells = board_.aliveCellCount();
    const std::uint64_t totalCells = static_cast<std::uint64_t>(board_.width()) * static_cast<std::uint64_t>(board_.height());
    const double alivePercentage = totalCells == 0 ? 0.0 : static_cast<double>(aliveCells) * 100.0 / static_cast<double>(totalCells);
    const int infoY = 700;
    DrawString(PanelContentX, infoY, "STATUS", muted);
    DrawFormatString(PanelContentX, infoY + 26, text, "Generation  %llu", static_cast<unsigned long long>(generation_));
    DrawFormatString(PanelContentX, infoY + 50, text, "FPS         %.1f", fps_);
    DrawFormatString(PanelContentX, infoY + 74, text, "Speed       %d gen/s", currentSimulationSpeed());
    DrawFormatString(PanelContentX, infoY + 98, text, "Random      %.0f%%", currentRandomAliveProbability() * 100.0);
    DrawFormatString(PanelContentX, infoY + 122, text, "Alive       %llu (%.2f%%)", static_cast<unsigned long long>(aliveCells), alivePercentage);
    DrawFormatString(PanelContentX, infoY + 146, text, "Pattern     %s", currentPattern().name);
    DrawFormatString(PanelContentX, infoY + 170, text, "Rotation    R%d", currentPatternRotationDegrees());
    DrawString(PanelContentX, infoY + 202, paused_ ? "PAUSED" : "RUNNING", paused_ ? GetColor(255, 210, 90) : GetColor(120, 230, 140));
    DrawString(PanelContentX, 954, "P/Shift+P: select   Q/E: rotate", muted);
    DrawString(PanelContentX, 978, "Click a pattern to select it", muted);
}

void LifeGameApplication::setPaused(bool paused) { paused_ = paused; simulationAccumulator_ = 0.0; SetMainWindowText(paused_ ? AppConfig::PausedWindowTitle : AppConfig::WindowTitle); }
void LifeGameApplication::saveBoard() { std::string path; if (!FileDialog::chooseSavePath(path)) return; std::string errorMessage; if (!LifeFile::save(board_, generation_, path, errorMessage)) FileDialog::showError(errorMessage); }
void LifeGameApplication::loadBoard() { std::string path; if (!FileDialog::chooseLoadPath(path)) return; std::string errorMessage; std::uint64_t loadedGeneration = 0; if (LifeFile::load(board_, loadedGeneration, path, errorMessage)) { generation_ = loadedGeneration; simulationAccumulator_ = 0.0; setPaused(true); return; } FileDialog::showError(errorMessage); }

void LifeGameApplication::editBoardWithMouse(const InputFrame& input) {
    if (input.mouseX < 0 || input.mouseY < 0 || input.mouseX >= AppConfig::BoardViewWidth || input.mouseY >= AppConfig::ScreenHeight) return;
    const auto [boardX, boardY] = camera_.screenToBoard(input.mouseX, input.mouseY);
    if (selectedPatternIndex_ == 0) { if (input.mouseLeftDown) board_.setAlive(boardX, boardY, true); }
    else if (input.mouseLeftPressed) placeSelectedPattern(boardX, boardY);
    if (input.mouseRightDown) board_.setAlive(boardX, boardY, false);
}

void LifeGameApplication::handleToolPanel(const InputFrame& input) {
    if (!input.mouseLeftPressed || input.mouseX < PanelX) return;
    if (inRect(input.mouseX, input.mouseY, PanelContentX, 42, AppConfig::WindowWidth - PanelPadding, 72)) { selectPattern(0); return; }
    for (std::size_t i = 0; i < ToolCategories.size(); ++i) {
        const int column = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        const int left = PanelContentX + column * 144;
        const int top = 86 + row * 36;
        if (inRect(input.mouseX, input.mouseY, left, top, left + 136, top + 28)) {
            for (std::size_t patternIndex = 1; patternIndex < PatternLibrary::size(); ++patternIndex) {
                if (PatternLibrary::at(patternIndex).category == ToolCategories[i]) {
                    selectPattern(patternIndex);
                    return;
                }
            }
            return;
        }
    }
    int row = 0;
    for (std::size_t i = 1; i < PatternLibrary::size(); ++i) {
        const LifePattern& pattern = PatternLibrary::at(i);
        if (pattern.category != toolCategory_) continue;
        const int top = PatternListY + row * PatternRowHeight;
        if (inRect(input.mouseX, input.mouseY, PanelContentX, top, AppConfig::WindowWidth - PanelPadding, top + 24)) { selectPattern(i); return; }
        ++row;
    }
}

void LifeGameApplication::placeSelectedPattern(int boardX, int boardY) { const LifePattern& pattern = currentPattern(); for (const PatternCell cell : pattern.cells) { const PatternCell rotated = rotateCellClockwise(cell, patternRotationQuarterTurns_); board_.setAlive(boardX + rotated.x, boardY + rotated.y, true); } }
void LifeGameApplication::selectNextPattern() noexcept { selectPattern((selectedPatternIndex_ + 1) % PatternLibrary::size()); }
void LifeGameApplication::selectPreviousPattern() noexcept { selectPattern((selectedPatternIndex_ + PatternLibrary::size() - 1) % PatternLibrary::size()); }
void LifeGameApplication::selectPattern(std::size_t index) noexcept { selectedPatternIndex_ = index % PatternLibrary::size(); patternRotationQuarterTurns_ = 0; const PatternCategory category = currentPattern().category; if (category != PatternCategory::Cell) toolCategory_ = category; }
void LifeGameApplication::rotatePattern(int direction) noexcept { if (selectedPatternIndex_ == 0 || direction == 0) return; patternRotationQuarterTurns_ = (patternRotationQuarterTurns_ + (direction > 0 ? 1 : 3)) % 4; }
const LifePattern& LifeGameApplication::currentPattern() const noexcept { return PatternLibrary::at(selectedPatternIndex_); }
int LifeGameApplication::currentPatternRotationDegrees() const noexcept { return patternRotationQuarterTurns_ * 90; }
void LifeGameApplication::advanceGeneration() { board_.step(); ++generation_; }
void LifeGameApplication::changeSimulationSpeed(int direction) { if (direction > 0) { if (simulationSpeedIndex_ + 1 < AppConfig::SimulationSpeeds.size()) ++simulationSpeedIndex_; } else if (direction < 0 && simulationSpeedIndex_ > 0) --simulationSpeedIndex_; simulationAccumulator_ = 0.0; }
int LifeGameApplication::currentSimulationSpeed() const noexcept { return AppConfig::SimulationSpeeds[simulationSpeedIndex_]; }
double LifeGameApplication::currentRandomAliveProbability() const noexcept { return AppConfig::RandomAliveProbabilities[randomAliveProbabilityIndex_]; }
