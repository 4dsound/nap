project(test)

add_executable(test main.cpp)

target_link_libraries(test napstatic)

# Add the app structure path (for realtime editing)
set(data_dir ${CMAKE_CURRENT_LIST_DIR}/data)
set(app_structure_filename objects.json)
target_compile_definitions(${PROJECT_NAME} PUBLIC "APP_STRUCTURE_PATH=${data_dir}/${app_structure_filename}")
target_compile_definitions(${PROJECT_NAME} PUBLIC "DATA_DIR=${data_dir}")

