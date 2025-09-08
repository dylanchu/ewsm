#include "service_manager_frame.hpp"
#include "service_detail_dialog.hpp"
#include "service_install_wizard.hpp"
#include <wx/artprov.h>
#include <wx/treectrl.h>
#include <windows.h>

#include "ecs/context/service/service_operation_thread.hpp"

using namespace ewsm;
inline auto ServiceStatusToString = ServiceSystem::ServiceStatusToString;
inline auto StartTypeToString = ServiceSystem::StartTypeToString;


ServiceManagerFrame::ServiceManagerFrame() :
    wxFrame(nullptr, wxID_ANY, "Service Manager")
{
    // 创建主面板
    wxPanel* panel = new wxPanel(this);

    // 创建服务列表
    m_serviceList = new wxListCtrl(panel, wxID_ANY,
                               wxDefaultPosition, wxDefaultSize,
                               wxLC_REPORT | wxLC_SINGLE_SEL); // 多选 wxLC_MULTIPLE_SEL
    m_serviceList->InsertColumn(0, "Service");
    m_serviceList->InsertColumn(1, "Status");
    m_serviceList->InsertColumn(2, "Start Type");
    m_serviceList->InsertColumn(3, "Description");

    // 创建操作按钮
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(panel, ID_INSTALL, "Install"));
    buttonSizer->Add(new wxButton(panel, ID_UNINSTALL, "Uninstall"));
    buttonSizer->Add(new wxButton(panel, ID_START, "Start"));
    buttonSizer->Add(new wxButton(panel, ID_STOP, "Stop"));
    buttonSizer->Add(new wxButton(panel, ID_RESTART, "Restart"));
    buttonSizer->Add(new wxButton(panel, ID_REFRESH, "Refresh"));

    // 主布局
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_serviceList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);

    panel->SetSizer(mainSizer);

    // 绑定事件
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnInstall, this, ID_INSTALL);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnUninstall, this, ID_UNINSTALL);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnStart, this, ID_START);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnStop, this, ID_STOP);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnRestart, this, ID_RESTART);
    Bind(wxEVT_BUTTON, &ServiceManagerFrame::OnRefresh, this, ID_REFRESH);
    Bind(wxEVT_LIST_ITEM_ACTIVATED, &ServiceManagerFrame::OnServiceSelected, this, m_serviceList->GetId());

    // 初始加载服务列表
    RefreshServiceList();
    // 在构造函数中添加状态栏
    Super::CreateStatusBar();
    Super::SetStatusText("Ready");


    // 在构造函数中添加工具栏
    wxToolBar* toolbar = Super::CreateToolBar();
    toolbar->AddTool(ID_INSTALL, "Install", wxArtProvider::GetBitmap(wxART_PLUS));
    toolbar->AddTool(ID_UNINSTALL, "Uninstall", wxArtProvider::GetBitmap(wxART_MINUS));
    toolbar->AddSeparator();
    toolbar->AddTool(ID_START, "Start", wxArtProvider::GetBitmap(wxART_GO_FORWARD));
    toolbar->AddTool(ID_STOP, "Stop", wxArtProvider::GetBitmap(wxART_STOP));
    toolbar->AddTool(ID_RESTART, "Restart", wxArtProvider::GetBitmap(wxART_REDO));
    toolbar->AddSeparator();
    toolbar->AddTool(ID_REFRESH, "Refresh", wxArtProvider::GetBitmap(wxART_REFRESH));
    toolbar->Realize();
    // 添加搜索框
    searchCtrl = new wxTextCtrl(panel, wxID_ANY, "",
                                         wxDefaultPosition, wxDefaultSize,
                                         wxTE_PROCESS_ENTER);
    searchCtrl->Bind(wxEVT_TEXT, &ServiceManagerFrame::OnSearch, this);

    // 在布局中添加搜索框
    mainSizer->Add(searchCtrl, 0, wxEXPAND | wxALL, 5);

    //服务分组功能
    // 添加分组树控件
    wxTreeCtrl* groupTree = new wxTreeCtrl(panel, wxID_ANY);
    groupTree->AddRoot("All Services");
    groupTree->AppendItem(groupTree->GetRootItem(), "System Services");
    groupTree->AppendItem(groupTree->GetRootItem(), "Application Services");
    mainSizer->Add(groupTree, 0, wxEXPAND | wxALL, 5);
    // 绑定选择事件
    // groupTree->Bind(wxEVT_TREE_SEL_CHANGED, &ServiceManagerFrame::OnGroupSelected, this);

    SetSize(m_lastSize);
    Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        this->m_lastSize = event.GetSize();
        OnSize(event);
    });
    Bind(wxEVT_CLOSE_WINDOW, [this](auto&) {
        this->m_instance = nullptr;
        this->Destroy();
    });

}

