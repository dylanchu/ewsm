// ReSharper disable file CppLocalVariableMayBeConst
#include "service_system.hpp"

#define UNKNOWN_DUMMY_STATUS 0xffffffff

ewsm::ServiceSystem::ServiceSystem(entt::registry& registry) : SystemBase(registry)
{
}

bool ewsm::ServiceSystem::Install(const ServiceComp& info)
{
    const SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = ::CreateServiceW(
        scm_handle,
        info.name.wc_str(),
        info.display_name.wc_str(),
        SERVICE_ALL_ACCESS,
        info.service_type,
        info.start_type,
        info.error_control,
        info.binary_path.wc_str(),
        nullptr, nullptr, nullptr, nullptr, nullptr);

    bool success = false;
    if (service_handle) {
        wxLogMessage("Service '%s' installed successfully", info.name);
        success = true;
    }
    else {
        LogError("CreateService failed");
    }

    if (service_handle) ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return success;
}
bool ewsm::ServiceSystem::Uninstall(const wxString& service_name)
{
    const SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = OpenService(scm_handle, service_name, SERVICE_ALL_ACCESS);
    if (!service_handle) {
        ::CloseServiceHandle(scm_handle);
        return false;
    }

    // 尝试停止服务
    StopService(service_handle);

    bool success = false;
    if (::DeleteService(service_handle)) {
        wxLogMessage("Service '%s' uninstalled successfully", service_name);
        success = true;
    }
    else {
        LogError("DeleteService failed");
    }

    ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return success;
}
bool ewsm::ServiceSystem::Start(const wxString& service_name)
{
    const SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = OpenService(scm_handle, service_name, SERVICE_START);
    if (!service_handle) {
        ::CloseServiceHandle(scm_handle);
        return false;
    }

    bool success = false;
    if (::StartServiceW(service_handle, 0, nullptr)) {
        wxLogMessage("Service '%s' started successfully", service_name);
        success = true;
    }
    else {
        if (const DWORD err = ::GetLastError(); err == ERROR_SERVICE_ALREADY_RUNNING) {
            wxLogMessage("Service '%s' is already running", service_name);
            success = true;
        }
        else {
            LogError("StartService failed");
        }
    }

    ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return success;
}
bool ewsm::ServiceSystem::Stop(const wxString& service_name)
{
    const SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = OpenService(scm_handle, service_name, SERVICE_STOP);
    if (!service_handle) {
        ::CloseServiceHandle(scm_handle);
        return false;
    }

    const bool success = StopService(service_handle);

    ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return success;
}
bool ewsm::ServiceSystem::Restart(const wxString& service_name)
{
    if (!Stop(service_name)) return false;
    return Start(service_name);
}
bool ewsm::ServiceSystem::Exists(const wxString& service_name)
{
    const auto scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = OpenService(scm_handle, service_name, SERVICE_QUERY_STATUS);
    const bool exists = (service_handle != nullptr);

    if (service_handle) ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return exists;
}
bool ewsm::ServiceSystem::IsRunning(const wxString& service_name)
{
    const SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return false;

    const SC_HANDLE service_handle = OpenService(scm_handle, service_name, SERVICE_QUERY_STATUS);
    if (!service_handle) {
        ::CloseServiceHandle(scm_handle);
        return false;
    }

    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded;
    bool running = false;

    if (::QueryServiceStatusEx(
            service_handle,
            SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&status),
            sizeof(SERVICE_STATUS_PROCESS),
            &bytesNeeded)) {
        running = (status.dwCurrentState == SERVICE_RUNNING);
    }
    else {
        LogError("QueryServiceStatusEx failed");
    }

    ::CloseServiceHandle(service_handle);
    ::CloseServiceHandle(scm_handle);
    return running;
}


// 获取服务详细信息
ServiceComp GetServiceInfo(const wxString& serviceName) {
    ServiceComp info;
    info.name = serviceName;

    SC_HANDLE scm_handle = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm_handle) {
        info.status = 0; // Not installed
        return info;
    }

    SC_HANDLE service_handle = OpenService(scm_handle, serviceName.wc_str(),
        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

    if (!service_handle) {
        CloseServiceHandle(scm_handle);
        info.status = 0; // Not installed
        return info;
    }

    // 获取服务状态
    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded;
    if (QueryServiceStatusEx(service_handle, SC_STATUS_PROCESS_INFO,
        reinterpret_cast<LPBYTE>(&status), sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        info.status = status.dwCurrentState;
    } else {
        info.status = UNKNOWN_DUMMY_STATUS;
    }

    // 获取服务配置
    DWORD bufferSize = 0;
    QueryServiceConfig(service_handle, nullptr, 0, &bufferSize);

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        std::vector<BYTE> buffer(bufferSize);
        LPQUERY_SERVICE_CONFIG config = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(buffer.data());

        if (QueryServiceConfig(service_handle, config, bufferSize, &bytesNeeded)) {
            info.start_type = config->dwStartType;
            info.binary_path = config->lpBinaryPathName;
            info.account = config->lpServiceStartName;
            info.display_name = config->lpDisplayName;
            info.group = config->lpLoadOrderGroup;

            // 获取依赖项
            if (config->lpDependencies) {
                const wchar_t* dep = config->lpDependencies;
                while (*dep) {
                    if (!info.dependencies.IsEmpty()) info.dependencies += ", ";
                    info.dependencies += dep;
                    dep += wcslen(dep) + 1;
                }
            }
        }
    }

    // 获取服务描述
    SERVICE_DESCRIPTIONW sd;
    DWORD bytesReturned;
    if (QueryServiceConfig2(service_handle, SERVICE_CONFIG_DESCRIPTION,
        reinterpret_cast<LPBYTE>(&sd), sizeof(SERVICE_DESCRIPTIONW), &bytesReturned)) {
        if (sd.lpDescription) {
        }
    }


    CloseServiceHandle(service_handle);
    CloseServiceHandle(scm_handle);

    return info;
}

