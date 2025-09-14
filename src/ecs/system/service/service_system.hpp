#pragma once
#include <wx/wx.h>
#include <winsvc.h>
#include "ecs/component/service/service_comp.hpp"
#include "ecs/system/system_base.hpp"
#include "utils/win_utils.hpp"


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

        static DWORD GetServiceStatus(const wxString& service_name);
        static wxString ServiceStatusToString(DWORD status);
        static wxString StartTypeToString(DWORD start_type);
        static std::vector<ServiceComp> GetAllServices();
    };

} // namespace ewsm
