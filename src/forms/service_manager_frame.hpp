#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>

#include "ecs/component/service/service_comp.hpp"


class ServiceOperationThread;


class ServiceManagerFrame : public wxFrame {
    using Super = wxFrame;
    ServiceManagerFrame();
public:
    // 获取单例实例
    static ServiceManagerFrame* Get() {
        if (!m_instance) {
            m_instance = new ServiceManagerFrame();
        }
        return m_instance;
    }
    inline static ServiceManagerFrame* m_instance = nullptr;
    ServiceManagerFrame(const ServiceManagerFrame&) = delete;
    ServiceManagerFrame& operator=(const ServiceManagerFrame&) = delete;

    static bool IsShowing() noexcept;

    void OnRefresh(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);

private:
    wxListCtrl* m_serviceList = nullptr;
    wxTextCtrl* searchCtrl = nullptr;
    inline static auto m_lastSize = wxSize(800, 600);
    
    enum {
        ID_INSTALL = wxID_HIGHEST + 1,
        ID_UNINSTALL,
        ID_START,
        ID_STOP,
        ID_RESTART,
        ID_REFRESH
    };
    
    void RefreshServiceList();
    void ShowServiceDetails(const wxString& serviceName);

    void OnServiceSelected(wxListEvent& event);
    void OnInstall(wxCommandEvent& event);
    void OnUninstall(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void AddServiceToList(const ServiceComp& service);
    void InitializeImageList();
    void OnTimer(wxTimerEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnRestart(wxCommandEvent& event);
    void OnOperationComplete(wxThreadEvent& event);


    friend ServiceOperationThread;
};