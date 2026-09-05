#include "FileDialog.h"

#include "AppConfig.h"
#include "DxLib.h"

#include <commdlg.h>
#include <cstring>
#include <windows.h>

#pragma comment(lib, "Comdlg32.lib")

namespace FileDialog {
bool chooseSavePath(std::string& path) {
    char fileName[MAX_PATH] = {};
    strcpy_s(fileName, AppConfig::SaveFileDefaultName);

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetMainWindowHandle();
    dialog.lpstrFilter = AppConfig::SaveFileFilter;
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = AppConfig::SaveFileExtension;
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&dialog) == FALSE) {
        return false;
    }

    path = fileName;
    return true;
}

bool chooseLoadPath(std::string& path) {
    char fileName[MAX_PATH] = {};

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetMainWindowHandle();
    dialog.lpstrFilter = AppConfig::SaveFileFilter;
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = AppConfig::SaveFileExtension;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&dialog) == FALSE) {
        return false;
    }

    path = fileName;
    return true;
}

void showError(const std::string& message) {
    MessageBoxA(
        GetMainWindowHandle(),
        message.c_str(),
        AppConfig::WindowTitle,
        MB_OK | MB_ICONERROR);
}
} // namespace FileDialog
