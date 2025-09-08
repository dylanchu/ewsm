#pragma once
#include <windows.h>
#include <shellapi.h>


namespace utils
{
    static bool IsRunAsAdmin() {
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        BOOL isAdmin = FALSE;

        if (AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup)) {

            if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
                isAdmin = FALSE;
            }
            FreeSid(AdministratorsGroup);
            }

        return isAdmin == TRUE;
    }

    static bool RestartAsAdmin() {
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0) {
            return false;
        }

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = path;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (!ShellExecuteExW(&sei)) {
            DWORD err = GetLastError();
            if (err == ERROR_CANCELLED) {
                // 用户拒绝了UAC提示
                return false;
            }
            return false;
        }

        return true;
    }

    // int main(int argc, char** argv) {
    //     // 检查是否以管理员身份运行
    //     if (!IsRunAsAdmin()) {
    //         // 请求管理员权限
    //         if (RestartAsAdmin()) {
    //             // 新实例已启动，退出当前实例
    //             return 0;
    //         } else {
    //             // 处理权限请求失败的情况
    //             MessageBox(NULL,
    //                 L"This application requires administrator privileges to function properly.",
    //                 L"Permission Required",
    //                 MB_OK | MB_ICONERROR);
    //             return 1;
    //         }
    //     }
    //
    //     // 正常启动应用程序
    //     // ...
    // }

}