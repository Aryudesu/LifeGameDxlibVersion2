#pragma once

namespace AppConfig {
inline constexpr char WindowTitle[] = "LifeGameDxlibVersion2";
inline constexpr char PausedWindowTitle[] = "LifeGameDxlibVersion2 - Paused";

inline constexpr int ScreenWidth = 1024;
inline constexpr int ScreenHeight = 1024;
inline constexpr int ScreenColorDepth = 32;

inline constexpr int BoardWidth = 1536;
inline constexpr int BoardHeight = 1536;

inline constexpr int MinCellSize = 1;
inline constexpr int MaxCellSize = 32;
inline constexpr int CameraMoveSpeed = 8;

inline constexpr int AliveColorR = 0;
inline constexpr int AliveColorG = 255;
inline constexpr int AliveColorB = 0;
inline constexpr int GridColorR = 64;
inline constexpr int GridColorG = 64;
inline constexpr int GridColorB = 64;

inline constexpr char SaveFileDefaultName[] = "lifegame.ary2";
inline constexpr char SaveFileExtension[] = "ary2";
inline constexpr char SaveFileFilter[] =
    "LifeGame v2 (*.ary2)\0*.ary2\0"
    "All Files (*.*)\0*.*\0";
} // namespace AppConfig
