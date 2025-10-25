// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
#include "service_manager_frame.hpp"
#include <windows.h>
#include <wx/artprov.h>
#include <wx/treectrl.h>
#include "ecs/context/service/service_operation_thread.hpp"
#include "ecs/system/service/service_system.hpp"
#include "service_detail_dialog.hpp"
#include "service_install_wizard.hpp"
#include "utils/debug_utils.hpp"
#include "utils/timer_manager.hpp"

using namespace ewsm;
inline auto ServiceStatusToString = ServiceSystem::ServiceStatusToString;
inline auto StartTypeToString = ServiceSystem::StartTypeToString;


ServiceListModel::ServiceListModel() = default;

void ServiceListModel::add_service(const ServiceComp& service)
{
    wxVector<wxVariant> data;
    data.push_back(wxVariant(service.name));
    data.push_back(wxVariant(ServiceStatusToString(service.status)));
    data.push_back(wxVariant("-"/*StartTypeToString(service.start_type).first*/));
    data.push_back(wxVariant(service.description));
    AppendItem(data);
}

unsigned int ServiceListModel::find_next_row_with_prefix(wxString prefix, const unsigned cur_idx, const bool ignore_case, const bool loop) const
{
    const auto cnt = GetItemCount();
    if (ignore_case) {
        prefix.LowerCase();
    }
    auto find_func = [this, &prefix, ignore_case](const unsigned begin, const unsigned end) -> int {
        wxString name;
        for (auto i = begin; i < end; ++i) {
            wxVariant x;
            GetValueByRow(x, i, 0);
            if (x.Convert(&name)) {
                if (ignore_case) {
                    name.LowerCase();
                }
                if (name.StartsWith(prefix)) {
                    return i;
                }
            }
        }
        return -1;
    };
    if (const auto ret = find_func(cur_idx + 1, cnt); ret >= 0) {
        return ret;
    }
    if (loop) {
        if (const auto ret = find_func(0, cur_idx); ret >= 0) {
            return ret;
        }
    }
    return cur_idx;
}

ServiceListViewCtrl::ServiceListViewCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxDataViewListCtrl(parent, id, pos, size, style)
{
    Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &ServiceListViewCtrl::on_service_selected, this);
}

void ServiceListViewCtrl::focus_next_item_by_prefix(const wxString& prefix)
{
    if (const auto* model = static_cast<ServiceListModel*>(GetStore())) {
        const auto next_row = model->find_next_row_with_prefix(prefix, GetSelectedRow(), true, true);
        SelectRow(next_row);
        EnsureVisible(GetItemByRow(next_row));
    }
}

void ServiceListViewCtrl::on_service_selected(wxDataViewEvent& event)
{
    if (const auto frame = static_cast<ServiceManagerFrame*>(GetParent()->GetParent())) {
        frame->ShowServiceDetails(get_selected_service_name());
    }
}

wxString ServiceListViewCtrl::get_selected_service_name() const
{
    if (const auto row = GetSelectedRow(); row != wxNOT_FOUND) {
        return GetTextValue(row, 0);
    }
    return wxEmptyString;
}

//======================================================================================================

