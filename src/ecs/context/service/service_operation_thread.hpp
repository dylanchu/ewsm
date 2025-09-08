#pragma once
#include <wx/wx.h>
#include <wx/thread.h>

class ServiceManagerFrame;

// 后台工作线程
class ServiceOperationThread : public wxThread {
public:
    enum OperationType {
        OP_STOP,
        OP_RESTART
    };

    ServiceOperationThread(ServiceManagerFrame* frame, const wxString& serviceName, OperationType op)
        : wxThread(wxTHREAD_DETACHED), m_frame(frame), m_serviceName(serviceName), m_operation(op) {}

    ExitCode Entry() override;

private:
    ServiceManagerFrame* m_frame;
    wxString m_serviceName;
    OperationType m_operation;
};