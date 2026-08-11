# OpenAI-compatible 翻译插件：复用现有 translation 配置与环境变量

qt_add_plugin(dracoPho-translate-openai
    SHARED
    openai_translate_config.cpp
    openai_translate_config.h
    openai_translate_plugin.cpp
    openai_translate_plugin.h
    openai_translation_parser.cpp
    openai_translation_parser.h
)

target_include_directories(dracoPho-translate-openai PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/plugin-sdk
)
target_link_libraries(dracoPho-translate-openai
    PRIVATE
        Qt6::Core
        Qt6::Network
)

install(TARGETS dracoPho-translate-openai
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dracoPho/plugins
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/dracoPho/plugins)

message(STATUS "dracoPho: translate-openai plugin enabled")