ServiceManagerFrame::ServiceManagerFrame() :
    wxFrame(nullptr, wxID_ANY, "Service Manager")
{
    // 创建主面板
    auto* panel = new wxPanel(this);

    // 服务列表 wxDataViewCtrl
    service_list_model_ = new ServiceListModel;
    service_list_view_ = new ServiceListViewCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_SINGLE);
    service_list_view_->AssociateModel(service_list_model_);
    auto* col = service_list_view_->AppendTextColumn(wxT("Service"), wxDATAVIEW_CELL_INERT, 100);
    col->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    col = service_list_view_->AppendTextColumn(wxT("Status"), wxDATAVIEW_CELL_INERT, 100);
    col->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    col = service_list_view_->AppendTextColumn(wxT("StartType"), wxDATAVIEW_CELL_INERT, 100);
    col->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    col = service_list_view_->AppendTextColumn(wxT("Description"), wxDATAVIEW_CELL_INERT, 100);
    col->SetWidth(100);
    service_list_view_->Bind(wxEVT_CHAR, [this](auto& evt) {
        const wchar_t key = evt.GetKeyCode();
        // 只处理字母和数字
        if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
            service_list_view_->focus_next_item_by_prefix(key);
        }
        else {
            evt.Skip();
        }
    });

    // 创建操作按钮
    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(panel, ID_INSTALL, "Install"));
    buttonSizer->Add(new wxButton(panel, ID_UNINSTALL, "Uninstall"));
    buttonSizer->Add(new wxButton(panel, ID_START, "Start"));
    buttonSizer->Add(new wxButton(panel, ID_STOP, "Stop"));
    buttonSizer->Add(new wxButton(panel, ID_RESTART, "Restart"));
    buttonSizer->Add(new wxButton(panel, ID_REFRESH, "Refresh"));

    // 主布局
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(service_list_view_, 1, wxEXPAND | wxALL, 5);
    main_sizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    panel->SetSizer(main_sizer);

    // 绑定事件
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnInstall, this, ID_INSTALL);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnUninstall, this, ID_UNINSTALL);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnStart, this, ID_START);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnStop, this, ID_STOP);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnRestart, this, ID_RESTART);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnRefresh, this, ID_REFRESH);

    // 初始加载服务列表
    RefreshServiceList();

    // 在构造函数中添加状态栏
    Super::CreateStatusBar();
    Super::SetStatusText("Ready");

    // // 添加工具栏
    // wxToolBar* toolbar = Super::CreateToolBar();
    // toolbar->AddTool(ID_INSTALL, "Install", wxArtProvider::GetBitmap(wxART_PLUS));
    // toolbar->AddTool(ID_UNINSTALL, "Uninstall", wxArtProvider::GetBitmap(wxART_MINUS));
    // toolbar->AddSeparator();
    // toolbar->AddTool(ID_START, "Start", wxArtProvider::GetBitmap(wxART_GO_FORWARD));
    // toolbar->AddTool(ID_STOP, "Stop", wxArtProvider::GetBitmap(wxART_STOP));
    // toolbar->AddTool(ID_RESTART, "Restart", wxArtProvider::GetBitmap(wxART_REDO));
    // toolbar->AddSeparator();
    // toolbar->AddTool(ID_REFRESH, "Refresh", wxArtProvider::GetBitmap(wxART_REFRESH));
    // toolbar->Realize();

    // 添加搜索框
    search_ctrl_ = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    search_ctrl_->Bind(wxEVT_TEXT, &ServiceManagerFrame::OnSearch, this);

    // 在布局中添加搜索框
    main_sizer->Add(search_ctrl_, 0, wxEXPAND | wxALL, 5);

    //服务分组功能
    // 添加分组树控件
    auto* groupTree = new wxTreeCtrl(panel, wxID_ANY);
    groupTree->AddRoot("All Services");
    groupTree->AppendItem(groupTree->GetRootItem(), "System Services");
    groupTree->AppendItem(groupTree->GetRootItem(), "Application Services");
    main_sizer->Add(groupTree, 0, wxEXPAND | wxALL, 5);
    // 绑定选择事件
    // groupTree->Bind(wxEVT_TREE_SEL_CHANGED, &ServiceManagerFrame::OnGroupSelected, this);

    SetSize(last_size_);
    Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        ServiceManagerFrame::last_size_ = event.GetSize();
        OnSize(event);
    });
    Bind(wxEVT_CLOSE_WINDOW, [this](auto&) {
        ServiceManagerFrame::instance = nullptr;
        this->Destroy();
    });

}

bool ServiceManagerFrame::IsShowing() noexcept
{
    return instance != nullptr && instance->IsShown();
}

// 在操作中更新状态
void ServiceManagerFrame::OnRefresh(wxCommandEvent& event) {
    RefreshServiceList();
    set_status_text("Service list refreshed");
}

// 搜索处理函数
void ServiceManagerFrame::OnSearch(wxCommandEvent& event) {
    wxString searchText = search_ctrl_->GetValue().Lower();
    // 过滤服务列表...
}

void ServiceManagerFrame::RefreshServiceList()
{
    // 获取所有服务
    const auto services = ServiceSystem::GetAllServices();

    service_list_model_->DeleteAllItems();
    for (const auto& service : services) {
        DEBUG_MSG("Service: {}", service.name.ToStdString(), service.binary_path.ToStdString());
        wxString nssm(L"nssm");
        if (service.binary_path.Contains(nssm)) {
            service_list_model_->add_service(service);
        }
    }

    // for (auto i = 0; i < service_list_->GetColumnCount() - 1; ++i) {
    //     service_list_->SetColumnWidth(i, wxLIST_AUTOSIZE);
    // }
    // service_list_->SetColumnWidth(service_list_->GetColumnCount() - 1, wxEXPAND);

}

void ServiceManagerFrame::ShowServiceDetails(const wxString& service_name)
{
    ServiceDetailDialog dlg(this, service_name);
    dlg.ShowModal();
    dlg.Destroy();
}

void ServiceManagerFrame::OnInstall(wxCommandEvent& event) {
    ServiceInstallWizard wizard(this);
    if (wizard.RunWizard()) {
        // 获取向导中的数据并安装服务
        RefreshServiceList();
    }
}

void ServiceManagerFrame::OnUninstall(wxCommandEvent& event) {
    const auto service_name = service_list_view_->get_selected_service_name();

    if (service_name.empty()) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to uninstall service \"%s\"?", service_name),
        "Confirm Uninstall",
        wxYES_NO | wxICON_QUESTION);
    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    if (ServiceSystem::Uninstall(service_name)) {
        RefreshServiceList();
    }
}

void ServiceManagerFrame::OnStart(wxCommandEvent& event) {
    const auto service_name = service_list_view_->get_selected_service_name();

    if (service_name.empty()) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    ServiceSystem::Start(service_name);
    RefreshServiceList();
}

