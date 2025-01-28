# Static library including a selection of nap modules
# This library can be linked into audio plugins to build a distributable cross-platform NAP plugin

project(napstatic)

include(macros_and_functions.cmake)

# Find RTTR library
# Find RTTR::Core through its provided cmake files using the package finder
set(RTTR_DIR "${NAP_ROOT}/thirdparty/rttr/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/cmake")
message(${RTTR_DIR})
find_package(RTTR CONFIG REQUIRED Core_Lib)

# Find rapidjson header only library
set(rapidjson_FIND_QUIETLY TRUE)
find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

# Find glm header only library
include(${NAP_ROOT}/system_modules/napmath/thirdparty/cmake_find_modules/Findglm.cmake)

# Add the static library target
add_library(${PROJECT_NAME} STATIC src/napstatic.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER napstatic)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add NAP module sources and include directories
add_napmodule_static(rtti)
add_napmodule_static(utility)
add_napmodule_static(system_modules/napscene)
add_napmodule_static(system_modules/napmath)
add_napcore_static()

# Link NAP thirdparty dependencies
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${GLM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC naputility RTTR::Core_Lib)
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread)
endif ()

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES MODULE_NAME=${PROJECT_NAME})
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_INIT)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PUBLIC NAP_STATIC)




