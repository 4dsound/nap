include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(assimp)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include)
if (WIN32)
    set(implib ${platform_dir}/lib/assimp-vc140-mt.lib)
    set(dll ${platform_dir}/bin/assimp-vc140-mt.dll)
else ()
    set(lib_dir ${platform_dir}/lib)
    set(implib ${lib_dir}/${implib_prefix}${PROJECT_NAME}${implib_suffix})
    set(dll ${lib_dir}/${dll_prefix}${PROJECT_NAME}${dll_suffix})
endif()

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})