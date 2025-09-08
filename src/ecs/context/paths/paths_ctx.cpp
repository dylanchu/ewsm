#include "paths_ctx.hpp"
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "app.hpp"


void ewsm::Paths::Init()
{
    const wxString sep = wxFileName::GetPathSeparator();
    const auto base_dir = wxStandardPaths::Get().GetUserLocalDataDir(); // Appdata\Local\{AppName}

    const wxFileName settings_file(base_dir + sep + "settings.ini");
    SettingsFile = settings_file.GetFullPath();

    const wxFileName trash_a(base_dir + sep + "trash", "a");
    if(!trash_a.DirExists()) {
        if(!trash_a.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
            wxMessageBox("Failed to create path: " + trash_a.GetPath(), "Error", wxOK | wxICON_ERROR);
        }
    }
    TrashDir = trash_a.GetPath(); // GetPath: only directory path
}
