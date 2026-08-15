set(FFmpegLibav_FOUND FALSE)
set(FFmpegLibav_LIBRARIES)
set(FFmpegLibav_REQUIRED_VARS)

if(MARK_SHOT_LINUX)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFmpegLibav QUIET IMPORTED_TARGET
            libavcodec
            libavformat
            libavutil
            libswresample
            libswscale
        )
        if(FFmpegLibav_FOUND AND NOT TARGET MarkShot::FFmpegLibav)
            add_library(MarkShot::FFmpegLibav INTERFACE IMPORTED)
            target_link_libraries(MarkShot::FFmpegLibav INTERFACE PkgConfig::FFmpegLibav)
        endif()
    endif()
elseif(WIN32)
    include(FindPackageHandleStandardArgs)

    find_path(FFmpegLibav_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        HINTS ENV MINGW_PREFIX
        PATH_SUFFIXES include
    )

    foreach(component IN ITEMS avcodec avformat avutil swresample swscale)
        find_library(FFmpegLibav_${component}_LIBRARY
            NAMES ${component} lib${component}
            HINTS ENV MINGW_PREFIX
            PATH_SUFFIXES lib
        )
        list(APPEND FFmpegLibav_REQUIRED_VARS FFmpegLibav_${component}_LIBRARY)
        list(APPEND FFmpegLibav_LIBRARIES "${FFmpegLibav_${component}_LIBRARY}")
    endforeach()

    list(APPEND FFmpegLibav_REQUIRED_VARS FFmpegLibav_INCLUDE_DIR)
    find_package_handle_standard_args(FFmpegLibav
        REQUIRED_VARS ${FFmpegLibav_REQUIRED_VARS}
    )

    if(FFmpegLibav_FOUND AND NOT TARGET MarkShot::FFmpegLibav)
        add_library(MarkShot::FFmpegLibav INTERFACE IMPORTED)
        target_include_directories(MarkShot::FFmpegLibav INTERFACE "${FFmpegLibav_INCLUDE_DIR}")
        # 静态库间存在循环依赖（avformat → avcodec → avutil → avformat），
        # 用链接器组重新扫描以解析所有未定义符号（MinGW ld 与 GNU ld 均支持）。
        target_link_libraries(MarkShot::FFmpegLibav INTERFACE
            -Wl,--start-group
            ${FFmpegLibav_LIBRARIES}
            -Wl,--end-group
        )
    endif()
endif()

if(MARK_SHOT_REQUIRE_FFMPEG AND NOT FFmpegLibav_FOUND)
    message(FATAL_ERROR "FFmpeg libav support was required but libavcodec/libavformat dependencies were not found")
endif()
