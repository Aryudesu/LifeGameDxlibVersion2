#pragma once

#include "Camera.h"
#include "InputController.h"
#include "LifeBoard.h"
#include "LifeRenderer.h"
#include "PatternLibrary.h"
#include "PatternListScroll.h"

#include <cstddef>
#include <cstdint>

class LifeGameApplication {
public:
    LifeGameApplication();

    int run();

private:
    void update(const InputFrame& input, double elapsedSeconds);
    void draw() const;
    void drawToolPanel() const;
    void setPaused(bool paused);
    void saveBoard();
    void loadBoard();
    void editBoardWithMouse(const InputFrame& input);
    void handleToolPanel(const InputFrame& input);
    void setBoundaryMode(BoundaryMode mode) noexcept;
    void placeSelectedPattern(int boardX, int boardY);
    void selectNextPattern() noexcept;
    void selectPreviousPattern() noexcept;
    void selectPattern(std::size_t index) noexcept;
    void rotatePattern(int direction) noexcept;
    const LifePattern& currentPattern() const noexcept;
    int currentPatternRotationDegrees() const noexcept;
    void advanceGeneration();
    void changeSimulationSpeed(int direction);
    int currentSimulationSpeed() const noexcept;
    double currentRandomAliveProbability() const noexcept;

    LifeBoard board_;
    LifeRenderer renderer_;
    Camera camera_;
    InputController inputController_;
    PatternListScroll patternListScroll_;
    bool paused_ = false;
    bool showGrid_ = false;
    std::uint64_t generation_ = 0;
    std::size_t simulationSpeedIndex_ = 0;
    std::size_t randomAliveProbabilityIndex_ = 0;
    std::size_t selectedPatternIndex_ = 0;
    PatternCategory toolCategory_ = PatternCategory::StillLife;
    int patternRotationQuarterTurns_ = 0;
    double simulationAccumulator_ = 0.0;
    double fps_ = 0.0;
};
