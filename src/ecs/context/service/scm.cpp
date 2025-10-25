// ReSharper disable file CppExpressionWithoutSideEffects
// ReSharper disable CppTooWideScopeInitStatement
#include "scm.hpp"
#include <windows.h>
#include <winsvc.h>
#include "ecs/component/service/service_comp.hpp"
#include "utils/win_utils.hpp"


#pragma region ServiceWrapper
ServiceWrapper::ServiceWrapper(const SC_HANDLE scm, const wxString& service_name, const DWORD access) :
    service_name(service_name)
{
    this->service_handle = ::OpenServiceW(scm, service_name.wc_str(), access);
}

ServiceWrapper::ServiceWrapper(SC_HANDLE service_handle, const wxString& service_name)
{
    this->service_handle = service_handle;
    this->service_name = service_name;
}

ServiceWrapper::~ServiceWrapper()
{
    if (service_handle) {
        ::CloseServiceHandle(service_handle);
    }
}

bool ServiceWrapper::start() const
{
    if (::StartServiceW(service_handle, 0, nullptr)) {
        wxLogMessage("Service '%s' started successfully", service_name);
        return true;
    }
    if (const DWORD err = ::GetLastError(); err != ERROR_SERVICE_ALREADY_RUNNING) {
        wxLogMessage("Service '%s' is already running", service_name);
        return true;
    }
    utils::LogError(wxString::Format("StartService '%s' failed", service_name));
    return false;
}

bool ServiceWrapper::stop() const
{
    SERVICE_STATUS status;
    if (!::ControlService(service_handle, SERVICE_CONTROL_STOP, &status)) {
        if (const DWORD err = ::GetLastError(); err != ERROR_SERVICE_NOT_ACTIVE) {
            utils::LogError("ControlService (stop) failed");
            return false;
        }
        return true;
    }
    // 等待服务停止
    return wait_for_status(SERVICE_STOPPED);
}

DWORD ServiceWrapper::get_status() const
{
    SERVICE_STATUS_PROCESS status;
    DWORD bytes_needed;
    const auto query_success = ::QueryServiceStatusEx(service_handle, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(SERVICE_STATUS_PROCESS), &bytes_needed);
    if (query_success) {
        return status.dwCurrentState;
    }
    utils::LogError("QueryServiceStatusEx failed");
    return UNKNOWN_DUMMY_STATUS;
}

bool ServiceWrapper::is_running() const
{
    return get_status() == SERVICE_RUNNING;
}

// 等待服务达到特定状态
bool ServiceWrapper::wait_for_status(const DWORD desired_state, const DWORD timeout) const
{
    SERVICE_STATUS_PROCESS status;
    DWORD bytes_needed;
    const DWORD start_time = ::GetTickCount();

    while (true) {
        const auto ret = ::QueryServiceStatusEx(service_handle, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(SERVICE_STATUS_PROCESS), &bytes_needed);
        if (!ret) {
            utils::LogError("QueryServiceStatusEx failed");
            return false;
        }
        if (status.dwCurrentState == desired_state) {
            return true;
        }
        // check timeout
        if (::GetTickCount() - start_time > timeout) {
            return false;
        }
        // wait some time according to wait hint
        DWORD wait_time = status.dwWaitHint / 10;
        if (wait_time < 1000) wait_time = 1000;
        if (wait_time > 10000) wait_time = 10000;
        ::Sleep(wait_time);
    }
}

