#include "service_operation_thread.hpp"
#include "ecs/system/paths/paths_system.hpp"
#include "ecs/system/service/service_system.hpp"
#include <wx/event.h>

#include "forms/service_manager_frame.hpp"

using namespace ewsm;

wxDEFINE_EVENT(SERVICE_OPERATION_COMPLETE, wxThreadEvent);


wxThread::ExitCode ServiceOperationThread::Entry()
{
    bool success = false;
    wxString message;

    // 执行操作
    switch (m_operation) {
    case OP_STOP:
        success = ServiceSystem::Stop(m_serviceName);
        message = success ? "Service stopped successfully" : "Failed to stop service";
        break;

    case OP_RESTART:
        success = ServiceSystem::Restart(m_serviceName);
        message = success ? "Service restarted successfully" : "Failed to restart service";
        break;

    default:
        message = "Invalid operation";
    }

    // 发送结果到主线程
    wxThreadEvent event(wxEVT_THREAD, SERVICE_OPERATION_COMPLETE);
    event.SetString(message);
    event.SetExtraLong(success ? 1 : 0);
    wxQueueEvent(m_frame, event.Clone());

    return 0;
}
