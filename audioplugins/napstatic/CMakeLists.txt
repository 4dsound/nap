# Static library including a selection of nap modules
# This library can be linked into audio plugins to build a distributable cross-platform NAP plugin

project(napstatic)

include(${NAP_ROOT}/cmake/nap_static.cmake)

# Add the static library target
add_library(${PROJECT_NAME} STATIC src/napstatic.cpp src/napstatic.h)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER napstatic)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

# Add the nap core
add_napframework_static(${PROJECT_NAME})
set(audio_file_support_filter ".*audiofileutils.*" ".*audiofileresource.*" ".*audiofile.*")
add_napmodule_static(${PROJECT_NAME} system_modules/napaudio ${audio_file_support_filter} ".*mod_napaudio")
add_napmodule_static(${PROJECT_NAME} modules/napaudioadvanced ${audio_file_support_filter} ".*mod_napaudioadvanced.cpp")





