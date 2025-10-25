#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include "ecs/component/service/service_comp.hpp"


class ServiceOperationThread;

class ServiceListModel final : public wxDataViewListStore
{
public:
    enum
    {
        Col_ServiceName,
        Col_Status,
        Col_StartType,
        Col_Description,
        Col_Max
    };

    ServiceListModel();
    void add_service(const ServiceComp& service);
    unsigned int find_next_row_with_prefix(wxString prefix, unsigned int cur_idx, bool ignore_case = true, bool loop = true) const;
};

class ServiceListViewCtrl final : public wxDataViewListCtrl
{
public:
    ServiceListViewCtrl() = default;
    ServiceListViewCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    void focus_next_item_by_prefix(const wxString& prefix);
    void on_service_selected(wxDataViewEvent& event);
    wxString get_selected_service_name() const;
};

class ServiceManagerFrame : public wxFrame {
    using Super = wxFrame;
    ServiceManagerFrame();
public:
    // 获取单例实例
    static ServiceManagerFrame* Get() {
        if (!instance) {
            instance = new ServiceManagerFrame();
        }
        return instance;
    }
    inline static ServiceManagerFrame* instance = nullptr;
    ServiceManagerFrame(const ServiceManagerFrame&) = delete;
    ServiceManagerFrame& operator=(const ServiceManagerFrame&) = delete;

    static bool IsShowing() noexcept;

    void OnRefresh(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);

private:
    ServiceListModel* service_list_model_ = nullptr;
    ServiceListViewCtrl* service_list_view_ = nullptr;
    wxTextCtrl* search_ctrl_ = nullptr;
    inline static auto last_size_ = wxSize(1200, 800);
    
    enum {
        ID_INSTALL = wxID_HIGHEST + 1,
        ID_UNINSTALL,
        ID_START,
        ID_STOP,
        ID_RESTART,
        ID_REFRESH
    };
    
    void RefreshServiceList();
    void ShowServiceDetails(const wxString& service_name);

    void OnInstall(wxCommandEvent& event);
    void OnUninstall(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void InitializeImageList();
    void OnStop(wxCommandEvent& event);
    void OnRestart(wxCommandEvent& event);
    void OnOperationComplete(wxThreadEvent& event);
    void set_status_text(const wxString& text, int time_ms = 5000);

    int status_text_reset_timer = 0;

    friend ServiceOperationThread;
    friend ServiceListViewCtrl;
};