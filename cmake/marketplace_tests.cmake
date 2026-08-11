qt_add_executable(dracoPho-plugin-index-parser-test
    tests/plugin_index_parser_test.cpp
    src/marketplace/plugin_index_parser.cpp
    src/marketplace/plugin_index_parser.h
)
target_include_directories(dracoPho-plugin-index-parser-test PRIVATE src)
target_compile_definitions(dracoPho-plugin-index-parser-test
    PRIVATE
        MARK_SHOT_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(dracoPho-plugin-index-parser-test
    PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME plugin-index-parser COMMAND dracoPho-plugin-index-parser-test)

qt_add_executable(dracoPho-plugin-installer-test
    tests/plugin_installer_test.cpp
    src/marketplace/plugin_installer.cpp
    src/marketplace/plugin_installer.h
    src/providers/provider_plugin_paths.cpp
    src/providers/provider_plugin_paths.h
)
target_include_directories(dracoPho-plugin-installer-test PRIVATE src)
target_link_libraries(dracoPho-plugin-installer-test
    PRIVATE
        Qt6::Core
        Qt6::Test
)
add_test(NAME plugin-installer COMMAND dracoPho-plugin-installer-test)
