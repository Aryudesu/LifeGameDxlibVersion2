#pragma once

#include <cstdint>
#include <string>

class LifeBoard;

namespace LifeFile {
bool save(
    const LifeBoard& board,
    std::uint64_t generation,
    const std::string& path,
    std::string& errorMessage);
bool load(
    LifeBoard& board,
    std::uint64_t& generation,
    const std::string& path,
    std::string& errorMessage);
}
