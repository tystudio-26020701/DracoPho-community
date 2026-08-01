qt_add_executable(mark-shot-provider-preference-config-test
    tests/provider_preference_config_test.cpp
    src/settings/provider_preference_config.cpp
    src/settings/provider_preference_config.h
    src/config_value.cpp
    src/config_value.h
)
target_include_directories(mark-shot-provider-preference-config-test PRIVATE src)
target_link_libraries(mark-shot-provider-preference-config-test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)
add_test(NAME provider-preference-config COMMAND mark-shot-provider-preference-config-test)

qt_add_executable(mark-shot-headless-capture-config-test
    tests/headless_capture_config_test.cpp
    src/headless_capture_config.cpp
    src/headless_capture_config.h
    src/config_value.cpp
    src/config_value.h
)
target_include_directories(mark-shot-headless-capture-config-test PRIVATE src)
target_link_libraries(mark-shot-headless-capture-config-test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)
add_test(NAME headless-capture-config COMMAND mark-shot-headless-capture-config-test)

qt_add_executable(mark-shot-i18n-tables-test
    tests/i18n_tables_test.cpp
    src/ui/i18n_tables.h
    src/ui/i18n_zh_cn.cpp
    src/ui/i18n_zh_tw.cpp
    src/ui/i18n_ja.cpp
    src/ui/i18n_ko.cpp
    src/ui/i18n_ru.cpp
    src/ui/i18n_it.cpp
    src/ui/i18n_ar.cpp
    src/ui/i18n_fr.cpp
    src/ui/i18n_de.cpp
    src/ui/i18n_es.cpp
    src/ui/i18n_pt.cpp
)
target_include_directories(mark-shot-i18n-tables-test PRIVATE src)
target_link_libraries(mark-shot-i18n-tables-test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)
add_test(NAME i18n-tables COMMAND mark-shot-i18n-tables-test)

qt_add_executable(mark-shot-settings-dialog-smoke-test
    tests/settings_dialog_smoke_test.cpp
    $<FILTER:$<FILTER:$<TARGET_OBJECTS:mark-shot>,EXCLUDE,main\.cpp\.o$>,EXCLUDE,main\.cpp\.obj$>
)
target_include_directories(mark-shot-settings-dialog-smoke-test PRIVATE src plugin-sdk)
target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE MARK_SHOT_VERSION="${PROJECT_VERSION}")
if(MSVC)
    target_compile_options(mark-shot-settings-dialog-smoke-test PRIVATE /utf-8)
endif()
target_link_libraries(mark-shot-settings-dialog-smoke-test
    PRIVATE
        Qt6::Core
        Qt6::Concurrent
        Qt6::Gui
        Qt6::Network
        Qt6::Widgets
        Qt6::Test
)
if(WIN32)
    # 与主目标保持一致：Windows 系统库（WGC 捕获 / 托盘等依赖）。
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE
        d3d11 dwmapi dxgi ksuser ole32 user32 uuid windowsapp)
endif()
if(MARK_SHOT_LINUX)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE Qt6::DBus)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE MARK_SHOT_WITH_DBUS)
endif()
if(MARK_SHOT_LINUX AND X11_xcb_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE X11::xcb X11::X11)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE HAVE_XCB)
endif()
if(MARK_SHOT_LINUX AND WaylandClient_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE PkgConfig::WaylandClient)
endif()
# 与主目标保持一致：可选功能库（内置扫码 / FFmpeg 录制 / PulseAudio 采集）。
if(ZXing_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE ZXing::ZXing)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE HAVE_ZXING_CPP)
elseif(ZXingCpp_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE PkgConfig::ZXingCpp)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE HAVE_ZXING_CPP)
endif()
if(FFmpegLibav_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE MarkShot::FFmpegLibav)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE HAVE_LIBAV_RECORDING)
endif()
if(PulseAudioRecording_FOUND)
    target_link_libraries(mark-shot-settings-dialog-smoke-test PRIVATE PkgConfig::PulseAudioRecording)
    target_compile_definitions(mark-shot-settings-dialog-smoke-test PRIVATE HAVE_PULSE_RECORDING)
endif()
add_test(NAME settings-dialog-smoke COMMAND mark-shot-settings-dialog-smoke-test)
