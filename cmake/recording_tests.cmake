qt_add_executable(mark-shot-recording-capture-backend-test
    tests/recording_capture_backend_test.cpp
    src/recording/recording_capture_backend.cpp
    src/recording/recording_capture_backend.h
)
target_include_directories(mark-shot-recording-capture-backend-test PRIVATE src)
target_link_libraries(mark-shot-recording-capture-backend-test
    PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME recording-capture-backend COMMAND mark-shot-recording-capture-backend-test)

qt_add_executable(mark-shot-recording-bgra-buffer-pool-test
    tests/recording_bgra_buffer_pool_test.cpp
    src/recording/recording_bgra_buffer_pool.cpp
    src/recording/recording_bgra_buffer_pool.h
)
target_include_directories(mark-shot-recording-bgra-buffer-pool-test PRIVATE src)
target_link_libraries(mark-shot-recording-bgra-buffer-pool-test
    PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME recording-bgra-buffer-pool COMMAND mark-shot-recording-bgra-buffer-pool-test)

if(MARK_SHOT_LINUX AND PipeWire_FOUND)
    qt_add_executable(mark-shot-pipewire-buffer-data-types-test
        tests/pipewire_buffer_data_types_test.cpp
        src/pipewire/pipewire_buffer_data_types.cpp
        src/pipewire/pipewire_buffer_data_types.h
    )
    target_include_directories(mark-shot-pipewire-buffer-data-types-test PRIVATE src)
    target_link_libraries(mark-shot-pipewire-buffer-data-types-test
        PRIVATE
            Qt6::Core
            Qt6::Test
            PkgConfig::PipeWire
    )
    add_test(NAME pipewire-buffer-data-types COMMAND mark-shot-pipewire-buffer-data-types-test)
endif()

qt_add_executable(mark-shot-recording-dialog-config-test
    tests/recording_dialog_config_test.cpp
    src/recording/recording_dialog_config.cpp
    src/recording/recording_dialog_config.h
    src/recording/recording_capture_backend.cpp
    src/recording/recording_capture_backend.h
    src/recording/recording_file_naming.cpp
    src/recording/recording_file_naming.h
    src/recording/recording_status.cpp
    src/recording/recording_status.h
    src/recording/recording_storage_config.cpp
    src/recording/recording_storage_config.h
    src/app_config_defaults.cpp
    src/app_config_defaults.h
    src/startup_behavior_config.cpp
    src/startup_behavior_config.h
    src/app_config_store.cpp
    src/app_config_store.h
    src/config_value.cpp
    src/config_value.h
    src/debug_log.cpp
    src/debug_log.h
    src/shell_command.cpp
    src/shell_command.h
    src/window_detection.cpp
    src/window_detection.h
)
target_include_directories(mark-shot-recording-dialog-config-test PRIVATE src)
target_link_libraries(mark-shot-recording-dialog-config-test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)
add_test(NAME recording-dialog-config COMMAND mark-shot-recording-dialog-config-test)

