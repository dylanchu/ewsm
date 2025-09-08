#include "keyboard_hooker.hpp"

utils::KeyboardRemapper::~KeyboardRemapper()
{
    uninstall_keyboard_hook();
}

bool utils::KeyboardRemapper::update_hook_state()
{
    if (kbHook) {
        if (modified_keys.empty()) {
            uninstall_keyboard_hook();
        }
        return true;
    }
    else {
        if (modified_keys.empty()) {
            return true;
        }
        return install_keyboard_hook();
    }
}

/* install low level keyboard hook */
bool utils::KeyboardRemapper::install_keyboard_hook()
{
    if (!kbHook) {
        HMODULE curModuleHandle = GetModuleHandleW(NULL);
        kbHook = SetWindowsHookExW(WH_KEYBOARD_LL, &keyboard_hook_callback, curModuleHandle, 0);
        return kbHook != NULL;
    }
    return false;
}

bool utils::KeyboardRemapper::uninstall_keyboard_hook()
{
    if (kbHook) {
        auto ret = UnhookWindowsHookEx(kbHook);
        kbHook = NULL;
        return ret;
    }
    return false;
}

bool utils::KeyboardRemapper::is_hook_installed()
{
    return kbHook;
}

DWORD utils::KeyboardRemapper::get_target_key(DWORD original)
{
    auto it = modified_keys.find(original);
    return it == modified_keys.end() ? 0 : it->second;
}

DWORD utils::KeyboardRemapper::map(DWORD src_key, DWORD as_key)
{
    auto last = modified_keys[src_key];
    if (as_key == KEY_NOTHING) {
        modified_keys.erase(src_key);
    }
    else {
        modified_keys[src_key] = as_key;
    }
    return last;
}

LRESULT CALLBACK utils::KeyboardRemapper::keyboard_hook_callback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || is_simulating_input) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        auto it = modified_keys.find(kb->vkCode);
        if (it != modified_keys.end() && it->second != KEY_NOTHING) {
            if (it->second != KEY_BLOCKED) {
                simulate_key_down(it->second);
            }
            return 1; // return non zero to block the key
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYUP) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        auto it = modified_keys.find(kb->vkCode);
        if (it != modified_keys.end() && it->second != KEY_NOTHING) {
            if (it->second != KEY_BLOCKED) {
                simulate_key_up(it->second);
            }
            return 1; // return non zero to block the key
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

std::unordered_map<DWORD, DWORD> utils::KeyboardRemapper::modified_keys;
