// ReSharper disable file CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
#include "service_system.hpp"
#include "ecs/context/service/scm.hpp"


ewsm::ServiceSystem::ServiceSystem(entt::registry& registry) : SystemBase(registry)
{
}

bool ewsm::ServiceSystem::Install(const ServiceComp& info)
{
    return ScmWrapper(SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE).install_service(info);
}

bool ewsm::ServiceSystem::Uninstall(const wxString& service_name)
{
    return ScmWrapper(SC_MANAGER_CONNECT).uninstall_service(service_name);
}

bool ewsm::ServiceSystem::Start(const wxString& service_name)
{
    return ScmWrapper(SC_MANAGER_CONNECT).start_service(service_name);
}

bool ewsm::ServiceSystem::Stop(const wxString& service_name)
{
    if (const auto scm = ScmWrapper(SC_MANAGER_CONNECT); scm.is_valid()) {
        return scm.stop_service(service_name);
    }
    return false;
}

bool ewsm::ServiceSystem::Restart(const wxString& service_name)
{
    return ScmWrapper(SC_MANAGER_CONNECT).restart_service(service_name);
}

bool ewsm::ServiceSystem::Exists(const wxString& service_name)
{
    auto scm = ScmWrapper(SC_MANAGER_CONNECT);
    return scm.exists(service_name);
}

bool ewsm::ServiceSystem::IsRunning(const wxString& service_name)
{
    return ScmWrapper(SC_MANAGER_CONNECT).is_service_running(service_name);
    // return GetServiceStatus(service_name) == SERVICE_RUNNING;
}


// 获取服务详细信息
ServiceComp GetServiceInfo(const wxString& service_name) {
    const auto scm = ScmWrapper(SC_MANAGER_CONNECT);
    return scm.get_service_info(service_name);
}

DWORD ewsm::ServiceSystem::GetServiceStatus(const wxString& service_name)
{
    return ScmWrapper(SC_MANAGER_CONNECT).get_service_status(service_name);
}

wxString ewsm::ServiceSystem::ServiceStatusToString(DWORD status)
{
    switch (status) {
    case SERVICE_STOPPED:
        return "Stopped";  // service is not running
    case SERVICE_START_PENDING:
        return "Starting";
    case SERVICE_STOP_PENDING:
        return "Stopping";
    case SERVICE_RUNNING:
        return "Running";
    case SERVICE_CONTINUE_PENDING:
        return "Resuming"; // service is resuming after being paused
    case SERVICE_PAUSE_PENDING:
        return "Pausing";  // service is pausing
    case SERVICE_PAUSED:
        return "Paused";   // service is paused
    default:
        if (status == 0) {
            return "Not Installed";
        }
        return wxString::Format("Unknown (0x%X)", status);
    }
}

wxString ewsm::ServiceSystem::StartTypeToString(DWORD start_type)
{
    switch (start_type) {
    case SERVICE_BOOT_START:
        return "Boot - Started by the system loader";
    case SERVICE_SYSTEM_START:
        return "System - Started during kernel initialization";
    case SERVICE_AUTO_START:
        return "Automatic - Started automatically at system startup";
    case SERVICE_DEMAND_START:
        return "Manual - Started manually by a user or application";
    case SERVICE_DISABLED:
        return "Disabled - Cannot be started";
    // case SERVICE_DELAYED_AUTO_START: return "Delayed Auto - Started shortly after system startup";
    // case SERVICE_TRIGGER_START:    return "Trigger Start - Started when a specific event occurs";
    // case SERVICE_AUTO_START_PREVENTING_SHUTDOWN:
    //     return "Auto (Prevent Shutdown) - Automatic start that prevents system shutdown";
    default:
        return wxString::Format("Unknown (0x%X) - Unrecognized start type", start_type);
    }
}

std::vector<ServiceComp> ewsm::ServiceSystem::GetAllServices()
{
    const auto scm = ScmWrapper(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    return scm.get_all_services();
}
