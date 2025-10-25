cmake_minimum_required(VERSION 3.25)
project(Ewsm LANGUAGES CXX RC)

# 基础配置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 路径
set(SRC_HOME ${CMAKE_SOURCE_DIR}/src)
set(RES_HOME ${CMAKE_SOURCE_DIR}/res)
set(wxWidgets_ROOT_DIR "E:/Envs/wxWidgets-3.2.6/static")
# 递归收集源文件
# file(GLOB_RECURSE ALL_SRCS "src/*.cpp" "src/*.h")
file(GLOB_RECURSE ALL_SRCS ${SRC_HOME}/*.h* ${SRC_HOME}/*.c* ${SRC_HOME}/*.inl)
file(GLOB_RECURSE ALL_RES_FILES ${RES_HOME}/*.ico ${RES_HOME}/*.rc ${RES_HOME}/*.manifest)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_SRCS} ${ALL_RES_FILES})

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 头文件包含
include_directories(
	${CMAKE_SOURCE_DIR}
	${SRC_HOME}
	"${SRC_HOME}/3rd/include"
	# wxWidgets静态库配置
	"${wxWidgets_ROOT_DIR}/include"
	"${wxWidgets_ROOT_DIR}/lib/vc_x64_lib/$<$<CONFIG:Debug>:mswud>$<$<CONFIG:Release>:mswu>"
)

link_directories(
	"${wxWidgets_ROOT_DIR}/lib/vc_x64_lib"
)

# 资源文件,清单文件
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(MY_RC_FILE ${RES_HOME}/app.rc)
    set(MY_MANIFEST ${RES_HOME}/app.manifest)
endif()
# 可执行目标
add_executable(${PROJECT_NAME} ${ALL_SRCS} ${MY_MANIFEST} ${MY_RC_FILE})
# 忘了manifest中的requireAdministrator吧,用这个设置需要管理员权限:
target_link_options(${PROJECT_NAME} PRIVATE "/MANIFESTUAC:level='requireAdministrator'")  # cmake>=3.24.2

# 宏定义
add_compile_definitions(
        __WXMSW__
        wxNO_XML_LIB;wxNO_NET_LIB;wxNO_JPEG_LIB;wxNO_TIFF_LIB
        wxNO_ADV_LIB;wxNO_GL_LIB;wxNO_QA_LIB;wxNO_XRC_LIB
        wxNO_AUI_LIB;wxNO_RIBBON_LIB;wxNO_STC_LIB
        wxBUILD_SHARED=0
        UNICODE
        _UNICODE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG _ITERATOR_DEBUG_LEVEL=2 _STL_DEBUG=1 __WXDEBUG__>
    $<$<CONFIG:Release>:NDEBUG _ITERATOR_DEBUG_LEVEL=0 _STL_DEBUG=0>
)

# 运行时配置
if(MSVC)
	# 明确设置使用静态运行时库 (MT/MTd)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # 关键
	# 下面这样也可以但有warning:
	# target_compile_options(${PROJECT_NAME} PRIVATE
	# 		$<$<CONFIG:Debug>:/MTd>
	# 		$<$<CONFIG:Release>:/MT>
	# )
	target_compile_options(${PROJECT_NAME} PRIVATE /W2)
endif()


# 明确指定项目子系统,否则GUI(WinMain)时会报找不到main
if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"   # Debug 模式用控制台
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" # Release 模式用 GUI
    )
endif ()


# 链接库选择
#set(wxLIBS_DEBUG wxbase32ud wxmsw32ud_core wxpngd wxzlibd)
#set(wxLIBS_RELEASE wxbase32u wxmsw32u_core wxpng wxzlib)
set(wxLIBS_DEBUG wxbase32ud wxmsw32ud_core wxpngd wxzlibd wxmsw32ud_stc wxscintillad)
set(wxLIBS_RELEASE wxbase32u wxmsw32u_core wxpng wxzlib wxmsw32u_stc wxscintilla)
# 系统库链接
target_link_libraries(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
                ${wxLIBS_DEBUG}
                ucrtd.lib msvcrtd.lib  # 添加调试CRT库
        >
        $<$<CONFIG:Release>:
                ${wxLIBS_RELEASE}
                ucrt.lib msvcrt.lib
        >
        comctl32 oleacc version
        kernel32 user32 gdi32 comdlg32 winspool
        shell32 ole32 oleaut32 uuid advapi32
        ws2_32 Rpcrt4 winmm imm32 setupapi
)
target_link_options(${PROJECT_NAME} PRIVATE "/NODEFAULTLIB:LIBCMTD")
target_link_options(${PROJECT_NAME} PRIVATE "/NODEFAULTLIB:LIBCMT")


# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 默认启动项目
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# asan address
#target_compile_options(${PROJECT_NAME} PRIVATE
#    "$<$<CXX_COMPILER_ID:MSVC>:/fsanitize=address>"
#    "$<$<CXX_COMPILER_ID:GNU>:-fsanitize=address>"
#    "$<$<CXX_COMPILER_ID:Clang>:-fsanitize=address>"
#)


# asan ----------------
set(CMAKE_MODULE_PATH "${SRC_HOME}/3rd/sanitizers-cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)
set(SANITIZE_ADDRESS ON)
set(SANITIZE_MEMORY ON)
#sanitizer_add_blacklist_file("asan_blacklist.txt")
add_sanitizers(${PROJECT_NAME})
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /INCREMENTAL:NO")
endif()
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
