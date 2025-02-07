# Static library including a selection of nap modules
# This library can be linked into audio plugins to build a distributable cross-platform NAP plugin

project(napstatic)

include(macros_and_functions.cmake)

# Add the static library target
add_library(${PROJECT_NAME} STATIC src/napstatic.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER napstatic)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

add_napframework_static(${PROJECT_NAME})




