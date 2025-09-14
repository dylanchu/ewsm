#pragma once
#include <wx/wx.h>
#include <windows.h>

#define UNKNOWN_DUMMY_STATUS 0xffffffff

struct ServiceComp;
class ScmWrapper;

class ServiceWrapper final {
public:
    ServiceWrapper() = default;
    ServiceWrapper(SC_HANDLE scm, const wxString& service_name, DWORD access);
    ServiceWrapper(SC_HANDLE service_handle, const wxString& service_name);
    ~ServiceWrapper();
    bool start() const;
    [[maybe_unused]] bool stop() const;
    DWORD get_status() const;
    bool is_running() const;
    bool wait_for_status(DWORD desired_state, DWORD timeout = 30000) const;
    bool is_valid() const { return service_handle != nullptr; }
    ServiceComp get_service_info() const;

    wxString service_name;
    SC_HANDLE service_handle;
};

class ScmWrapper final
{
public:
    explicit ScmWrapper(DWORD access = SC_MANAGER_ALL_ACCESS);
    ~ScmWrapper();
    ServiceWrapper open_service(const wxString& service_name, DWORD access = SERVICE_ALL_ACCESS) const;
    bool is_service_running(const wxString& service_name) const;
    bool install_service(const ServiceComp& info) const;
    bool uninstall_service(const wxString& service_name) const;
    bool start_service(const wxString& service_name) const;
    bool stop_service(const wxString& service_name) const;
    static bool stop_service(const ServiceWrapper& service);
    bool restart_service(const wxString& service_name) const;
    bool exists(const wxString& service_name) const;
    DWORD get_service_status(const wxString& service_name) const;
    std::vector<ServiceComp> get_all_services() const;
    ServiceComp get_service_info(const wxString& service_name) const;

    bool is_valid() const { return scm_handle != nullptr; }

private:
    SC_HANDLE scm_handle;
};
