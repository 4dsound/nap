cmake_minimum_required(VERSION 3.18.4)
project(mod_napapp)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${ANDROID_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

package_module()
