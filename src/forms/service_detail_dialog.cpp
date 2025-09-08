#include "service_detail_dialog.hpp"
#include <wx/notebook.h>

#include "app.hpp"

using namespace ewsm;


ServiceDetailDialog::ServiceDetailDialog(wxWindow* parent, const wxString& serviceName) :
    wxDialog(parent, wxID_ANY, "Service Details: " + serviceName)
{

    // 创建选项卡
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    // 基本信息面板
    wxPanel* basicPanel = new wxPanel(notebook);
    CreateBasicPanel(basicPanel, serviceName);
    notebook->AddPage(basicPanel, "Basic");

    // 配置面板
    wxPanel* configPanel = new wxPanel(notebook);
    CreateConfigPanel(configPanel, serviceName);
    notebook->AddPage(configPanel, "Configuration");

    // 日志面板
    wxPanel* logPanel = new wxPanel(notebook);
    CreateLogPanel(logPanel, serviceName);
    notebook->AddPage(logPanel, "Logs");

    // 性能面板
    wxPanel* perfPanel = new wxPanel(notebook);
    CreatePerfPanel(perfPanel, serviceName);
    notebook->AddPage(perfPanel, "Performance");

    // 按钮
    wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);

    // 主布局
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizerAndFit(mainSizer);
}
void ServiceDetailDialog::CreateBasicPanel(wxWindow* parent, const wxString& serviceName)
{
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 5, 5);
    grid->AddGrowableCol(1);

    // 获取服务详情
    // ServiceComp info = ewsm::ServiceSystem::GetServiceInfo(serviceName);
    const auto* comp = wxGetApp().reg.try_get<ServiceComp>(entt::null); // todo: fixme
    if (!comp)
        return;

    const auto& info = *comp;

    // 添加字段
    grid->Add(new wxStaticText(parent, wxID_ANY, "Name:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, info.name));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Display Name:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, info.display_name));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Status:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, ServiceSystem::ServiceStatusToString(info.status)));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Start Type:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, ServiceSystem::StartTypeToString(info.start_type)));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Binary Path:"));
    grid->Add(new wxTextCtrl(parent, wxID_ANY, info.binary_path, wxDefaultPosition, wxDefaultSize, wxTE_READONLY));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Description:"));
    grid->Add(new wxTextCtrl(parent, wxID_ANY, info.description, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY));

    parent->SetSizer(grid);
}
void ServiceDetailDialog::CreateConfigPanel(wxWindow* parent, const wxString& serviceName)
{
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, 5, 5);
    grid->AddGrowableCol(1);

    // // 获取服务配置
    // ServiceConfig config = ServiceSystem::GetServiceConfig(serviceName);
    //
    // // 添加可编辑字段
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Startup Type:"));
    // wxChoice* startTypeChoice = new wxChoice(parent, wxID_ANY);
    // startTypeChoice->Append("Automatic");
    // startTypeChoice->Append("Manual");
    // startTypeChoice->Append("Disabled");
    // startTypeChoice->SetSelection(config.start_type == SERVICE_AUTO_START ? 0 : config.start_type == SERVICE_DEMAND_START ? 1
    //                                                                                                                     : 2);
    // grid->Add(startTypeChoice);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Account:"));
    // wxTextCtrl* accountCtrl = new wxTextCtrl(parent, wxID_ANY, config.account);
    // grid->Add(accountCtrl);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Arguments:"));
    // wxTextCtrl* argsCtrl = new wxTextCtrl(parent, wxID_ANY, config.arguments);
    // grid->Add(argsCtrl);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Working Directory:"));
    // wxTextCtrl* workDirCtrl = new wxTextCtrl(parent, wxID_ANY, config.working_dir);
    // grid->Add(workDirCtrl);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Environment:"));
    // wxTextCtrl* envCtrl = new wxTextCtrl(parent, wxID_ANY, config.environment,
    //                                      wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    // grid->Add(envCtrl, 1, wxEXPAND);

    parent->SetSizer(grid);
}
void ServiceDetailDialog::CreateLogPanel(wxWindow* parent, const wxString& serviceName)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // // 日志查看器
    // wxTextCtrl* logViewer = new wxTextCtrl(parent, wxID_ANY, "",
    //                                        wxDefaultPosition, wxDefaultSize,
    //                                        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    // // 加载日志
    // wxString logContent = ServiceSystem::GetServiceLog(serviceName);
    // logViewer->SetValue(logContent);
    // // 自动滚动到底部
    // logViewer->ShowPosition(logViewer->GetLastPosition());
    // sizer->Add(logViewer, 1, wxEXPAND);

    parent->SetSizer(sizer);
}
void ServiceDetailDialog::CreatePerfPanel(wxWindow* parent, const wxString& serviceName)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // CPU 使用率图表
    wxStaticText* cpuLabel = new wxStaticText(parent, wxID_ANY, "CPU Usage:");
    sizer->Add(cpuLabel, 0, wxEXPAND | wxALL, 5);

    wxGauge* cpuGauge = new wxGauge(parent, wxID_ANY, 100);
    sizer->Add(cpuGauge, 0, wxEXPAND | wxALL, 5);

    // 内存使用图表
    wxStaticText* memLabel = new wxStaticText(parent, wxID_ANY, "Memory Usage:");
    sizer->Add(memLabel, 0, wxEXPAND | wxALL, 5);

    wxGauge* memGauge = new wxGauge(parent, wxID_ANY, 100);
    sizer->Add(memGauge, 0, wxEXPAND | wxALL, 5);

    // 实时更新
    m_timer = new wxTimer(this);
    Bind(wxEVT_TIMER, [=](wxTimerEvent&) {
        // ServicePerf perf = ServiceSystem::GetServicePerf(serviceName);
        // cpuGauge->SetValue(perf.cpuUsage);
        // memGauge->SetValue(perf.memoryUsage);
    });
    m_timer->Start(1000); // 每秒更新一次

    parent->SetSizer(sizer);
}
