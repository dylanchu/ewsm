#include "paths_system.hpp"
#include "ecs/context/paths/paths_ctx.hpp"
#include "wx/datetime.h"
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "wx/dir.h"
#include "app.hpp"


ewsm::PathsSystem::PathsSystem(entt::registry& registry) : SystemBase(registry)
{
    Paths::Init();
}

wxString ewsm::PathsSystem::GetTimestampedFilename(const wxString& suffix)
{
    const wxDateTime now = wxDateTime::Now();
    auto time_str = now.Format("%Y-%m-%d_%H-%M-%S"); // FIXME: -%02d microseconds
    return suffix.empty() ? time_str : time_str + "_" + suffix;
}

int ewsm::PathsSystem::RemoveFile(const wxString& fp)
{
    return wxRemoveFile(fp);
}

int ewsm::PathsSystem::MoveFileToTrashDir(const wxString& fp)
{
    const auto name = wxFileNameFromPath(fp);
    wxFileName dst = wxStandardPaths::Get().GetUserLocalDataDir();
    dst.AppendDir(wxGetApp().GetAppName());
    dst.AppendDir("trash");
    dst.SetName(name);
    if(!dst.DirExists()) {
        if(!dst.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
            wxMessageBox("Failed to create path: " + dst.GetFullPath(), "Error", wxOK | wxICON_ERROR);
            return 1;
        }
    }
    return wxRename(fp, dst.GetFullPath());
}

void ewsm::PathsSystem::OpenDirInExplorer(const wxString& dir_path)
{
    wxString path = dir_path;
    if(!path.EndsWith(wxFILE_SEP_PATH)) {
        path += wxFILE_SEP_PATH;
    }
    if(!wxLaunchDefaultApplication(path)) {
        wxLogError("Cannot Open Directory: %s", path);
    }
}
