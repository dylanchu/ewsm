#include "service_install_wizard.hpp"
ServiceInstallWizard::ServiceInstallWizard(wxWindow* parent) :
    wxWizard(parent, wxID_ANY, "Install New Service")
{
    // 创建向导页面
    m_page1 = new wxWizardPageSimple(this);
    m_page2 = new wxWizardPageSimple(this);
    m_page3 = new wxWizardPageSimple(this);

    // 设置页面顺序
    wxWizardPageSimple::Chain(m_page1, m_page2);
    wxWizardPageSimple::Chain(m_page2, m_page3);

    // 设置页面内容
    SetupPage1();
    SetupPage2();
    SetupPage3();
}
bool ServiceInstallWizard::RunWizard()
{
    return wxWizard::RunWizard(m_page1);
}
void ServiceInstallWizard::SetupPage1()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(m_page1, wxID_ANY, "Service Name:"), 0, wxALL, 5);
    m_nameCtrl = new wxTextCtrl(m_page1, wxID_ANY);
    sizer->Add(m_nameCtrl, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page1, wxID_ANY, "Display Name:"), 0, wxALL, 5);
    m_displayNameCtrl = new wxTextCtrl(m_page1, wxID_ANY);
    sizer->Add(m_displayNameCtrl, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page1, wxID_ANY, "Description:"), 0, wxALL, 5);
    m_descCtrl = new wxTextCtrl(m_page1, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    sizer->Add(m_descCtrl, 1, wxEXPAND | wxALL, 5);

    m_page1->SetSizer(sizer);
}
void ServiceInstallWizard::SetupPage2()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(m_page2, wxID_ANY, "Executable Path:"), 0, wxALL, 5);
    wxBoxSizer* pathSizer = new wxBoxSizer(wxHORIZONTAL);
    m_pathCtrl = new wxTextCtrl(m_page2, wxID_ANY);
    pathSizer->Add(m_pathCtrl, 1, wxEXPAND);
    wxButton* browseBtn = new wxButton(m_page2, wxID_ANY, "Browse...");
    browseBtn->Bind(wxEVT_BUTTON, &ServiceInstallWizard::OnBrowse, this);
    pathSizer->Add(browseBtn, 0, wxLEFT, 5);
    sizer->Add(pathSizer, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page2, wxID_ANY, "Arguments:"), 0, wxALL, 5);
    m_argsCtrl = new wxTextCtrl(m_page2, wxID_ANY);
    sizer->Add(m_argsCtrl, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page2, wxID_ANY, "Working Directory:"), 0, wxALL, 5);
    m_workDirCtrl = new wxTextCtrl(m_page2, wxID_ANY);
    sizer->Add(m_workDirCtrl, 0, wxEXPAND | wxALL, 5);

    m_page2->SetSizer(sizer);
}
void ServiceInstallWizard::SetupPage3()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(m_page3, wxID_ANY, "Startup Type:"), 0, wxALL, 5);
    wxChoice* startTypeChoice = new wxChoice(m_page3, wxID_ANY);
    startTypeChoice->Append("Automatic");
    startTypeChoice->Append("Manual");
    startTypeChoice->Append("Disabled");
    startTypeChoice->SetSelection(0);
    sizer->Add(startTypeChoice, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page3, wxID_ANY, "Account:"), 0, wxALL, 5);
    wxTextCtrl* accountCtrl = new wxTextCtrl(m_page3, wxID_ANY, "LocalSystem");
    sizer->Add(accountCtrl, 0, wxEXPAND | wxALL, 5);

    sizer->Add(new wxStaticText(m_page3, wxID_ANY, "Recovery Options:"), 0, wxALL, 5);
    wxCheckListBox* recoveryOptions = new wxCheckListBox(m_page3, wxID_ANY);
    recoveryOptions->Append("Restart the service");
    recoveryOptions->Append("Run a program");
    recoveryOptions->Append("Restart the computer");
    sizer->Add(recoveryOptions, 1, wxEXPAND | wxALL, 5);

    m_page3->SetSizer(sizer);
}
void ServiceInstallWizard::OnBrowse(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Select Executable", "", "",
                     "Executable files (*.exe)|*.exe", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        m_pathCtrl->SetValue(dlg.GetPath());
    }
}