DWORD ewsm::ServiceSystem::GetServiceStatus(const wxString& serviceName)
{
    SC_HANDLE scm_handle = open_scm();
    if (!scm_handle) return UNKNOWN_DUMMY_STATUS;

    SC_HANDLE service_handle = OpenService(scm_handle, serviceName, SERVICE_QUERY_STATUS);
    if (!service_handle) {
        CloseServiceHandle(scm_handle);
        return UNKNOWN_DUMMY_STATUS;
    }

    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded;
    DWORD result = UNKNOWN_DUMMY_STATUS;

    if (QueryServiceStatusEx(
        service_handle,
        SC_STATUS_PROCESS_INFO,
        reinterpret_cast<LPBYTE>(&status),
        sizeof(SERVICE_STATUS_PROCESS),
        &bytesNeeded
    )) {
        result = status.dwCurrentState;
    } else {
        LogError("QueryServiceStatusEx failed");
    }

    CloseServiceHandle(service_handle);
    CloseServiceHandle(scm_handle);
    return result;
}

wxString ewsm::ServiceSystem::ServiceStatusToString(DWORD status)
{
    switch (status) {
    case SERVICE_STOPPED:
        return "Stopped - The service is not running";
    case SERVICE_START_PENDING:
        return "Starting - The service is starting";
    case SERVICE_STOP_PENDING:
        return "Stopping - The service is stopping";
    case SERVICE_RUNNING:
        return "Running - The service is running normally";
    case SERVICE_CONTINUE_PENDING:
        return "Resuming - The service is resuming after being paused";
    case SERVICE_PAUSE_PENDING:
        return "Pausing - The service is pausing";
    case SERVICE_PAUSED:
        return "Paused - The service is paused";
    default:
        if (status == 0) return "Not Installed - The service is not installed";
        return wxString::Format("Unknown (0x%X) - Unrecognized service status", status);
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
    std::vector<ServiceComp> services;

    // 打开服务控制管理器
    SC_HANDLE scm_handle = ::OpenSCManager(
        nullptr,                    // 本地计算机
        nullptr,                    // SERVICES_ACTIVE_DATABASE
        SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE
    );

    if (!scm_handle) {
        LogError("OpenSCManager failed");
        return services;
    }

    // 第一次调用获取所需缓冲区大小
    DWORD bytesNeeded = 0;
    DWORD serviceCount = 0;
    DWORD resumeHandle = 0;

    EnumServicesStatusEx(
        scm_handle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_TYPE_ALL,
        SERVICE_STATE_ALL,
        nullptr,
        0,
        &bytesNeeded,
        &serviceCount,
        &resumeHandle,
        nullptr
    );

    if (GetLastError() != ERROR_MORE_DATA) {
        LogError("EnumServicesStatusEx failed to get buffer size");
        CloseServiceHandle(scm_handle);
        return services;
    }

    // 分配缓冲区
    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS_PROCESS servicesBuffer =
        reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer.data());

    // 实际枚举服务
    if (!EnumServicesStatusEx(
        scm_handle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_TYPE_ALL,
        SERVICE_STATE_ALL,
        buffer.data(),
        bytesNeeded,
        &bytesNeeded,
        &serviceCount,
        &resumeHandle,
        nullptr
    )) {
        LogError("EnumServicesStatusEx failed");
        CloseServiceHandle(scm_handle);
        return services;
    }

    // 处理每个服务
    for (DWORD i = 0; i < serviceCount; i++) {
        ServiceComp info;
        ENUM_SERVICE_STATUS_PROCESS& service = servicesBuffer[i];

        // 基本服务信息
        info.name = service.lpServiceName;
        info.display_name = service.lpDisplayName;
        info.status = service.ServiceStatusProcess.dwCurrentState;
        info.pid = service.ServiceStatusProcess.dwProcessId;

        // 获取详细配置信息
        SC_HANDLE service_handle = OpenService(
            scm_handle,
            service.lpServiceName,
            SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS
        );

        if (service_handle) {
            // GetServiceConfig(service_handle, info); // 获取服务配置
            // GetServiceDescription(service_handle, info); // 获取服务描述
            CloseServiceHandle(service_handle);
        }
        services.push_back(info);
    }

    CloseServiceHandle(scm_handle);
    return services;
}