bool ServiceManagerFrame::IsShowing() noexcept
{
    return m_instance != nullptr && m_instance->IsShown();
}

// 在操作中更新状态
void ServiceManagerFrame::OnRefresh(wxCommandEvent& event) {
    RefreshServiceList();
    SetStatusText("Service list refreshed");
}

// 搜索处理函数
void ServiceManagerFrame::OnSearch(wxCommandEvent& event) {
    wxString searchText = searchCtrl->GetValue().Lower();
    // 过滤服务列表...
}

void ServiceManagerFrame::RefreshServiceList()
{
    m_serviceList->DeleteAllItems();

    // 获取所有服务
    std::vector<ServiceComp> services = ServiceSystem::GetAllServices();

    for (const auto& service : services) {
        long index = m_serviceList->InsertItem(m_serviceList->GetItemCount(), service.name);
        m_serviceList->SetItem(index, 1, ServiceStatusToString(service.status));
        m_serviceList->SetItem(index, 2, StartTypeToString(service.start_type));
        m_serviceList->SetItem(index, 3, service.description);

        // 设置状态颜色
        if (service.status == SERVICE_RUNNING) {
            m_serviceList->SetItemTextColour(index, *wxGREEN);
        }
        else if (service.status == SERVICE_STOPPED) {
            m_serviceList->SetItemTextColour(index, *wxRED);
        }
        else {
            m_serviceList->SetItemTextColour(index, *wxBLUE);
        }
    }
}

void ServiceManagerFrame::OnServiceSelected(wxListEvent& event)
{
    wxString serviceName = m_serviceList->GetItemText(event.GetIndex());
    ShowServiceDetails(serviceName);
}

void ServiceManagerFrame::ShowServiceDetails(const wxString& serviceName)
{
    ServiceDetailDialog dlg(this, serviceName);
    dlg.ShowModal();
}

void ServiceManagerFrame::OnInstall(wxCommandEvent& event) {
    ServiceInstallWizard wizard(this);
    if (wizard.RunWizard()) {
        // 获取向导中的数据并安装服务
        RefreshServiceList();
    }
}

void ServiceManagerFrame::OnUninstall(wxCommandEvent& event) {
    long selected = m_serviceList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString serviceName = m_serviceList->GetItemText(selected);

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to uninstall the service '%s'?", serviceName),
        "Confirm Uninstall",
        wxYES_NO | wxICON_QUESTION);
    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    if (ServiceSystem::Uninstall(serviceName)) {
        RefreshServiceList();
    }
}

void ServiceManagerFrame::OnStart(wxCommandEvent& event) {
    long selected = m_serviceList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) return;

    wxString serviceName = m_serviceList->GetItemText(selected);
    ServiceSystem::Start(serviceName);
    RefreshServiceList();
}

// 在列表控件中添加状态图标
void ServiceManagerFrame::AddServiceToList(const ServiceComp& service) {
    long index = m_serviceList->InsertItem(m_serviceList->GetItemCount(), service.name);

    // 设置状态图标
    int image_idx = -1;
    switch (service.status) {
        case SERVICE_RUNNING:          image_idx = 0; break; // 绿色图标
        case SERVICE_STOPPED:          image_idx = 1; break; // 红色图标
        case SERVICE_START_PENDING:    image_idx = 2; break; // 黄色闪烁图标
        case SERVICE_STOP_PENDING:     image_idx = 3; break; // 黄色闪烁图标
        case SERVICE_PAUSED:           image_idx = 4; break; // 蓝色图标
        // case SERVICE_DISABLED:         image_idx = 5; break; // 灰色图标
        default:                       image_idx = 6; break; // 问号图标
    }

    m_serviceList->SetItemImage(index, image_idx);

    // 设置状态文本
    m_serviceList->SetItem(index, 1, ServiceStatusToString(service.status));

    // 设置启动类型
    m_serviceList->SetItem(index, 2, StartTypeToString(service.start_type));

    // 设置描述
    m_serviceList->SetItem(index, 3, service.description);
}

// 初始化图像列表
void ServiceManagerFrame::InitializeImageList() {
    auto* imageList = new wxImageList(16, 16);

    // 添加状态图标
    imageList->Add(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_OTHER, wxSize(16, 16))); // 运行中
    imageList->Add(wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_OTHER, wxSize(16, 16))); // 已停止
    imageList->Add(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_OTHER, wxSize(16, 16))); // 启动中
    imageList->Add(wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_OTHER, wxSize(16, 16))); // 停止中
    imageList->Add(wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_OTHER, wxSize(16, 16))); // 已暂停
    imageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16))); // 已禁用
    imageList->Add(wxArtProvider::GetBitmap(wxART_QUESTION, wxART_OTHER, wxSize(16, 16))); // 未知

    m_serviceList->AssignImageList(imageList, wxIMAGE_LIST_SMALL);
}

