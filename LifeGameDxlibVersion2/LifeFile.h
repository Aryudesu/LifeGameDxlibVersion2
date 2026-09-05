#pragma once

#include <string>

class LifeBoard;

namespace LifeFile {
bool save(const LifeBoard& board, const std::string& path, std::string& errorMessage);
bool load(LifeBoard& board, const std::string& path, std::string& errorMessage);
}
