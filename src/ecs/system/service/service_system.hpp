#pragma once
#include <wx/wx.h>
#include <winsvc.h>          // 服务控制管理器 API
#include "ecs/component/service/service_comp.hpp"
#include "ecs/system/system_base.hpp"


namespace ewsm
{
    class ServiceSystem : public SystemBase<ServiceSystem>
    {
    public:
        explicit ServiceSystem(entt::registry& registry);
        ~ServiceSystem() override = default;

        static bool Install(const ServiceComp& info);
        static bool Uninstall(const wxString& service_name);
        static bool Start(const wxString& service_name);
        static bool Stop(const wxString& service_name);
        static bool Restart(const wxString& service_name);
        static bool Exists(const wxString& service_name);
        static bool IsRunning(const wxString& service_name);

        static DWORD GetServiceStatus(const wxString& serviceName);
        static wxString ServiceStatusToString(DWORD status);
        static wxString StartTypeToString(DWORD start_type);
        static std::vector<ServiceComp> GetAllServices();

    private:
        static SC_HANDLE open_scm()
        {
            const SC_HANDLE scm_handle = ::OpenSCManagerW(
                nullptr,
                nullptr,
                SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
            if (!scm_handle) {
                LogError("OpenSCManager failed");
            }
            return scm_handle;
        }

        static SC_HANDLE OpenService(const SC_HANDLE scm_handle, const wxString& service_name, const DWORD desired_access)
        {
            const SC_HANDLE service_handle = ::OpenServiceW(
                scm_handle,
                service_name.wc_str(),
                desired_access);

            if (!service_handle) {
                LogError("OpenService failed");
            }

            return service_handle;
        }

        static bool StopService(const SC_HANDLE service_handle)
        {
            SERVICE_STATUS status;
            if (!::ControlService(service_handle, SERVICE_CONTROL_STOP, &status)) {
                // 如果服务未运行，忽略停止错误
                if (const DWORD err = ::GetLastError(); err != ERROR_SERVICE_NOT_ACTIVE) {
                    LogError("ControlService (stop) failed");
                    return false;
                }
                return true;
            }

            // 等待服务停止
            return WaitForStatus(service_handle, SERVICE_STOPPED);
        }

        // 等待服务达到特定状态
        static bool WaitForStatus(const SC_HANDLE service_handle, const DWORD desired_state, const DWORD timeout = 30000)
        {
            SERVICE_STATUS_PROCESS status;
            DWORD bytesNeeded;
            const DWORD start_time = ::GetTickCount();

            while (true) {
                if (!::QueryServiceStatusEx(
                        service_handle,
                        SC_STATUS_PROCESS_INFO,
                        reinterpret_cast<LPBYTE>(&status),
                        sizeof(SERVICE_STATUS_PROCESS),
                        &bytesNeeded)) {
                    LogError("QueryServiceStatusEx failed");
                    return false;
                }

                if (status.dwCurrentState == desired_state) {
                    return true;
                }

                // 检查超时
                if (::GetTickCount() - start_time > timeout) {
                    return false;
                }

                // 根据服务状态等待不同时间
                DWORD waitTime = status.dwWaitHint / 10;
                if (waitTime < 1000) waitTime = 1000;
                if (waitTime > 10000) waitTime = 10000;

                ::Sleep(waitTime);
            }
        }

        // 记录错误信息
        static void LogError(const wxString& message)
        {
            const DWORD err = ::GetLastError();
            wchar_t* err_msg = nullptr;

            ::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                err,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPWSTR>(&err_msg),
                0,
                nullptr);

            const wxString full_message = wxString::Format(
                "%s: %s (Error %d)",
                message,
                err_msg ? wxString(err_msg) : wxString("Unknown error"),
                err);

            wxLogError(full_message);

            if (err_msg) {
                ::LocalFree(err_msg);
            }
        }
    };

} // namespace ewsm
