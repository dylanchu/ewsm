#pragma once
#include <wx/notebook.h>
#include <wx/wizard.h>
#include <wx/wx.h>

class ServiceInstallWizard : public wxWizard {
public:
    ServiceInstallWizard(wxWindow* parent);

    bool RunWizard();

private:
    void SetupPage1();
    void SetupPage2();
    void SetupPage3();

    void OnBrowse(wxCommandEvent& event);

    wxWizardPageSimple* m_page1;
    wxWizardPageSimple* m_page2;
    wxWizardPageSimple* m_page3;
    
    wxTextCtrl* m_nameCtrl;
    wxTextCtrl* m_displayNameCtrl;
    wxTextCtrl* m_descCtrl;
    wxTextCtrl* m_pathCtrl;
    wxTextCtrl* m_argsCtrl;
    wxTextCtrl* m_workDirCtrl;
};