ServiceComp ServiceWrapper::get_service_info() const
{
    ServiceComp info;
    info.name = service_name;
    if (!is_valid()) {
        return info;
    }
    // 获取服务状态
    SERVICE_STATUS_PROCESS status;
    DWORD bytes_needed;
    if (QueryServiceStatusEx(service_handle, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
        info.status = status.dwCurrentState;
    }
    else {
        info.status = UNKNOWN_DUMMY_STATUS;
    }

    // 获取服务配置
    DWORD buf_size = 0;
    QueryServiceConfig(service_handle, nullptr, 0, &buf_size);

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        std::vector<BYTE> buffer(buf_size);
        const auto config = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(buffer.data());

        if (QueryServiceConfig(service_handle, config, buf_size, &bytes_needed)) {
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
    if (QueryServiceConfig2(service_handle, SERVICE_CONFIG_DESCRIPTION, reinterpret_cast<LPBYTE>(&sd), sizeof(SERVICE_DESCRIPTIONW), &bytesReturned)) {
        if (sd.lpDescription) {
        }
    }

    return info;
}
#pragma endregion ServiceWrapper

// ########################################################################################

#pragma region ScmWrapper
ScmWrapper::ScmWrapper(const DWORD access)
{
    scm_handle = ::OpenSCManagerW(nullptr /* local  machine */, nullptr /* SERVICES_ACTIVE_DATABASE */, access);
    if (!scm_handle) {
        // wxLogError("Open SCManager failed");
        utils::LogError("Open SCManager failed");
    }
}

ScmWrapper::~ScmWrapper()
{
    if (scm_handle) {
        ::CloseServiceHandle(scm_handle);
    }
}

ServiceWrapper ScmWrapper::open_service(const wxString& service_name, const DWORD access) const
{
    return ServiceWrapper(scm_handle, service_name, access);
}

bool ScmWrapper::is_service_running(const wxString& service_name) const
{
    const auto service = open_service(service_name, SERVICE_QUERY_STATUS);
    return !service.is_valid() ? false : service.is_running();
}

bool ScmWrapper::install_service(const ServiceComp& info) const
{
    if (!scm_handle) return false;

    const auto service = ServiceWrapper(
        ::CreateServiceW(
            scm_handle, info.name.wc_str(), info.display_name.wc_str(),
            SERVICE_ALL_ACCESS,
            info.service_type, info.start_type, info.error_control,
            info.binary_path.wc_str(),
            nullptr, nullptr, nullptr, nullptr, nullptr),
        info.name);

    const bool success = service.service_handle != nullptr;
    if (!success) {
        utils::LogError("CreateService failed");
    }
    return success;
}

bool ScmWrapper::uninstall_service(const wxString& service_name) const
{
    if (service_name.empty()) [[unlikely]] {
        return false;
    }

    if (const auto service = open_service(service_name, SERVICE_ALL_ACCESS); service.is_valid()) {
        // 尝试停止服务
        service.stop();

        const bool success = ::DeleteService(service.service_handle);
        if (!success) {
            utils::LogError("DeleteService failed");
        }

        return success;
    }
    return false;
}

bool ScmWrapper::start_service(const wxString& service_name) const
{
    if (service_name.empty()) [[unlikely]] {
        return false;
    }
    if (const auto service = open_service(service_name, SERVICE_START); service.is_valid()) {
        return service.start();
    }
    return false;
}

bool ScmWrapper::stop_service(const wxString& service_name) const
{
    if (service_name.empty()) [[unlikely]] {
        return false;
    }
    if (const auto service = open_service(service_name, SERVICE_STOP); service.is_valid()) {
        return service.stop();
    }
    return false;
}

bool ScmWrapper::stop_service(const ServiceWrapper& service)
{
    return service.stop();
}

bool ScmWrapper::restart_service(const wxString& service_name) const
{
    if (stop_service(service_name)) {
        return start_service(service_name);
    }
    return false;
}

bool ScmWrapper::exists(const wxString& service_name) const
{
    const auto service = open_service(service_name, SERVICE_QUERY_CONFIG);
    return service.is_valid();
}

DWORD ScmWrapper::get_service_status(const wxString& service_name) const
{
    if (!scm_handle) return UNKNOWN_DUMMY_STATUS;
    const auto service = open_service(service_name, SERVICE_QUERY_STATUS);
    if (!service.is_valid()) {
        return UNKNOWN_DUMMY_STATUS;
    }
    return service.get_status();
}

std::vector<ServiceComp> ScmWrapper::get_all_services() const
{
    if (!scm_handle) return {};

    // 第一次调用获取所需缓冲区大小
    DWORD bytes_needed = 0;
    DWORD service_count = 0;
    DWORD resume_handle = 0;

    EnumServicesStatusEx(scm_handle, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, nullptr,
                         0, &bytes_needed, &service_count, &resume_handle, nullptr);

    if (GetLastError() != ERROR_MORE_DATA) {
        utils::LogError("EnumServicesStatusEx failed to get buffer size");
        return {};
    }

    // 分配缓冲区
    std::vector<BYTE> buffer(bytes_needed);
    const auto services_buffer = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer.data());

    // 实际枚举服务
    if (!EnumServicesStatusEx(scm_handle, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, buffer.data(),
                              bytes_needed, &bytes_needed, &service_count, &resume_handle, nullptr)) {
        utils::LogError("EnumServicesStatusEx failed");
        return {};
    }

    std::vector<ServiceComp> services;
    // 处理每个服务
    for (DWORD i = 0; i < service_count; i++) {
        auto& [lpServiceName, lpDisplayName, ServiceStatusProcess] = services_buffer[i];

        // 基本服务信息
        ServiceComp info;
        info = get_service_info(lpServiceName);
        // info.name = lpServiceName;
        // info.display_name = lpDisplayName;
        info.status = ServiceStatusProcess.dwCurrentState;
        info.pid = ServiceStatusProcess.dwProcessId;

        // // 获取详细配置信息
        // const auto service = open_service(lpServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
        // if (service.is_valid()) {
        //     // GetServiceConfig(service_handle, info); // 获取服务配置
        //     // GetServiceDescription(service_handle, info); // 获取服务描述
        // }
        services.push_back(info);
    }

    CloseServiceHandle(scm_handle);
    return services;
}

ServiceComp ScmWrapper::get_service_info(const wxString& service_name) const
{
    const auto service = open_service(service_name, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    return service.get_service_info();
}
#pragma endregion ScmWrapper
