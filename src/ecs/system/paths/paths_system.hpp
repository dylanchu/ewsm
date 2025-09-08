#pragma once
#include "ecs/system/system_base.hpp"
#include "wx/string.h"
#include "wx/filename.h"


namespace ewsm
{
    class PathsSystem : public SystemBase<PathsSystem>
    {
    public:
        explicit PathsSystem(entt::registry& registry);
        ~PathsSystem() override = default;

        static wxString GetTimestampedFilename(const wxString &suffix);
        static int RemoveFile(const wxString &fp);
        static int MoveFileToTrashDir(const wxString &fp);
        static void OpenDirInExplorer(const wxString &dir_path);
    };
}
