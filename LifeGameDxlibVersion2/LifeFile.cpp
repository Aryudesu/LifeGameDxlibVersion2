#include "LifeFile.h"

#include "LifeBoard.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <vector>
#include <windows.h>

namespace {
constexpr std::array<char, 8> Magic = {'A', 'R', 'Y', 'L', 'I', 'F', 'E', '2'};
constexpr std::uint32_t FormatVersion = 2;
constexpr std::uint64_t MaxPayloadBytes = 64ULL * 1024ULL * 1024ULL;

void writeU32(std::ostream& output, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        output.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

void writeU64(std::ostream& output, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        output.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

bool readU32(std::istream& input, std::uint32_t& value) {
    value = 0;
    for (int i = 0; i < 4; ++i) {
        const int byte = input.get();
        if (byte == EOF) {
            return false;
        }
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte)) << (i * 8);
    }
    return true;
}

bool readU64(std::istream& input, std::uint64_t& value) {
    value = 0;
    for (int i = 0; i < 8; ++i) {
        const int byte = input.get();
        if (byte == EOF) {
            return false;
        }
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << (i * 8);
    }
    return true;
}

std::uint32_t checksum(const std::vector<std::uint8_t>& data) {
    std::uint32_t hash = 2166136261u;
    for (const std::uint8_t byte : data) {
        hash ^= byte;
        hash *= 16777619u;
    }
    return hash;
}

std::vector<std::uint8_t> packBoard(const LifeBoard& board) {
    const std::uint64_t cellCount =
        static_cast<std::uint64_t>(board.width()) * static_cast<std::uint64_t>(board.height());
    std::vector<std::uint8_t> payload(static_cast<std::size_t>((cellCount + 7) / 8), 0);

    std::uint64_t index = 0;
    for (int y = 0; y < board.height(); ++y) {
        for (int x = 0; x < board.width(); ++x, ++index) {
            if (board.isAlive(x, y)) {
                payload[static_cast<std::size_t>(index / 8)] |=
                    static_cast<std::uint8_t>(1u << (index % 8));
            }
        }
    }

    return payload;
}

bool replaceFile(const std::string& temporaryPath, const std::string& destinationPath) {
    return MoveFileExA(
        temporaryPath.c_str(),
        destinationPath.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}
}

namespace LifeFile {
bool save(
    const LifeBoard& board,
    std::uint64_t generation,
    const std::string& path,
    std::string& errorMessage) {
    errorMessage.clear();

    if (board.width() <= 0 || board.height() <= 0) {
        errorMessage = "Invalid board dimensions.";
        return false;
    }

    const auto payload = packBoard(board);
    if (payload.size() > MaxPayloadBytes) {
        errorMessage = "Board data is too large to save.";
        return false;
    }

    const std::string temporaryPath = path + ".tmp";
    std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        errorMessage = "Could not open the temporary save file.";
        return false;
    }

    output.write(Magic.data(), static_cast<std::streamsize>(Magic.size()));
    writeU32(output, FormatVersion);
    writeU32(output, static_cast<std::uint32_t>(board.width()));
    writeU32(output, static_cast<std::uint32_t>(board.height()));
    writeU64(output, generation);
    writeU64(output, static_cast<std::uint64_t>(payload.size()));
    writeU32(output, checksum(payload));
    output.write(
        reinterpret_cast<const char*>(payload.data()),
        static_cast<std::streamsize>(payload.size()));
    output.flush();

    if (!output) {
        output.close();
        DeleteFileA(temporaryPath.c_str());
        errorMessage = "Failed while writing the save file.";
        return false;
    }

    output.close();
    if (!replaceFile(temporaryPath, path)) {
        DeleteFileA(temporaryPath.c_str());
        errorMessage = "Could not replace the destination save file.";
        return false;
    }

    return true;
}

bool load(
    LifeBoard& board,
    std::uint64_t& generation,
    const std::string& path,
    std::string& errorMessage) {
    errorMessage.clear();

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        errorMessage = "Could not open the save file.";
        return false;
    }

    std::array<char, Magic.size()> magic{};
    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!input || magic != Magic) {
        errorMessage = "This is not a LifeGameDxlibVersion2 save file.";
        return false;
    }

    std::uint32_t version = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint64_t loadedGeneration = 0;
    std::uint64_t payloadSize = 0;
    std::uint32_t storedChecksum = 0;

    if (!readU32(input, version) ||
        !readU32(input, width) ||
        !readU32(input, height) ||
        !readU64(input, loadedGeneration) ||
        !readU64(input, payloadSize) ||
        !readU32(input, storedChecksum)) {
        errorMessage = "The save file header is truncated.";
        return false;
    }

    if (version != FormatVersion) {
        errorMessage = "Unsupported save file version.";
        return false;
    }

    if (width != static_cast<std::uint32_t>(board.width()) ||
        height != static_cast<std::uint32_t>(board.height())) {
        errorMessage = "The save file board size does not match this board.";
        return false;
    }

    const std::uint64_t cellCount =
        static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height);
    const std::uint64_t expectedPayloadSize = (cellCount + 7) / 8;
    if (payloadSize != expectedPayloadSize || payloadSize > MaxPayloadBytes) {
        errorMessage = "Invalid save data size.";
        return false;
    }

    if (payloadSize > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        errorMessage = "Save data is too large for this system.";
        return false;
    }

    std::vector<std::uint8_t> payload(static_cast<std::size_t>(payloadSize));
    input.read(
        reinterpret_cast<char*>(payload.data()),
        static_cast<std::streamsize>(payload.size()));
    if (!input) {
        errorMessage = "The save file data is truncated.";
        return false;
    }

    if (input.peek() != EOF) {
        errorMessage = "Unexpected extra data was found in the save file.";
        return false;
    }

    if (checksum(payload) != storedChecksum) {
        errorMessage = "The save file checksum does not match.";
        return false;
    }

    board.clear();
    std::uint64_t index = 0;
    for (int y = 0; y < board.height(); ++y) {
        for (int x = 0; x < board.width(); ++x, ++index) {
            const bool alive =
                (payload[static_cast<std::size_t>(index / 8)] &
                 static_cast<std::uint8_t>(1u << (index % 8))) != 0;
            if (alive) {
                board.setAlive(x, y, true);
            }
        }
    }

    generation = loadedGeneration;
    return true;
}
}
