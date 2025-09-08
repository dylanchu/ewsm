#pragma once
#include <wx/wx.h>

#include "ecs/system/service/service_system.hpp"

class ServiceDetailDialog : public wxDialog {
public:
    ServiceDetailDialog(wxWindow* parent, const wxString& serviceName);

private:
    void CreateBasicPanel(wxWindow* parent, const wxString& serviceName);

    void CreateConfigPanel(wxWindow* parent, const wxString& serviceName);

    void CreateLogPanel(wxWindow* parent, const wxString& serviceName);

    void CreatePerfPanel(wxWindow* parent, const wxString& serviceName);

    wxTimer* m_timer;
};