qt_add_executable(mark-shot-recording-config-dialog-test
    tests/recording_config_dialog_test.cpp
    src/recording/recording_start_flow.cpp
    src/recording/recording_start_flow.h
    src/recording/recording_config_dialog.cpp
    src/recording/recording_config_dialog.h
    src/recording/recording_dialog_config.cpp
    src/recording/recording_dialog_config.h
    src/recording/recording_capture_backend.cpp
    src/recording/recording_capture_backend.h
    src/recording/recording_file_naming.cpp
    src/recording/recording_file_naming.h
    src/recording/recording_storage_config.cpp
    src/recording/recording_storage_config.h
    src/recording/recording_display_source.cpp
    src/recording/recording_display_source.h
    src/recording/audio/audio_capture_reader_factory.cpp
    src/recording/audio/audio_capture_reader_factory.h
    src/recording/audio/audio_capture_reader.h
    src/recording/audio/audio_capture_sample.h
    src/recording/audio/pulse_audio_capture_reader.cpp
    src/recording/audio/pulse_audio_capture_reader.h
    src/recording/audio/wasapi_audio_capture_reader.cpp
    src/recording/audio/wasapi_audio_capture_reader.h
    src/settings/settings_wheel_guard.cpp
    src/settings/settings_wheel_guard.h
    src/settings/settings_ui_helpers.cpp
    src/settings/settings_ui_helpers.h
    src/settings/settings_design_tokens.cpp
    src/settings/settings_design_tokens.h
    src/app_config_defaults.cpp
    src/app_config_defaults.h
    src/startup_behavior_config.cpp
    src/startup_behavior_config.h
    src/app_config_store.cpp
    src/app_config_store.h
    src/config_value.cpp
    src/config_value.h
    src/debug_log.cpp
    src/debug_log.h
    src/shell_command.cpp
    src/shell_command.h
    src/window_detection.cpp
    src/window_detection.h
    src/windows_integration.cpp
    src/windows_integration.h
    src/ui/i18n.cpp
    src/ui/i18n.h
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
    src/ui/interface_language_config.cpp
    src/ui/interface_language_config.h
    src/ui/interface_theme_config.cpp
    src/ui/interface_theme_config.h
    src/ui/theme.cpp
    src/ui/theme.h
)
target_include_directories(mark-shot-recording-config-dialog-test PRIVATE src)
target_link_libraries(mark-shot-recording-config-dialog-test
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
)
if(PulseAudioRecording_FOUND)
    target_compile_definitions(mark-shot-recording-config-dialog-test PRIVATE HAVE_PULSE_RECORDING)
    target_link_libraries(mark-shot-recording-config-dialog-test PRIVATE PkgConfig::PulseAudioRecording)
endif()
# Windows 需要 WASAPI 读取器依赖与 windows_integration 的系统库，
# 与主目标保持一致，避免 Windows 测试链接失败。
if(WIN32)
    target_link_libraries(mark-shot-recording-config-dialog-test PRIVATE dwmapi ksuser ole32 uuid)
endif()
add_test(NAME recording-config-dialog COMMAND mark-shot-recording-config-dialog-test)

if(FFmpegLibav_FOUND)
    qt_add_executable(mark-shot-libav-recording-process-test
        tests/libav_recording_process_test.cpp
        src/recording/audio/audio_capture_reader.h
        src/recording/audio/audio_capture_reader_factory.cpp
        src/recording/audio/audio_capture_reader_factory.h
        src/recording/audio/audio_capture_sample.h
        src/recording/audio/pulse_audio_capture_reader.cpp
        src/recording/audio/pulse_audio_capture_reader.h
        src/recording/audio/wasapi_audio_capture_reader.cpp
        src/recording/audio/wasapi_audio_capture_reader.h
        src/debug_log.cpp
        src/debug_log.h
        src/recording/libav_audio_encoder.cpp
        src/recording/libav_audio_encoder.h
        src/recording/libav_error.cpp
        src/recording/libav_error.h
        src/recording/libav_gif_recording_process.cpp
        src/recording/libav_gif_recording_process.h
        src/recording/libav_recording_process.cpp
        src/recording/libav_recording_process.h
        src/recording/recording_frame_converter.cpp
        src/recording/recording_frame_converter.h
        src/recording/recording_frame_payload.cpp
        src/recording/recording_frame_payload.h
    )
    target_include_directories(mark-shot-libav-recording-process-test PRIVATE src)
    target_compile_definitions(mark-shot-libav-recording-process-test PRIVATE HAVE_LIBAV_RECORDING)
    if(PulseAudioRecording_FOUND)
        target_compile_definitions(mark-shot-libav-recording-process-test PRIVATE HAVE_PULSE_RECORDING)
    endif()
    target_link_libraries(mark-shot-libav-recording-process-test
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Test
            MarkShot::FFmpegLibav
    )
    if(PulseAudioRecording_FOUND)
        target_link_libraries(mark-shot-libav-recording-process-test PRIVATE PkgConfig::PulseAudioRecording)
    endif()
    if(WIN32)
        target_link_libraries(mark-shot-libav-recording-process-test PRIVATE ksuser ole32 uuid)
    endif()
    add_test(NAME libav-recording-process COMMAND mark-shot-libav-recording-process-test)
endif()
