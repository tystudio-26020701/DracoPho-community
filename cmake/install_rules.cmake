install(TARGETS dracoPho RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    README.md
    README.zh-CN.md
    CHANGELOG.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
install(DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs
)

if(TARGET dracoPho-layer-shell)
    install(TARGETS dracoPho-layer-shell
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dracoPho
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(MARK_SHOT_LINUX)
    set(MARK_SHOT_DESKTOP_EXEC "${CMAKE_INSTALL_FULL_BINDIR}/dracoPho")
    configure_file(data/dracoPho.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/dracoPho.desktop" @ONLY)
    configure_file(data/dracoPho-edit.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/dracoPho-edit.desktop" @ONLY)
    configure_file(data/net.local.dracoPho.desktop.in "${CMAKE_CURRENT_BINARY_DIR}/net.local.dracoPho.desktop" @ONLY)

    install(PROGRAMS
        scripts/dracoPho-ocr
        scripts/dracoPho-code-scan
        scripts/dracoPho-translate
        scripts/dracoPho-upload
        scripts/dracoPho-window-detection-niri
        scripts/dracoPho-window-detection-hyprland
        scripts/dracoPho-window-detection-gnome
        scripts/dracoPho-window-detection-kde
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY scripts/lib/mark_shot_window_detection
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dracoPho/python
        FILES_MATCHING
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/dracoPho.desktop"
        "${CMAKE_CURRENT_BINARY_DIR}/dracoPho-edit.desktop"
        "${CMAKE_CURRENT_BINARY_DIR}/net.local.dracoPho.desktop"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(FILES
        data/icons/hicolor/scalable/apps/dracoPho.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    )
    install(DIRECTORY
        packaging/gnome-extension/mark-shot-scroll-helper@snemc.org/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/gnome-shell/extensions/mark-shot-scroll-helper@snemc.org
        FILES_MATCHING
            PATTERN "metadata.json"
            PATTERN "*.js"
    )
endif()
