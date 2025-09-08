#pragma once
#include "wx/string.h"
#include "wx/filename.h"

namespace ewsm {

    struct Paths
    {
        inline static wxString SettingsFile{};
        inline static wxString TrashDir{};

        static void Init();
    };
} // namespace utils

