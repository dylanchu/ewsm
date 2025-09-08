#pragma once
#include <optional>
#include <wx/string.h>
#include "ecs/system/system_base.hpp"


class wxFileName;
class wxFileConfig;

namespace ewsm
{
    struct RefPicComp;
    struct Settings;

    inline static const wxString VENDOR = wxT("chudongyu");

    class SettingsSystem : public SystemBase<SettingsSystem>
    {
    public:
        explicit SettingsSystem(entt::registry& registry);
        ~SettingsSystem() override;

        Settings& GetSettings() const noexcept;

        static void ReadFromConf(Settings& settings);
        static void WriteToConf(const Settings& settings);
        static void Reconcile(Settings& settings);
        static void Apply(const Settings& settings);

    private:
        int auto_save_timer = 0;
    };
}