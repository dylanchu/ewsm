#pragma once
#include <wx/wx.h>

struct ServiceComp {
    wxString name;           // 服务名称
    wxString description;    // 服务描述
    wxString binary_path;    // 服务可执行文件路径
    wxString display_name;   // 显示名称
    DWORD service_type;      // 服务类型
    DWORD start_type;        // 启动类型
    DWORD error_control;     // 错误控制级别
    DWORD status;            // 当前状态
    wxString account;        // 运行账户
    wxString dependencies;   // 依赖服务
    wxString group;          // 服务组
    DWORD pid;               // 进程ID（如果正在运行）

    // 构造函数
    explicit ServiceComp(const wxString& name = wxEmptyString,
                const wxString& binary_path = wxEmptyString,
                const wxString& display_name = wxEmptyString,
                const DWORD service_type = SERVICE_WIN32_OWN_PROCESS,
                const DWORD start_type = SERVICE_AUTO_START,
                const DWORD error_control = SERVICE_ERROR_NORMAL)
        : name(name), binary_path(binary_path), display_name(display_name),
          service_type(service_type), start_type(start_type), error_control(error_control) {
        if (this->display_name.IsEmpty()) {
            this->display_name = name;
        }
    }
};

// // 服务信息结构体
// struct ServiceInfo {
//     wxString name;          // 服务名称
//     wxString display_name;   // 显示名称
//     wxString description;   // 描述
//     DWORD start_type;        // 启动类型
//     wxString binary_path;    // 可执行文件路径
//     DWORD status;           // 当前状态
//     wxString account;       // 运行账户
//     wxString dependencies;  // 依赖服务
//     wxString group;         // 服务组
//     DWORD pid;              // 进程ID（如果正在运行）
// };

// struct ServiceInfo {
//     wxString name;          // 服务名称
//     wxString binary_path;    // 服务可执行文件路径
//     wxString display_name;   // 显示名称
//     wxString description;   // 服务描述
//     wxString dependencies;  // 依赖服务
//     wxString account;       // 运行账户
//     wxString password;      // 账户密码
//     DWORD serviceType;      // 服务类型
//     DWORD start_type;        // 启动类型
//     DWORD errorControl;     // 错误控制级别
//
//     // 构造函数
//     ServiceInfo(const wxString& name = wxEmptyString,
//                 const wxString& binary_path = wxEmptyString,
//                 const wxString& display_name = wxEmptyString,
//                 DWORD serviceType = SERVICE_WIN32_OWN_PROCESS,
//                 DWORD start_type = SERVICE_AUTO_START,
//                 DWORD errorControl = SERVICE_ERROR_NORMAL)
//         : name(name), binary_path(binary_path), display_name(display_name),
//           serviceType(serviceType), start_type(start_type), errorControl(errorControl) {
//         if (this->display_name.IsEmpty()) {
//             this->display_name = name;
//         }
//     }
// };