// 初始化图像列表
void ServiceManagerFrame::InitializeImageList() {
    // auto* imageList = new wxImageList(16, 16);
    // // 添加状态图标
    // imageList->Add(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_OTHER, wxSize(16, 16))); // 运行中
    // imageList->Add(wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_OTHER, wxSize(16, 16))); // 已停止
    // imageList->Add(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_OTHER, wxSize(16, 16))); // 启动中
    // imageList->Add(wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_OTHER, wxSize(16, 16))); // 停止中
    // imageList->Add(wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_OTHER, wxSize(16, 16))); // 已暂停
    // imageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16))); // 已禁用
    // imageList->Add(wxArtProvider::GetBitmap(wxART_QUESTION, wxART_OTHER, wxSize(16, 16))); // 未知
    // service_listv_->AssignImageList(imageList, wxIMAGE_LIST_SMALL);
}

void ServiceManagerFrame::OnStop(wxCommandEvent& event) {
    const auto service_name = service_list_view_->get_selected_service_name();
    if (service_name.empty()) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // 检查服务是否正在运行
    if (!ServiceSystem::IsRunning(service_name)) {
        wxMessageBox("Service is not running", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to stop service \"%s\"?", service_name),
        "Confirm Stop",
        wxYES_NO | wxICON_QUESTION);

    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    // 显示等待提示
    set_status_text(wxString::Format("Stopping service: %s...", service_name));
    // service_list_->SetItemTextColour(selected, wxColour(255, 165, 0)); // 橙色表示操作中

    // 禁用按钮防止重复操作
    FindWindow(ID_STOP)->Disable();
    FindWindow(ID_RESTART)->Disable();
    FindWindow(ID_START)->Disable();

    // 在后台线程中执行停止操作
    auto* thread = new ServiceOperationThread(
        this, service_name, ServiceOperationThread::OP_STOP);
    thread->Run();
}

void ServiceManagerFrame::OnRestart(wxCommandEvent& event) {
    const auto service_name = service_list_view_->get_selected_service_name();

    if (service_name.empty()) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // 检查服务是否正在运行
    bool isRunning = ServiceSystem::IsRunning(service_name);

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to restart service \"%s\"?", service_name),
        "Confirm Restart",
        wxYES_NO | wxICON_QUESTION);
    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    // 显示等待提示
    set_status_text(wxString::Format("Restarting service: %s...", service_name));
    // service_list_->SetItemTextColour(selected, wxColour(255, 165, 0)); // 橙色表示操作中

    // 禁用按钮防止重复操作
    FindWindow(ID_STOP)->Disable();
    FindWindow(ID_RESTART)->Disable();
    FindWindow(ID_START)->Disable();

    // 在后台线程中执行重启操作
    auto* thread = new ServiceOperationThread(
        this, service_name, ServiceOperationThread::OP_RESTART);
    thread->Run();
}

void ServiceManagerFrame::OnOperationComplete(wxThreadEvent& event) {
    // 获取操作结果
    const wxString message = event.GetString();
    const bool success = event.GetExtraLong() != 0;

    const auto service_name = service_list_view_->get_selected_service_name();

    if (service_name.empty()) {
        set_status_text(message);
        return;
    }

    // 更新状态文本
    set_status_text(message, 5000);

    // 根据操作结果更新UI
    if (success) {
        // 刷新服务状态
        const DWORD status = ServiceSystem::GetServiceStatus(service_name);

        // // 更新列表项颜色
        // if (status == SERVICE_RUNNING) {
        //     service_list_->SetItemTextColour(selected, *wxGREEN);
        // } else if (status == SERVICE_STOPPED) {
        //     service_list_->SetItemTextColour(selected, *wxRED);
        // } else {
        //     service_list_->SetItemTextColour(selected, *wxBLUE);
        // }

        // 更新状态文本
        const auto row = service_list_model_->find_next_row_with_prefix(service_name, 0, true, true);
        service_list_model_->SetValueByRow(ServiceStatusToString(status), row, 0);
    } else {
        // // 操作失败，恢复原状态
        // const DWORD status = ServiceSystem::GetServiceStatus(service_name);
        // if (status == SERVICE_RUNNING) {
        //     service_list_->SetItemTextColour(selected, *wxGREEN);
        // } else {
        //     service_list_->SetItemTextColour(selected, *wxRED);
        // }

        // 显示错误消息
        wxMessageBox(message, "Error", wxOK | wxICON_ERROR);
    }

    // 重新启用按钮
    FindWindow(ID_STOP)->Enable();
    FindWindow(ID_RESTART)->Enable();
    FindWindow(ID_START)->Enable();
}

void ServiceManagerFrame::set_status_text(const wxString& text, const int time_ms)
{
    SetStatusText(text);
    auto& time_mgr = utils::TimerManager::Get();
    if (status_text_reset_timer) {
        time_mgr.RemoveTimer(status_text_reset_timer);
    }
    if (time_ms >= 0) {
        status_text_reset_timer = time_mgr.AddTimer(time_ms, [this] {
            SetStatusText("");
        });
    }
}
