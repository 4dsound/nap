include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(SDL2)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
if (WIN32)
    set(include_dir ${platform_dir}/include)
else ()
    set(include_dir ${platform_dir}/include/SDL2)
endif()
set(lib_dir ${platform_dir}/lib)
set(implib ${lib_dir}/${implib_prefix}${PROJECT_NAME}${implib_suffix})
set(dll ${lib_dir}/${dll_prefix}${PROJECT_NAME}${dll_suffix})

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})