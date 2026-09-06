#pragma once

enum class BoundaryMode {
    Dead,
    Toroidal,
};

inline const char* boundaryModeName(BoundaryMode mode) noexcept {
    return mode == BoundaryMode::Dead ? "Dead" : "Toroidal";
}
