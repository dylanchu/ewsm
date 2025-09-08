#pragma once
#include <unordered_map>
#include "wx/wxprec.h"
#include "debug_utils.hpp"

namespace utils
{

    class KeyboardRemapper
    {
    public:
        ~KeyboardRemapper();
        static KeyboardRemapper& Get()
        {
            static KeyboardRemapper instance;
            return instance;
        }
        bool update_hook_state();
        bool install_keyboard_hook();
        bool uninstall_keyboard_hook();
        bool is_hook_installed();
        DWORD get_target_key(DWORD src);
        DWORD map(DWORD src_key, DWORD as_key);

        constexpr static DWORD KEY_NOTHING = 0x00;
        constexpr static DWORD KEY_BLOCKED = 0xFF;
        static LRESULT CALLBACK keyboard_hook_callback(int nCode, WPARAM wParam, LPARAM lParam);
        static void simulate_key_down(DWORD key_code);
        static void simulate_key_up(DWORD key_code);

    private:
        KeyboardRemapper();
        KeyboardRemapper(KeyboardRemapper&) = delete;
        KeyboardRemapper(KeyboardRemapper&&) = delete;
        static std::unordered_map<DWORD, DWORD> modified_keys; // {origin->modified}, 0: blocked
        inline static bool is_simulating_input = false;
        HHOOK kbHook = NULL;
    };


    inline KeyboardRemapper::KeyboardRemapper()
    {
        modified_keys.reserve(0xFF);
    }

    inline void KeyboardRemapper::simulate_key_down(DWORD key_code)
    {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key_code;
        is_simulating_input = true;
        SendInput(1, &input, sizeof(INPUT));
        is_simulating_input = false;
    }

    inline void KeyboardRemapper::simulate_key_up(DWORD key_code)
    {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key_code;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        is_simulating_input = true;
        SendInput(1, &input, sizeof(INPUT));
        is_simulating_input = false;
    }

    template <typename T>
    wxMenu* create_keyboard_remapper_menu(T* agent)
    {
        auto& remapper = KeyboardRemapper::Get();
        auto menu = new wxMenu();
        // block lwin
        auto item = menu->AppendCheckItem(wxID_ANY, wxT("Block LWin"));
        item->Check(remapper.is_hook_installed() && remapper.get_target_key(VK_LWIN) == KeyboardRemapper::KEY_BLOCKED);
        agent->Bind(wxEVT_MENU, [=, &remapper](wxCommandEvent& event) {
            DEBUG_MSG("--- LWIN block event: %d", event.IsChecked());
            if (event.IsChecked()) {
                    remapper.map(VK_LWIN, KeyboardRemapper::KEY_BLOCKED);
                    if (!remapper.update_hook_state()) {
                        wxMessageBox("Failed to install keyboard hook", "Error", wxOK | wxICON_ERROR);
                        item->Check(false); // or: menu->Check(event.GetId(), false);
                    }
            }
            else {
                remapper.map(VK_LWIN, KeyboardRemapper::KEY_NOTHING);
                remapper.update_hook_state();
            }
        }, item->GetId());
        // hot corner
        //     remapper.simulate_key_down(VK_RWIN);
        //     remapper.simulate_key_down(VK_TAB);
        //     remapper.simulate_key_up(VK_TAB);
        //     remapper.simulate_key_up(VK_RWIN);
        return menu;
    }

} // namespace utils
