macro(add_napframework_static target_name)
    # Find RTTR library
    # Find RTTR::Core through its provided cmake files using the package finder
    set(RTTR_DIR "${NAP_ROOT}/thirdparty/rttr/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/cmake")
    message(${RTTR_DIR})
    find_package(RTTR CONFIG REQUIRED Core)

    # Find rapidjson header only library
    set(rapidjson_FIND_QUIETLY TRUE)
    find_package(rapidjson REQUIRED)
    add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

    # Find glm header only library
    include(${NAP_ROOT}/system_modules/napmath/thirdparty/cmake_find_modules/Findglm.cmake)

    # Add NAP module sources and include directories
    add_napmodule_static(${target_name} rtti ".*naprtti.cpp")
    add_napmodule_static(${target_name} utility)
    add_napcore_static(${target_name})
    add_napmodule_static(${target_name} system_modules/napscene ".*mod_napscene.cpp")
    add_napmodule_static(${target_name} system_modules/napmath ".*mod_napmath.cpp")

    # Link NAP thirdparty dependencies
    target_include_directories(${target_name} PUBLIC ${RAPIDJSON_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${GLM_INCLUDE_DIRS})
    target_link_libraries(${target_name} PUBLIC naputility RTTR::Core_Lib)
    if(APPLE)
        target_link_libraries(${target_name} PRIVATE "-framework CoreServices")
    elseif(UNIX)
        target_link_libraries(${target_name} PRIVATE dl pthread)
    endif ()

    # Set compile definitions
    target_compile_definitions(${target_name} PRIVATE _USE_MATH_DEFINES MODULE_NAME=${target_name})
    if(MSVC)
        target_compile_definitions(${target_name} PUBLIC NOMINMAX)
    endif()
    target_compile_definitions(${target_name} PUBLIC GLM_FORCE_INIT)
endmacro()


macro(add_napmodule_static target_name module_name)
    set(source_dir ${NAP_ROOT}/${module_name}/src)
    file(GLOB_RECURSE SOURCES ${source_dir}/*.cpp ${source_dir}/*.h)

    # Loop through optional arguments and exclude them from the sources list
    foreach(element ${ARGN})
        list(FILTER SOURCES EXCLUDE REGEX ${element})
    endforeach()

    target_sources(${target_name} PUBLIC ${SOURCES})
    target_include_directories(${target_name} PUBLIC ${source_dir})
endmacro()


macro(add_napcore_static target_name)
    set(source_dir ${NAP_ROOT}/core/src)

    file(GLOB SOURCES ${source_dir}/nap/*.cpp src/nap/*.h)
    list(FILTER SOURCES EXCLUDE REGEX "${source_dir}/nap/napcore.cpp")

    # Add whether we're a NAP release or not
    configure_file(${source_dir}/nap/packaginginfo.h.in packaginginfo.h @ONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/packaginginfo.h)

    # Populate version info
    include(${NAP_ROOT}/cmake/version.cmake)
    configure_file(${source_dir}/nap/version.h.in version.h @ONLY)

    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/version.h)

    if(APPLE)
        list(APPEND
                SOURCES
                ${source_dir}/nap/osx/directorywatcher.cpp
                ${source_dir}/nap/osx/module.cpp
                ${source_dir}/nap/osx/core_env.cpp
                ${source_dir}/nap/native/modulemanager_ext.cpp
                ${source_dir}/nap/native/logger_ext.cpp
                ${source_dir}/nap/native/core_ext.cpp
                ${source_dir}/nap/native/resourcemanager_ext.cpp
                )
    elseif(MSVC)
        list(APPEND
                SOURCES
                ${source_dir}/nap/win32/directorywatcher.cpp
                ${source_dir}/nap/win32/module.cpp
                ${source_dir}/nap/win32/core_env.cpp
                ${source_dir}/nap/native/modulemanager_ext.cpp
                ${source_dir}/nap/native/logger_ext.cpp
                ${source_dir}/nap/native/core_ext.cpp
                ${source_dir}/nap/native/resourcemanager_ext.cpp
                )
    elseif(UNIX)
        file(GLOB FILEWATCHER_SOURCES
                ${source_dir}/nap/linux/FileWatcher/*.cpp
                ${source_dir}/nap/linux/FileWatcher/*.h)
        list(APPEND
                SOURCES
                ${FILEWATCHER_SOURCES}
                ${source_dir}/nap/linux/directorywatcher.cpp
                ${source_dir}/nap/linux/module.cpp
                ${source_dir}/nap/linux/core_env.cpp
                ${source_dir}/nap/native/modulemanager_ext.cpp
                ${source_dir}/nap/native/logger_ext.cpp
                ${source_dir}/nap/native/core_ext.cpp
                ${source_dir}/nap/native/resourcemanager_ext.cpp
                )
    endif()
    target_sources(${target_name} PUBLIC ${SOURCES})
    target_include_directories(${target_name} PUBLIC ${source_dir})
endmacro()
