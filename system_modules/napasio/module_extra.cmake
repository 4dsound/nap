if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src ${ASIO_INCLUDE_DIR})
target_compile_definitions(${PROJECT_NAME} PUBLIC ASIO_STANDALONE)

# additional definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
endif()

add_license(asio ${ASIO_LICENSE_FILES})
