#include "service_detail_dialog.hpp"
#include <wx/notebook.h>
#include <wx/stc/stc.h>
#include "app.hpp"
#include "ecs/system/service/service_system.hpp"
#include "utils/timer_manager.hpp"

using namespace ewsm;


ServiceDetailDialog::ServiceDetailDialog(wxWindow* parent, const wxString& service_name) :
    wxDialog(parent, wxID_ANY, "Service Details: " + service_name)
{
    // 创建选项卡
    auto* notebook = new wxNotebook(this, wxID_ANY);

    // 基本信息面板
    auto* basic_panel = new wxPanel(notebook);
    CreateBasicPanel(basic_panel, service_name);
    notebook->AddPage(basic_panel, "Basic");

    // 配置面板
    auto* config_panel = new wxPanel(notebook);
    CreateConfigPanel(config_panel, service_name);
    notebook->AddPage(config_panel, "Configuration");

    // 日志面板
    auto* log_panel = new wxPanel(notebook);
    CreateLogPanel(log_panel, service_name);
    notebook->AddPage(log_panel, "Logs");

    // 性能面板
    auto* perf_panel = new wxPanel(notebook);
    CreatePerfPanel(perf_panel, service_name);
    notebook->AddPage(perf_panel, "Performance");

    // 按钮
    wxSizer* button_sizer = CreateButtonSizer(wxOK | wxCANCEL);

    // 主布局
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    main_sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 5);

    main_sizer->SetMinSize(600, 400);
    SetSizerAndFit(main_sizer);
}

ServiceDetailDialog::~ServiceDetailDialog()
{
    if (update_timer) {
        utils::TimerManager::Get().RemoveTimer(update_timer);
        update_timer = 0;
    }
}

void ServiceDetailDialog::CreateBasicPanel(wxWindow* parent, const wxString& service_name)
{
    auto* grid = new wxFlexGridSizer(2, 10, 10);
    grid->AddGrowableCol(1);

    // 获取服务详情
    const auto info = ewsm::ServiceSystem::GetServiceInfo(service_name);

    // 添加字段
    grid->Add(new wxStaticText(parent, wxID_ANY, "Name:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, info.name));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Display Name:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, info.display_name));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Status:"));
    grid->Add(new wxStaticText(parent, wxID_ANY, ServiceSystem::ServiceStatusToString(info.status)));

    grid->Add(new wxStaticText(parent, wxID_ANY, "Start Type:"));
    auto pr = ServiceSystem::StartTypeToString(info.start_type);
    grid->Add(new wxStaticText(parent, wxID_ANY, wxString::Format("%s - %s", pr.first, pr.second)));

    auto create_stc = [](wxWindow* p, const wxString& text) -> wxStyledTextCtrl* {
        auto* item = new wxStyledTextCtrl(p, wxID_ANY, wxDefaultPosition, wxDefaultSize);
        item->SetText(text);
        item->SetWrapMode(wxSTC_WRAP_WORD);
        item->SetMinClientSize(wxSize(0,0)); // set a small minimum size
        item->SetReadOnly(true);
        item->SetCaretStyle(wxSTC_CARETSTYLE_INVISIBLE); // set caret invisible
        item->SetUseHorizontalScrollBar(false);
        item->SetMarginWidth(1,0); // hide left margin
        item->SetTechnology(wxSTC_TECHNOLOGY_DIRECTWRITE); // use D2D drawing
        return item;
    };

    grid->Add(new wxStaticText(parent, wxID_ANY, "Binary Path:"));
    grid->Add(create_stc(parent, info.binary_path), 4, wxEXPAND)->SetInitSize(-1, -1);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Description:"));
    grid->Add(create_stc(parent, info.description), 0, wxEXPAND)->SetInitSize(-1, -1);

    parent->SetSizer(grid);
}

void ServiceDetailDialog::CreateConfigPanel(wxWindow* parent, const wxString& service_name)
{
    auto* grid = new wxFlexGridSizer(2, 5, 5);
    grid->AddGrowableCol(1);

    // 获取服务配置
    const auto config = ServiceSystem::GetServiceInfo(service_name);

    // 添加可编辑字段
    grid->Add(new wxStaticText(parent, wxID_ANY, "Startup Type:"));
    auto* start_type_choice = new wxChoice(parent, wxID_ANY);
    start_type_choice->Append("Automatic");
    start_type_choice->Append("Manual");
    start_type_choice->Append("Disabled");
    start_type_choice->SetSelection(config.start_type == SERVICE_AUTO_START ? 0 : config.start_type == SERVICE_DEMAND_START ? 1 : 2);
    grid->Add(start_type_choice);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Account:"));
    auto* account_ctrl = new wxTextCtrl(parent, wxID_ANY, config.account);
    grid->Add(account_ctrl);

    // grid->Add(new wxStaticText(parent, wxID_ANY, "Arguments:"));
    // auto* args_ctrl = new wxTextCtrl(parent, wxID_ANY, config.arguments);
    // grid->Add(args_ctrl);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Working Directory:"));
    // auto* work_dir_ctrl = new wxTextCtrl(parent, wxID_ANY, config.working_dir);
    // grid->Add(work_dir_ctrl);
    //
    // grid->Add(new wxStaticText(parent, wxID_ANY, "Environment:"));
    // auto* env_ctrl = new wxTextCtrl(parent, wxID_ANY, config.environment, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    // grid->Add(env_ctrl, 1, wxEXPAND);

    parent->SetSizer(grid);
}

void ServiceDetailDialog::CreateLogPanel(wxWindow* parent, const wxString& service_name)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // // 日志查看器
    // auto* log_viewer = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    // // 加载日志
    // wxString log_content = ServiceSystem::GetServiceLog(service_name);
    // log_viewer->SetValue(log_content);
    // // 自动滚动到底部
    // log_viewer->ShowPosition(log_viewer->GetLastPosition());
    // sizer->Add(log_viewer, 1, wxEXPAND);

    parent->SetSizer(sizer);
}

void ServiceDetailDialog::CreatePerfPanel(wxWindow* parent, const wxString& service_name)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // CPU 使用率图表
    auto* cpu_label = new wxStaticText(parent, wxID_ANY, "CPU Usage:");
    sizer->Add(cpu_label, 0, wxEXPAND | wxALL, 5);

    auto* cpu_gauge = new wxGauge(parent, wxID_ANY, 100);
    sizer->Add(cpu_gauge, 0, wxEXPAND | wxALL, 5);

    // 内存使用图表
    auto* mem_label = new wxStaticText(parent, wxID_ANY, "Memory Usage:");
    sizer->Add(mem_label, 0, wxEXPAND | wxALL, 5);

    auto* mem_gauge = new wxGauge(parent, wxID_ANY, 100);
    sizer->Add(mem_gauge, 0, wxEXPAND | wxALL, 5);

    // // 实时更新
    // update_timer = utils::TimerManager::Get().AddRepeatTimer(1000, [=, this]() {
    //     ServicePerf perf = ServiceSystem::GetServicePerf(service_name);
    //     cpu_gauge->SetValue(perf.cpuUsage);
    //     mem_gauge->SetValue(perf.memoryUsage);
    // });

    parent->SetSizer(sizer);
}