// 在状态列中使用动画图标
void ServiceManagerFrame::OnTimer(wxTimerEvent& event) {
    for (int i = 0; i < m_serviceList->GetItemCount(); i++) {
        wxString serviceName = m_serviceList->GetItemText(i);
        DWORD status = ServiceSystem::GetServiceStatus(serviceName);

        if (status == SERVICE_START_PENDING || status == SERVICE_STOP_PENDING) {
            // 切换图标创建动画效果
            // int currentImage = m_serviceList->GetItemImage(i);
            auto currentImage = m_serviceList->GetImageList(i);
            // int newImage = (currentImage == 2) ? 7 : 2; // 在两个动画帧之间切换
            // m_serviceList->SetItemImage(i, newImage);
        }
    }
}


void ServiceManagerFrame::OnStop(wxCommandEvent& event) {
    // 获取选中的服务
    long selected = m_serviceList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString serviceName = m_serviceList->GetItemText(selected);

    // 检查服务是否正在运行
    if (!ServiceSystem::IsRunning(serviceName)) {
        wxMessageBox("Service is not running", "Information", wxOK | wxICON_INFORMATION);
        return;
    }

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to stop the service '%s'?", serviceName),
        "Confirm Stop",
        wxYES_NO | wxICON_QUESTION);

    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    // 显示等待提示
    SetStatusText(wxString::Format("Stopping service: %s...", serviceName));
    m_serviceList->SetItemTextColour(selected, wxColour(255, 165, 0)); // 橙色表示操作中

    // 禁用按钮防止重复操作
    FindWindow(ID_STOP)->Disable();
    FindWindow(ID_RESTART)->Disable();
    FindWindow(ID_START)->Disable();

    // 在后台线程中执行停止操作
    ServiceOperationThread* thread = new ServiceOperationThread(
        this, serviceName, ServiceOperationThread::OP_STOP);
    thread->Run();
}

void ServiceManagerFrame::OnRestart(wxCommandEvent& event) {
    // 获取选中的服务
    long selected = m_serviceList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        wxMessageBox("Please select a service first", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString serviceName = m_serviceList->GetItemText(selected);

    // 检查服务是否正在运行
    bool isRunning = ServiceSystem::IsRunning(serviceName);

    // 确认对话框
    wxMessageDialog dlg(this,
        wxString::Format("Are you sure you want to restart the service '%s'?", serviceName),
        "Confirm Restart",
        wxYES_NO | wxICON_QUESTION);
    if (dlg.ShowModal() != wxID_YES) {
        return;
    }

    // 显示等待提示
    SetStatusText(wxString::Format("Restarting service: %s...", serviceName));
    m_serviceList->SetItemTextColour(selected, wxColour(255, 165, 0)); // 橙色表示操作中

    // 禁用按钮防止重复操作
    FindWindow(ID_STOP)->Disable();
    FindWindow(ID_RESTART)->Disable();
    FindWindow(ID_START)->Disable();

    // 在后台线程中执行重启操作
    ServiceOperationThread* thread = new ServiceOperationThread(
        this, serviceName, ServiceOperationThread::OP_RESTART);
    thread->Run();
}

void ServiceManagerFrame::OnOperationComplete(wxThreadEvent& event) {
    // 获取操作结果
    wxString message = event.GetString();
    bool success = event.GetExtraLong() != 0;

    // 获取选中的服务
    long selected = m_serviceList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) {
        SetStatusText(message);
        return;
    }

    wxString serviceName = m_serviceList->GetItemText(selected);

    // 更新状态文本
    SetStatusText(message);

    // 根据操作结果更新UI
    if (success) {
        // 刷新服务状态
        DWORD status = ServiceSystem::GetServiceStatus(serviceName);

        // 更新列表项颜色
        if (status == SERVICE_RUNNING) {
            m_serviceList->SetItemTextColour(selected, *wxGREEN);
        } else if (status == SERVICE_STOPPED) {
            m_serviceList->SetItemTextColour(selected, *wxRED);
        } else {
            m_serviceList->SetItemTextColour(selected, *wxBLUE);
        }

        // 更新状态文本
        wxString statusText = ServiceStatusToString(status);
        m_serviceList->SetItem(selected, 1, statusText);
    } else {
        // 操作失败，恢复原状态
        DWORD status = ServiceSystem::GetServiceStatus(serviceName);
        if (status == SERVICE_RUNNING) {
            m_serviceList->SetItemTextColour(selected, *wxGREEN);
        } else {
            m_serviceList->SetItemTextColour(selected, *wxRED);
        }

        // 显示错误消息
        wxMessageBox(message, "Error", wxOK | wxICON_ERROR);
    }

    // 重新启用按钮
    FindWindow(ID_STOP)->Enable();
    FindWindow(ID_RESTART)->Enable();
    FindWindow(ID_START)->Enable();
}