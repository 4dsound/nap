# default artnet directory
find_path(ARTNET_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES artnet/artnet.h
          HINTS ${THIRDPARTY_DIR}/libartnet
          )

if(WIN32)
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/msvc/install/bin/Release)
    set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libartnet.lib)
    set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR})
    set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libartnet.dll)
elseif(APPLE)
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/osx/bin/Release)
    set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libArtnet.dylib)
    set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR} ${ARTNET_DIR}/osx)
    set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS})
elseif(ANDROID)
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/android/bin/Release/${ANDROID_ABI})
    set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libartnet.so)
    set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR} ${ARTNET_DIR}/android)
    set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS})
else()
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/linux/bin)
    set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libartnet.so.1)
    set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR} ${ARTNET_DIR}/linux)
    set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS})
endif()

mark_as_advanced(ARTNET_INCLUDE_DIRS)
mark_as_advanced(ARTNET_LIBS_DIR)


add_library(artnet SHARED IMPORTED)
set_target_properties(artnet PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${ARTNET_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${ARTNET_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(artnet PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${ARTNET_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${ARTNET_LIBS}
                          )
endif()


# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(artnet REQUIRED_VARS ARTNET_DIR)

# Copy the artnet dynamic linked lib into the build directory
macro(copy_artnet_dll)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different
                               $<TARGET_FILE:artnet>
                               "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                       )
endmacro()
