#pragma once
#include "wx/wxprec.h"

class wxFileConfig;


#define SETTING_FIELD(Type, Name, Default) Type Name = Default;
#define GENERAL_SETTING_FIELDS \
    SETTING_FIELD(bool, auto_start, false) \
    /* -- end -- */
#define HOTKEYS_SETTING_FIELDS \
    SETTING_FIELD(int, capture_modifiers, WXK_CONTROL) \
    SETTING_FIELD(int, capture_key_code, WXK_F1) \
    /* -- end -- */
#undef SETTING_FIELD


namespace ewsm
{
    struct Settings
    {
#define SETTING_FIELD(Type, Name, Default) Type Name = Default;
        struct general
        {
            GENERAL_SETTING_FIELDS
        } general;
        struct hotkeys
        {
            HOTKEYS_SETTING_FIELDS
        } hotkeys;
#undef SETTING_FIELD
    };

}
