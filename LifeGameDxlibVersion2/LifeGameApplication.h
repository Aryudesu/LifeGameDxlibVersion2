#pragma once

#include "Camera.h"
#include "InputController.h"
#include "LifeBoard.h"
#include "LifeRenderer.h"

#include <cstddef>
#include <cstdint>

class LifeGameApplication {
public:
    LifeGameApplication();

    int run();

private:
    enum class PlacementPattern : std::size_t {
        Cell,
        Glider,
        Blinker,
        LightweightSpaceship,
        Count
    };

    void update(const InputFrame& input, double elapsedSeconds);
    void draw() const;
    void setPaused(bool paused);
    void saveBoard();
    void loadBoard();
    void editBoardWithMouse(const InputFrame& input);
    void placeSelectedPattern(int boardX, int boardY);
    void selectNextPattern() noexcept;
    void selectPreviousPattern() noexcept;
    void rotatePattern(int direction) noexcept;
    const char* currentPatternName() const noexcept;
    int currentPatternRotationDegrees() const noexcept;
    void advanceGeneration();
    void changeSimulationSpeed(int direction);
    int currentSimulationSpeed() const noexcept;
    double currentRandomAliveProbability() const noexcept;

    LifeBoard board_;
    LifeRenderer renderer_;
    Camera camera_;
    InputController inputController_;
    bool paused_ = false;
    bool showGrid_ = false;
    std::uint64_t generation_ = 0;
    std::size_t simulationSpeedIndex_ = 0;
    std::size_t randomAliveProbabilityIndex_ = 0;
    PlacementPattern selectedPattern_ = PlacementPattern::Cell;
    int patternRotationQuarterTurns_ = 0;
    double simulationAccumulator_ = 0.0;
    double fps_ = 0.0;
};
