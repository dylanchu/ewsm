#pragma once
#include <wx/wx.h>


class ServiceDetailDialog : public wxDialog {
    using Super = wxDialog;
public:
    ServiceDetailDialog(wxWindow* parent, const wxString& service_name);
    ~ServiceDetailDialog() override;

private:
    void CreateBasicPanel(wxWindow* parent, const wxString& service_name);

    void CreateConfigPanel(wxWindow* parent, const wxString& service_name);

    void CreateLogPanel(wxWindow* parent, const wxString& service_name);

    void CreatePerfPanel(wxWindow* parent, const wxString& service_name);

    int update_timer = 0;
};